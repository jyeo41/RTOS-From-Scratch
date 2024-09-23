#include <stdint.h>
#include "stm32f407xx.h"
#include "kernel.h"

/* These pointers will be used inside ISRs so make sure they're volatile */
tcb_type* volatile current_thread;
tcb_type* volatile next_thread;

/* Function to set the priorities for the interrupts so PendSV does NOT preempt Systick.
 * PendSV should only context switch by tail-chaining and once other interrupts have already been serviced.
 */
void kernel_initialize(void)
{
	/* Lower number set means higher priority calling */
	NVIC_SetPriority(SysTick_IRQn, 0U);
	NVIC_SetPriority(PendSV_IRQn, 0xFFU);
}

void kernel_scheduler(void)
{
	/* Scheduling algorithm goes here such as FCFS, RR, Priority-based, etc */
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
}

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
