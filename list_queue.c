
#include "list.h"

/*
pthread_mutex_t q_mutex;
pthread_cond_t q_cond;
int waiting_threads;
*/

int priv_queue_init(void)
{
	printf("%s:%d\n", __func__, __LINE__);
/*
	pthread_mutex_init(&q_mutex, NULL);
	pthread_cond_init(&q_cond, NULL);
	waiting_threads = 0;
*/
	return list_init();
}

int priv_queue_push(void *data)
{
	msg_node_p node_p;

	node_p = alloc_new_node();
	if (node_p == NULL) {
		return -1;
	}

	node_p->udp_packet_p = (can_udp_packet_p)data;
	list_append_node(node_p);

/*
	pthread_mutex_lock(&q_mutex);
	if (waiting_threads > 0) {
		pthread_cond_signal(&q_cond);
	}
	pthread_mutex_unlock(&q_mutex);
*/
	return 0;
}

int priv_queue_pop(void **data, unsigned int msg_id)
{
	msg_node_p node_p = NULL;

	if (msg_id != MSG_NULL_ID) {
		change_msg_state(msg_id, MSG_STATE_IDLE);
	}

	while (node_p == NULL) {
/*
		pthread_mutex_lock(&q_mutex);
		pthread_cond_wait(&q_cond, &q_mutex);
		pthread_mutex_unlock(&q_mutex);
*/
		node_p = list_fetch_idle_node();
	}

	*data = (void *) node_p->udp_packet_p;
	free(node_p);
	return 0;
}

int priv_queue_update(unsigned int msg_id)
{
	if (msg_id != MSG_NULL_ID) {
		change_msg_state(msg_id, MSG_STATE_IDLE);
	}

	return 0;
}

struct queue_ops glib_queue_ops = {
	.init = priv_queue_init,
	.push = priv_queue_push,
	.pop = priv_queue_pop,
	.update = priv_queue_update
};
