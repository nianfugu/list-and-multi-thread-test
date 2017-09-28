
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include "list.h"

#define msleep(x) usleep(x*1000)


#define SEMAPHORE_POP_USED
#undef SEMAPHORE_POP_USED

#define MAX_MSG_ID	30
#define MIN_MSG_ID	1
#define MSG_COUNT	(MAX_MSG_ID - MIN_MSG_ID + 1)

#define MAX_MSG_DATA	100
#define MIN_MSG_DATA	20

#define MSG_STATE_BUSY	1
#define MSG_STATE_IDLE	0

extern List *list_head;
extern List *list_tail;

sem_t msg_sem;

#ifdef SEMAPHORE_POP_USED
sem_t msg_pushed_sem;
#else
pthread_mutex_t out_mutex;
pthread_cond_t out_cond;
#endif

pthread_mutex_t msg_mutex;
pthread_mutex_t msg_state_mutex;

unsigned char msg_state[MSG_COUNT] = {0};

static int change_msg_state(int id, int state)
{
	if ((id < MIN_MSG_ID) || (id > MAX_MSG_ID)) {
		printf("Error! invalid msg-id %d\n", id);
		return -1;
	}

	if ((state != MSG_STATE_BUSY) && (state != MSG_STATE_IDLE)) {
		printf("Error! invalid msg-state %d\n", state);
		return -1;
	}

	pthread_mutex_lock(&msg_state_mutex);
	msg_state[id - 1] = state;
	pthread_mutex_unlock(&msg_state_mutex);

	return 0;
}

static List * fetch_idle_node(List *head)
{
	List * pnode = head;

	if (pnode == NULL)
		return pnode;

	if ((pnode->id < MIN_MSG_ID) || (pnode->id > MAX_MSG_ID)) {
		printf("Error! invalid msg-id %d\n", pnode->id);
		return NULL;
	}

	pthread_mutex_lock(&msg_state_mutex);
	while (msg_state[pnode->id - 1] != MSG_STATE_IDLE) {

		printf("\nBusy node -- (%d, %d)\n", pnode->id, pnode->data);
		pnode = pnode->next;

		if (pnode == NULL)
			break;
	}
	pthread_mutex_unlock(&msg_state_mutex);

	pnode = list_fetch_node(pnode);
	return pnode;
}

void *consumer_thread(void *arg)
{
	List *pnode;
	int msg_id = MAX_MSG_ID+1;
	int msg_data = 0;

#ifndef SEMAPHORE_POP_USED
	pthread_cleanup_push(pthread_mutex_unlock, (void *)&out_mutex);
#endif
	pthread_cleanup_push(pthread_mutex_unlock, (void *)&msg_mutex);

	while(1){
		//printf("Consumer-%lu -- running\n", pthread_self());

#ifdef SEMAPHORE_POP_USED
		sem_wait(&msg_pushed_sem);
#else
		pthread_mutex_lock(&out_mutex);
		pthread_cond_wait(&out_cond, &out_mutex);
		pthread_mutex_unlock(&out_mutex);
#endif

		//printf("Consumer-%lu -- try to find an idle node!\n", pthread_self());
		pthread_mutex_lock(&msg_mutex);
		pnode = fetch_idle_node(list_head);
		pthread_mutex_unlock(&msg_mutex);

		if (pnode == NULL) {
			//printf("Consumer-%lu -- Idle node NOT found!\n", pthread_self());
		} else {
			change_msg_state(pnode->id, MSG_STATE_BUSY);
			msg_id = pnode->id;
			msg_data = pnode->data;

			// handle the message
			printf("Consumer-%lu -- node: %d, %d\n", pthread_self(), pnode->id, pnode->data);
			free(pnode);

			sleep(2);

			change_msg_state(msg_id, MSG_STATE_IDLE);
			//printf("Consumer-%lu -- node: %d, %d\n", pthread_self(), msg_id, msg_data);

			// notify current id is idle and schedule threads
#ifdef SEMAPHORE_POP_USED
			sem_post(&msg_pushed_sem);
#else
			pthread_cond_broadcast(&out_cond);
#endif
		}

		//pthread_mutex_unlock(&out_mutex);
	}

	pthread_cleanup_pop(0);
#ifndef SEMAPHORE_POP_USED
	pthread_cleanup_pop(0);
#endif
}

void *producer_thread(void *arg)
{

	List *pnode;

	pthread_cleanup_push(pthread_mutex_unlock, (void *)&msg_mutex);

	while(1){
		printf("Producer -- running\n");

		sem_wait(&msg_sem);
		pnode = alloc_new_node();
		if (pnode == NULL) {
			printf("Producer -- No enough memory for new node\n");
		} else {
			//pnode->id = MIN_MSG_ID;
			pnode->id = (rand() % (MAX_MSG_ID-MIN_MSG_ID + 1)) + MIN_MSG_ID;
			pnode->data = (rand() % (MAX_MSG_DATA-MIN_MSG_DATA + 1)) + MIN_MSG_DATA;

			printf("Producer -- push a new node into list\n");
			pthread_mutex_lock(&msg_mutex);
			list_append_node(pnode);
			list_print(list_head);
			pthread_mutex_unlock(&msg_mutex);

#ifdef SEMAPHORE_POP_USED
			sem_post(&msg_pushed_sem);
#else
			pthread_cond_broadcast(&out_cond);
#endif
		}
	}

	pthread_cleanup_pop(0);
}


int main()
{
	pthread_t thrd1, thrd2, thrd3, thrd4, thrd5, thrd6, thrd7;
	int sleep_delay;

	printf("condition variable study!\n");

	srand((unsigned int) time(NULL));

	sem_init(&msg_sem, 0, 1);

#ifdef SEMAPHORE_POP_USED
	sem_init(&msg_pushed_sem, 0, 1);
#else
	pthread_mutex_init(&out_mutex, NULL);
	pthread_cond_init(&out_cond, NULL);
#endif

	pthread_mutex_init(&msg_mutex, NULL);
	pthread_mutex_init(&msg_state_mutex, NULL);


	pthread_create(&thrd1, NULL, (void*)consumer_thread, NULL);
	pthread_create(&thrd2, NULL, (void*)consumer_thread, NULL);
	pthread_create(&thrd3, NULL, (void*)consumer_thread, NULL);
	pthread_create(&thrd4, NULL, (void*)consumer_thread, NULL);
	pthread_create(&thrd5, NULL, (void*)consumer_thread, NULL);
	pthread_create(&thrd6, NULL, (void*)consumer_thread, NULL);
	pthread_create(&thrd7, NULL, (void*)producer_thread, NULL);

	do {
		//pthread_cond_signal(&out_cond);
		//pthread_cond_broadcast(&out_cond);

		//sem_post(&msg_sem);

#if 0
		sleep_delay = (rand() % 4)+1;
		printf("main sleep %d\n", sleep_delay);
		sleep(sleep_delay);
#else
		sleep_delay = ((rand() % 8)+1) * 100;
		printf("main sleep %d ms\n", sleep_delay);
		msleep(sleep_delay);
#endif

		sem_post(&msg_sem);

	} while (1);

	sleep(2);

	sem_destroy(&msg_sem);
	pthread_mutex_destroy(&msg_mutex);
	pthread_mutex_destroy(&msg_state_mutex);

#ifdef SEMAPHORE_POP_USED
	sem_destroy(&msg_pushed_sem);
#else
	pthread_cond_destroy(&out_cond);
	pthread_mutex_destroy(&out_mutex);
#endif

	pthread_exit(0);

	return 0;
}


