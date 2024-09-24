#include <stdint.h>
#include "stm32f407xx.h"
#include "kernel.h"

static void kernel_on_idle(void);

/* These pointers will be used inside ISRs so make sure they're volatile */
static tcb_type* volatile current_thread;
static tcb_type* volatile next_thread;

static tcb_type* kernel_tcbs[32 + 1];	/* array that holds all the thread pointers */
static uint8_t kernel_tcbs_count;		/* int value that keeps count of total threads started */
static uint8_t kernel_tcbs_index;		/* index value to be used for round robin scheduling */
static uint32_t kernel_tcbs_ready_mask;		/* 32 bit mask to keep track of all thread's and whether they are ready or blocked */


uint32_t idlethread_stack[40];
tcb_type idlethread;
void main_idlethread(void)
{
	while (1) {
		kernel_on_idle();
	}
}

void kernel_initialize(void)
{
	/* Lower number set means higher priority calling. */
	NVIC_SetPriority(SysTick_IRQn, 0U);
	NVIC_SetPriority(PendSV_IRQn, 0xFFU);

	kernel_tcb_start(
			&idlethread,
			&main_idlethread,
			idlethread_stack,
			sizeof(idlethread_stack));
}

/* Function to start the kernel.
 * Set the priorities for the interrupts so PendSV does NOT preempt Systick.
 * PendSV should only context switch by tail-chaining and once other interrupts have already been serviced.
 * Start the scheduler to initiate the running state of one thread, without having to wait for the Systick to trigger it first.
 */
void kernel_run(void)
{
	__disable_irq();
	kernel_scheduler_round_robin();
	__enable_irq();
}

void kernel_scheduler_round_robin(void)
{
	/* If no threads are ready to run, run the idle thread by setting the index to 0 */
	if (kernel_tcbs_ready_mask == 0U) {
		kernel_tcbs_index = 0U;
	} else {
		/* We need to check which thread(s) is ready to run. This means iterating through the array of all tcbs.
		 * At each iteration, we check the corresponding ready_mask bit for that thread_index and run it if its ready.
		 * The loop should continue to run as long as the thread bit it's checking is 0, UNTIL it reaches a ready thread. */
		do {
			++kernel_tcbs_index;
			/* Wrap around to the beginning of the tcbs array once you get to the end.
			 * Skipping the idle thread. */
			if (kernel_tcbs_index == kernel_tcbs_count) {
				kernel_tcbs_index = 1U;
			}
		} while ((kernel_tcbs_ready_mask & (1U << (kernel_tcbs_index - 1U))) == 0U);
	}
	/* Once it's found a ready thread, schedule it to run as the next thread */
	next_thread = kernel_tcbs[kernel_tcbs_index];

	if (next_thread != current_thread) {
		/* Set the PendSVHandler bit to get ready for context switch.
		 * Note: NVIC_SetPendingIRQ(PendSV_IRQn) does NOT work.
		 * 	This is because NVIC_SetPendingIRQ() function only works for external, positive IRQ numbered interrupts.
		 * 	PendSV is a special interrupt just like Systick which has a negative IRQn.
		 * 	This means you have to use the SCB_ICSR register to set the pending bit.
		 */
		SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
	}
}

/* Function to initialize threads */
void kernel_tcb_start(
	tcb_type* me,
	tcb_type_handler tcb_handler,
	void* stack_array,
	uint32_t stack_size)
{
	/* In ARM Cortex M, the stack grows from high to low memory so we need to start from the END of the stack, hence we add stack_size.
	 * The stack also needs to be aligned at the 8 byte boundary, so we integer divide by 8 then * by 8 to guarantee this.
	 * For example, 52 is 4 bytes aligned, but not 8 bytes aligned. Integer division by 8 then * 8 results in 48, which is 8 bytes aligned.
	 * The user of the function might not be aware of these requirements, so its unwise to assume the end of the provided stack memory would be properly aligned.
	 * This is why we round down the end address.
	 */
	uint32_t* sp = (uint32_t*)((((uint32_t)stack_array + stack_size) / 8) * 8);

	/* The -1U and +1U ensures that the stack_limit will be 8 byte aligned.
	 * If the address is already 8 byte aligned then it won't be rounded down incorrectly.
	 * If the address is not aligned, then it guarantees it will be rounded UP to the next correct 8 byte boundary.
	 * Example:
	 *
	 * 0x10000005 is not aligned.
	 * 0x10000005 - 1 = 0x10000004
	 * 0x10000004 / 8 = 0x02000000
	 * 0x02000000 + 1 = 0x02000001
	 * 0x02000001 * 8 = 0x10000008
	 *
	 * 0x10000008 is now 8 byte aligned
	 */
	uint32_t* stack_limit = (uint32_t*)(((((uint32_t)stack_array - 1U) / 8) + 1U) * 8);

	/* Push the exception entry and CPU context registers in order.
	 * Make sure to set the thumb mode in PSR register bit 24.
	 */
	*(--sp) = xPSR_T_Msk;				/* PSR */
	*(--sp) = (uint32_t)tcb_handler;	/* PC  */
	*(--sp) = 0xAAAAAAAEU;				/* LR  */
	*(--sp) = 0xAAAAAAACU;				/* R12 */
	*(--sp) = 0xAAAAAAA3U;				/* R3  */
	*(--sp) = 0xAAAAAAA2U;				/* R2  */
	*(--sp) = 0xAAAAAAA1U;				/* R1  */
	*(--sp) = 0xAAAAAAA0U;				/* R0  */

	*(--sp) = 0xAAAAAAABU;				/* R11 */
	*(--sp) = 0xAAAAAAAAU;				/* R10 */
	*(--sp) = 0xAAAAAAA9U;				/* R9  */
	*(--sp) = 0xAAAAAAA8U;				/* R8  */
	*(--sp) = 0xAAAAAAA7U;				/* R7  */
	*(--sp) = 0xAAAAAAA6U;				/* R6  */
	*(--sp) = 0xAAAAAAA5U;				/* R5  */
	*(--sp) = 0xAAAAAAA4U;				/* R4  */

	/* Update the tcb sp */
	me->sp = sp;

	/* Prefill the unused memory addresses for debugging purposes.
	 * We start the sp at the next empty address.
	 */
	for (sp = sp - 1; sp >= stack_limit; sp--) {
		*sp = 0xBAADF00DU;
	}

	/* Check to make sure we don't go over the thread limit */
	if (kernel_tcbs_count < (sizeof(kernel_tcbs) / sizeof(kernel_tcbs[0]))) {
		kernel_tcbs[kernel_tcbs_count] = me;
	}

	/* For all non-idle threads, make sure to set them ready to run.
	 * We skip the idle thread by checking > 1 */
	if (kernel_tcbs_count > 0) {
		kernel_tcbs_ready_mask |= (1U << (kernel_tcbs_count - 1));
	}

	/* Increment the count at the end, otherwise the tcbs_ready_mask won't be set properly and the bit alignment will be off */
	kernel_tcbs_count++;
}

