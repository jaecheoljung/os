/********************************************************
 * Filename: core/timer.c
 *
 * Author: wsyoo, RTOSLab. SNU.
 * 
 * Description: 
 ********************************************************/
#include <core/eos.h>

static eos_counter_t system_timer;

int8u_t eos_init_counter(eos_counter_t *counter, int32u_t init_value) {
	counter->tick = init_value;
	counter->alarm_queue = NULL;
	return 0;
}

void eos_set_alarm(eos_counter_t* counter, eos_alarm_t* alarm, int32u_t timeout, void (*entry)(void *arg), void *arg) {
	alarm->timeout = timeout;
	alarm->alarm_queue_node.priority = timeout;
	alarm->handler = entry;
	alarm->arg = arg;
	_os_add_node_priority(&(counter->alarm_queue), &(alarm->alarm_queue_node));
}

eos_counter_t* eos_get_system_timer() {
	return &system_timer;
}

void eos_trigger_counter(eos_counter_t* counter) {
	counter->tick ++;
	PRINT("tick %d\n", counter->tick);
	while(counter->alarm_queue != NULL){
		eos_alarm_t * alarm = (eos_alarm_t *) (counter->alarm_queue->ptr_data);
		if (alarm->timeout != counter->tick) break;
		(alarm->handler)(alarm->arg);
		_os_remove_node(&(counter->alarm_queue), counter->alarm_queue);
	}
	eos_schedule();
}

/* Timer interrupt handler */
static void timer_interrupt_handler(int8s_t irqnum, void *arg) {
	/* trigger alarms */
	eos_trigger_counter(&system_timer);
}

void _os_init_timer() {
	eos_init_counter(&system_timer, 0);

	/* register timer interrupt handler */
	eos_set_interrupt_handler(IRQ_INTERVAL_TIMER0, timer_interrupt_handler, NULL);
}
