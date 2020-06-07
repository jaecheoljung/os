/********************************************************
 * Filename: core/sync.c
 * 
 * Author: wsyoo, RTOSLab. SNU.
 * 
 * Description: semaphore, condition variable management.
 ********************************************************/
#include <core/eos.h>

void eos_init_semaphore(eos_semaphore_t *sem, int32u_t initial_count, int8u_t queue_type) {
	/* initialization */
	sem->count = initial_count;
	sem->queue_type = queue_type;
	sem->wait_queue = NULL;
}

int32u_t eos_acquire_semaphore(eos_semaphore_t *sem, int32s_t timeout) {
	
	eos_disable_interrupt();

	if (sem->count > 0){
		sem->count --;
		eos_enable_interrupt();
		return 1;
	}

	eos_enable_interrupt();

	if (timeout == -1){
		return 0;
	}

	eos_tcb_t * cur = eos_get_current_task();
	cur->state = 3;
	if (sem->queue_type == 0) _os_add_node_tail(&(sem->wait_queue), &(cur->node));
	if (sem->queue_type == 1) _os_add_node_priority(&(sem->wait_queue), &(cur->node));

	if (timeout == 0){
		eos_schedule();
		eos_disable_interrupt();
		sem->count --;
		eos_enable_interrupt();
		return 1;
	}

	if (timeout >= 1){
		eos_set_alarm(eos_get_system_timer(),
					&(cur->alarm),
					eos_get_system_timer()->tick + timeout,
					_os_wakeup_sleeping_task,
					cur);
		eos_schedule();

		eos_disable_interrupt();

		if (sem->count > 0){
			sem->count --;
			eos_enable_interrupt();
			return 1;
		}

		eos_enable_interrupt();
		return 0;
	}
}

void eos_release_semaphore(eos_semaphore_t *sem) {
	
	eos_disable_interrupt();

	sem->count ++;
	if (sem->count > 0 && sem->wait_queue != NULL){
		eos_tcb_t * next = (eos_tcb_t*) sem->wait_queue->ptr_data;
		_os_remove_node(&(sem->wait_queue), sem->wait_queue);

		if (next->state == 3){
			_os_wakeup_sleeping_task(next);
		}
	}
	
	eos_enable_interrupt();
	
}

void eos_init_condition(eos_condition_t *cond, int32u_t queue_type) {
	/* initialization */
	cond->wait_queue = NULL;
	cond->queue_type = queue_type;
}

void eos_wait_condition(eos_condition_t *cond, eos_semaphore_t *mutex) {
	/* release acquired semaphore */
	eos_release_semaphore(mutex);
	/* wait on condition's wait_queue */
	_os_wait(&cond->wait_queue);
	/* acquire semaphore before return */
	eos_acquire_semaphore(mutex, 0);
}

void eos_notify_condition(eos_condition_t *cond) {
	/* select a task that is waiting on this wait_queue */
	_os_wakeup_single(&cond->wait_queue, cond->queue_type);
}