/* Function to block current thread for a specified amount of time.
 * It is important to note the idlethread must never be blocked.
 */
void kernel_tcb_block(uint32_t blocking_timeout)
{
	/* The thread blocking must happen inside of a critical section */
	__disable_irq();

	/* The blocking function should NEVER be called on the idle thread */
	if (current_thread != kernel_tcbs[0]) {

		/* First load the desired blocking timeout to the thread attribute */
		current_thread->timeout = blocking_timeout;

		/* Then block the thread by clearing the appropriate bit in the tcb ready mask */
		kernel_tcbs_ready_mask &= ~(1U << (kernel_tcbs_index - 1));

		/* Immediately call the scheduler to context switch away from the blocked thread */
		kernel_scheduler_round_robin();
	}

	__enable_irq();
}

/* This function works in tandem with the kernel_tcb_block().
 * At every iteration of the Systick Handler, this function is called to go through each timeout value
 * 	for every thread in kernel_tcbs[] and decrement all non-0 timeout values by 1. If the timeout value
 * 	reaches 0, then unblock the thread.
 */
void kernel_tcb_permit(void)
{
	/* Start at i = 1U to skip the idle thread */
	uint8_t i;
	for (i = 1U; i < kernel_tcbs_count; i++) {
		if (kernel_tcbs[i]->timeout != 0) {
			--kernel_tcbs[i]->timeout;

			/* Nested if statement because the previous if statement COULD decrement a thread's timeout to 0 */
			if (kernel_tcbs[i]->timeout == 0) {
				kernel_tcbs_ready_mask |= (1U << (i - 1));
			}
		}
	}
}

static void kernel_on_idle(void)
{
	led_green_toggle();
	led_green_toggle();
}

/* ARM Cortex M exception handler that centralizes context switching for an RTOS implementation.
 * The code for the context switching needs to be written in inline assembly.
 * It was assisted by writing the desired C code logic first, then triggering the PendSV manually and using the
 * 	compiler generated ASM code as the base.
 *
 * The logic for the PendSV Handler is as follows:
 * 1) Disable interrupts
 * 2) Check if theres a current thread running. If there is, Push the context by saving R4-R11 and saving the SP to current TCB's SP.
 * 3) Load the next thread and set the current thread to the next thread.
 * 4) Load the SP for the now new current thread into the processor's SP.
 * 5) Restore the context of the new current thread by popping registers R4-R11.
 * 6) Enable interrupts.
 * 7) Branch to the next thread.
 */
__attribute__((naked)) void PendSV_Handler(void)
{
	/* __disable__irq(); */
	__asm("CPSID	I");
	__asm("LDR    R3, =current_thread");


	/* if (current_thread != (tcb_type*)0) */
	__asm("LDR     R3, [R3, #0]");
	__asm("CMP     R3, #0");
	__asm("BEQ.N   PendSV_Restore");

	/* Save R4 - R11 */
	__asm("PUSH	{R4-R11}");

	/* current_thread->sp = sp;
	 * Save the current SP in to current_thread->sp
	 */
	__asm("LDR     R3, =current_thread");
	__asm("LDR	   R3, [R3, #0]");
	__asm("STR     SP, [R3, #0]");

	/* current_thread = next_thread; */
	__asm("PendSV_Restore:");
	__asm("LDR     R3, =next_thread");
	__asm("LDR     R3, [R3, #0]");
	__asm("LDR     R2, =current_thread");
	__asm("STR     R3, [R2, #0]");

	/* sp = current_thread->sp;
	 * Note: Since SP is a special-purpose register, you have to use LDR instruction.
	 * 		 Replaced "STR [R3, #0], SP" with
	 * 		 		  "LDR SP, [R3, #0]"
	 */
	__asm("LDR     R3, =current_thread");	/* Load the address of current thread */
	__asm("LDR     R3, [R3, #0]");			/* Load the value of current thread (pointer to TCB) */
	__asm("LDR     SP, [R3, #0]");			/* Load the SP from TCB into the stack pointer */

	/* Restore R4-R11 */
	__asm("POP	 {R4-R11}");

	/* __enable_irq(); */
	__asm("CPSIE   I");

	/* return to the next thread */
	__asm("BX	LR");
}
