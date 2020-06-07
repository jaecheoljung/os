/********************************************************
 * Filename: core/comm.c
 *  
 * Author: jtlim, RTOSLab. SNU.
 * 
 * Description: message queue management. 
 ********************************************************/
#include <core/eos.h>

void eos_init_mqueue(eos_mqueue_t *mq, void *queue_start, int16u_t queue_size, int8u_t msg_size, int8u_t queue_type) {
	mq->queue_start = queue_start;
	mq->queue_size = queue_size;
	mq->msg_size = msg_size;
	mq->queue_type = queue_type;
	mq->front = queue_start;
	mq->rear = queue_start;
	eos_init_semaphore(&(mq->putsem), queue_size, queue_type);
	eos_init_semaphore(&(mq->getsem), 0, queue_type);
}

int8u_t eos_send_message(eos_mqueue_t *mq, void *message, int32s_t timeout) {

	int32u_t acq = eos_acquire_semaphore(&(mq->putsem), timeout);
	
	if (acq == 0) return 0;

	for (int16u_t i = 0; i<mq->msg_size; i++){

		*(int8u_t *) mq->rear = *(int8u_t *) (message + i);
		mq->rear = (int8u_t *) mq->rear + 1;

		if ((int8u_t *) mq->rear - (int8u_t *) mq->queue_start == (mq->queue_size) * (mq->msg_size)){
			mq->rear = mq->queue_start;
		}
	}

	eos_release_semaphore(&(mq->getsem));

	return 1;
}

int8u_t eos_receive_message(eos_mqueue_t *mq, void *message, int32s_t timeout) {

	int32u_t acq = eos_acquire_semaphore(&(mq->getsem), timeout);

	if (acq == 0) return 0;

	for (int16u_t i = 0; i<mq->msg_size; i++){

		*(int8u_t *) (message + i) = *(int8u_t *) mq->front;
		mq->front = (int8u_t *) mq->front + 1;

		if ((int8u_t *) mq->front - (int8u_t *) mq->queue_start == (mq->queue_size) * (mq->msg_size)){
			mq->front = mq->queue_start;
		}
	}

	eos_release_semaphore(&(mq->putsem));

	return 1;
}
