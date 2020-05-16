#include <core/eos.h>
#define READY		1
#define RUNNING		2
#define WAITING		3

static _os_node_t *_os_ready_queue[LOWEST_PRIORITY + 1];

static eos_tcb_t *_os_current_task;

int32u_t eos_create_task(eos_tcb_t *task, addr_t sblock_start, size_t sblock_size, void (*entry)(void *arg), void *arg, int32u_t priority) {

	task->sblock_start	= sblock_start;
	task->sblock_size	= sblock_size;
	task->entry			= entry;
	task->arg			= arg;
	task->sp 			= _os_create_context(sblock_start, sblock_size, entry, arg);	
	task->node.priority = priority;
	task->node.ptr_data = task;
	task->state 		= READY;
	task->period 		= 0;
	task->alarm.tcb 	= task;
	task->alarm.alarm_queue_node.ptr_data = &(task->alarm);

	_os_add_node_tail(&(_os_ready_queue[priority]), &(task->node));
	_os_set_ready(priority);
	PRINT("task: 0x%x, priority: %d\n", (int32u_t)task, priority);
	return 0;
}

int32u_t eos_destroy_task(eos_tcb_t *task) {
}

void eos_schedule() {

	if (_os_current_task != NULL){
		addr_t sp = _os_save_context();
		if (sp == 0) return;
		_os_current_task->sp = sp;
		if (_os_current_task->state == RUNNING){
			int32u_t p = _os_current_task->node.priority;
			_os_add_node_tail(&_os_ready_queue[p], &(_os_current_task->node));
			_os_set_ready(p);
		}
	}

	int32u_t p = _os_get_highest_priority();
	_os_current_task = (eos_tcb_t *) (_os_ready_queue[p]->ptr_data);
	_os_current_task->state = RUNNING;
	_os_remove_node(&_os_ready_queue[p], &(_os_current_task->node));
	if (_os_ready_queue[p] == NULL) _os_unset_ready(p);
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
	task->period = period;
}

int32u_t eos_get_period(eos_tcb_t *task) {
}

int32u_t eos_suspend_task(eos_tcb_t *task) {
}

int32u_t eos_resume_task(eos_tcb_t *task) {
}

void eos_sleep(int32u_t tick) {
	if (_os_current_task->period == 0){
		eos_schedule();
		return;
	}

	_os_current_task->state = WAITING;
	_os_current_task->timeout = eos_get_system_timer()->tick + _os_current_task->period;
	//PRINT("set alarm at %d\n", _os_current_task->timeout);
	eos_set_alarm(eos_get_system_timer(),
				&(_os_current_task->alarm),
				_os_current_task->timeout,
				_os_wakeup_sleeping_task,
				_os_current_task);
	eos_schedule();
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
	eos_tcb_t * tcb = (eos_tcb_t *) arg;
	int32u_t p = tcb->node.priority;
	_os_add_node_tail(&_os_ready_queue[p], &(tcb->node));
	_os_set_ready(p);
	//PRINT("Wake up %d\n", p);
}