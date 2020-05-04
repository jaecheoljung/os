/********************************************************
 * Filename: core/task.c
 * 
 * Author: parkjy, RTOSLab. SNU.
 * 
 * Description: task management.
 ********************************************************/
#include <core/eos.h>

#define READY		1
#define RUNNING		2
#define WAITING		3

/*
 * Queue (list) of tasks that are ready to run.
 */
static _os_node_t *_os_ready_queue[LOWEST_PRIORITY + 1];

/*
 * Pointer to TCB of running task
 */
static eos_tcb_t *_os_current_task;

int32u_t eos_create_task(eos_tcb_t *task, addr_t sblock_start, size_t sblock_size, void (*entry)(void *arg), void *arg, int32u_t priority) {

	//Register task information
	task->sblock_start	= sblock_start;
	task->sblock_size	= sblock_size;
	task->entry			= entry;
	task->arg			= arg;
	task->sp 			= _os_create_context(sblock_start, sblock_size, entry, arg);	
	task->node.next 	= NULL;
	task->node.priority = priority;
	task->node.ptr_data = task;
	task->state 		= READY;
	
	//Load task on ready queue

	_os_add_node_tail(&(_os_ready_queue[priority]), &(task->node));
	_os_set_ready(priority);
	PRINT("task: 0x%x, priority: %d\n", (int32u_t)task, priority);
	
	return 0;
}

int32u_t eos_destroy_task(eos_tcb_t *task) {
}

void eos_schedule() {
	
	if (_os_current_task != NULL){
		
		//Save context
		addr_t sp = _os_save_context();

		//If it was restored, do nothing.
		if (sp == 0) return;

		//Put current task into ready queue.
		_os_current_task->state = READY;
		_os_current_task->sp = sp;
		_os_add_node_tail(&(_os_ready_queue[_os_current_task->node.priority]), &(_os_current_task->node));
		_os_set_ready(_os_current_task->node.priority);

	}

	//Find next task!
	int32u_t p = _os_get_highest_priority();
	_os_current_task = (eos_tcb_t *) (_os_ready_queue[p]->ptr_data);
	_os_current_task->state = RUNNING;

	//Pop it from queue.
	_os_remove_node(&(_os_ready_queue[p]), &(_os_current_task->node));
	if (_os_ready_queue[p] == NULL) _os_unset_ready(p);

	//Restore its context.
	_os_restore_context(_os_current_task->sp);

}

eos_tcb_t *eos_get_current_task() {
	return _os_current_task;
}

void eos_change_priority(eos_tcb_t *task, int32u_t priority) {
}

int32u_t eos_get_priority(eos_tcb_t *task) {
}

void eos_set_period(eos_tcb_t *task, int32u_t period){
}

int32u_t eos_get_period(eos_tcb_t *task) {
}

int32u_t eos_suspend_task(eos_tcb_t *task) {
}

int32u_t eos_resume_task(eos_tcb_t *task) {
}

void eos_sleep(int32u_t tick) {
}

void _os_init_task() {
	PRINT("initializing task module.\n");

	/* init current_task */
	_os_current_task = NULL;

	/* init multi-level ready_queue */
	int32u_t i;
	for (i = 0; i < LOWEST_PRIORITY; i++) {
		_os_ready_queue[i] = NULL;
	}
}

void _os_wait(_os_node_t **wait_queue) {
}

void _os_wakeup_single(_os_node_t **wait_queue, int32u_t queue_type) {
}

void _os_wakeup_all(_os_node_t **wait_queue, int32u_t queue_type) {
}

void _os_wakeup_sleeping_task(void *arg) {
}
