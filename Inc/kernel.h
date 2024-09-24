#ifndef KERNEL_H_
#define KERNEL_H_

#include <stdint.h>

/* Struct definition for a thread (TCB) */
typedef struct {
	/* sp is of type void* because it allows the RTOS to manage the sp without
	 * 	assuming the data type on the stack. Remember, void* is a generic pointer type
	 * 	which means it can point to any data type.
	 */
	void* sp;

	/* Timeout variable to keep track of how long a thread should stay blocked */
	uint32_t timeout;
}tcb_type;

/* Function pointer needed to pass in the address of the respective threads */
typedef void (*tcb_type_handler)();

void kernel_initialize(void);
void kernel_scheduler_round_robin(void);
void kernel_run(void);
void kernel_tcb_block(uint32_t blocking_timeout);
void kernel_tcb_permit(void);

/* Function to start a thread, the void* stack_array variable is the address to the start of the stack in memory */
void kernel_tcb_start(
	tcb_type* me,
	tcb_type_handler tcb_handler,
	void* stack_array,
	uint32_t stack_size);

#endif /* KERNEL_H_ */
