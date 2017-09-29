

#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include "list.h"

#ifdef MULTITHREAD_TEST

#define msleep(x) usleep(x*1000)
#define THREAD_SYNC

sem_t msg_sem;
#ifdef THREAD_SYNC
pthread_mutex_t out_mutex;
pthread_cond_t out_cond;
#endif


void *consumer_thread(void *arg)
{
	msg_node_t *node_p;
	int msg_id = 0;
	int msg_data = 0;

#ifdef THREAD_SYNC
	pthread_cleanup_push(pthread_mutex_unlock, (void *)&out_mutex);
#endif

	while (1) {
		//printf("Consumer-%lu -- running\n", pthread_self());

#ifdef THREAD_SYNC
		pthread_mutex_lock(&out_mutex);
		pthread_cond_wait(&out_cond, &out_mutex);
		pthread_mutex_unlock(&out_mutex);
#endif

		//printf("Consumer-%lu -- try to find an idle node!\n", pthread_self());
		node_p = list_fetch_idle_node();

		if (node_p == NULL)
			continue;

		change_msg_state(node_p->udp_packet_p->message_id, MSG_STATE_BUSY);
		msg_id = node_p->udp_packet_p->message_id;
		msg_data = node_p->udp_packet_p->message_len;

		// handle the message
		printf("Consumer-%lu -- node: %d, %d\n", pthread_self(), msg_id, msg_data);
		sleep(2);
		free_node(node_p);
		change_msg_state(msg_id, MSG_STATE_IDLE);

#ifdef THREAD_SYNC
		// notify current id is idle and schedule threads
		pthread_cond_broadcast(&out_cond);
#endif
	}

#ifdef THREAD_SYNC
	pthread_cleanup_pop(0);
#endif
}

void *producer_thread(void *arg)
{
	msg_node_t *node_p;

	while (1) {
		printf("Producer -- running\n");

		sem_wait(&msg_sem);
		node_p = alloc_new_node();
		if (node_p == NULL)
			continue;

		node_p->udp_packet_p->message_id = (rand() % (MAX_MSG_ID-MIN_MSG_ID + 1)) + MIN_MSG_ID;
		node_p->udp_packet_p->message_len = (rand() % (MAX_MSG_LEN - MIN_MSG_LEN + 1)) + MIN_MSG_LEN;

		printf("Producer -- push a new node into list\n");
		list_append_node(node_p);
		list_print(NULL);

#ifdef THREAD_SYNC
		pthread_cond_broadcast(&out_cond);
#endif
	}

}


int main()
{
	pthread_t thrd1, thrd2, thrd3, thrd4, thrd5, thrd6, thrd7;
	int sleep_delay = 0;

	printf("multi-thread test with private list!\n");

	if (list_init() < 0) {
		printf("Error!!\n");
		goto err_exit;
	}

	srand((unsigned int) time(NULL));
	sem_init(&msg_sem, 0, 1);
#ifdef THREAD_SYNC
	pthread_mutex_init(&out_mutex, NULL);
	pthread_cond_init(&out_cond, NULL);
#endif

	pthread_create(&thrd1, NULL, (void*)consumer_thread, NULL);
	pthread_create(&thrd2, NULL, (void*)consumer_thread, NULL);
	pthread_create(&thrd3, NULL, (void*)consumer_thread, NULL);
	pthread_create(&thrd4, NULL, (void*)consumer_thread, NULL);
	pthread_create(&thrd5, NULL, (void*)consumer_thread, NULL);
	pthread_create(&thrd6, NULL, (void*)consumer_thread, NULL);
	pthread_create(&thrd7, NULL, (void*)producer_thread, NULL);

	do {
		sleep_delay = ((rand() % 8)+1) * 100;
		printf("main sleep %d ms\n", sleep_delay);
		msleep(sleep_delay);

		sem_post(&msg_sem);

	} while (1);

	sleep(2);

	sem_destroy(&msg_sem);
#ifdef THREAD_SYNC
	pthread_cond_destroy(&out_cond);
	pthread_mutex_destroy(&out_mutex);
#endif
	pthread_exit(0);

err_exit:
	list_deinit();
	return 0;
}

#endif // #ifdef MULTITHREAD_TEST

