
#include "list.h"


static unsigned int list_num;
static unsigned int list_state;
static unsigned char *msg_state;

msg_node_p list_head = NULL;
msg_node_p list_tail = NULL;

pthread_mutex_t list_mutex;
pthread_mutex_t list_state_mutex;


msg_node_p alloc_new_node(void)
{
	msg_node_p node_p;
	can_udp_packet_p packet_p;

	node_p = (msg_node_p) malloc(sizeof(msg_node_t));
	if (node_p == NULL) {
		log_err("No enough memory to 'List'\n");
		return NULL;
	}

	packet_p = (can_udp_packet_p) malloc(sizeof(can_udp_packet_t));
	if (packet_p == NULL) {
		log_err("No enough memory to 'can_udp_packet_p'\n");
		free(node_p);
		return NULL;
	}

	node_p->udp_packet_p = packet_p;

	return node_p;
}

int free_node(msg_node_p node_p)
{
	if (node_p == NULL) {
		return -1;
	}

	if (node_p->udp_packet_p != NULL) {
		free(node_p->udp_packet_p);
		node_p->udp_packet_p = NULL;
	}

	free(node_p);
	node_p = NULL;

	return 0;
}

int list_append_node(msg_node_p node_p)
{

	if (node_p == NULL) {
		return -1;
	}

	if (list_state != LIST_STATE_INITED) {
		log_err("invalid list state %u\n", list_state);
		return -1;
	}

	pthread_mutex_lock(&list_mutex);
	node_p->prev = list_tail;
	node_p->next = NULL;

	if (list_head == NULL)
		list_head = node_p;

	if (list_tail != NULL)
		list_tail->next = node_p;

	list_tail = node_p;
	pthread_mutex_unlock(&list_mutex);
	return 0;
}

int list_insert_node(msg_node_p node_p, msg_node_p new_node_p)
{
	if (list_state != LIST_STATE_INITED) {
		log_err("invalid list state %u\n", list_state);
		return -1;
	}

	pthread_mutex_lock(&list_mutex);
	if (node_p->prev == NULL)
		list_head = new_node_p;
	else
		node_p->prev->next = new_node_p;

	new_node_p->prev = node_p->prev;
	new_node_p->next = node_p;
	node_p->prev = new_node_p;
	pthread_mutex_unlock(&list_mutex);
	return 0;
}

int list_remove_node(msg_node_p node_p)
{
	if (node_p == NULL)
		return -1;

	if (list_state != LIST_STATE_INITED) {
		log_err("invalid list state %u\n", list_state);
		return -1;
	}

	pthread_mutex_lock(&list_mutex);
	if ((node_p->prev == NULL) && (node_p->next == NULL)) {
		// only one node in list
		list_head = NULL;
		list_tail = NULL;
	} else if (node_p->prev == NULL) {
		// head node
		list_head = node_p->next;
		node_p->next->prev = NULL;
	} else if (node_p->next == NULL) {
		// tail node
		list_tail = node_p->prev;
		node_p->prev->next = NULL;
	} else {
		node_p->prev->next = node_p->next;
		node_p->next->prev = node_p->prev;
	}
	pthread_mutex_unlock(&list_mutex);

	return free_node(node_p);
}

msg_node_p list_fetch_node(msg_node_p head_tail_node)
{
	msg_node_p node_p = head_tail_node;

	if (node_p == NULL)
		return NULL;

	if (list_state != LIST_STATE_INITED) {
		log_err("invalid list state %u\n", list_state);
		return NULL;
	}

	// only one node in list
	if ((node_p->prev == NULL) && (node_p->next == NULL)) {
		list_head = NULL;
		list_tail = NULL;
		return node_p;
	}

	if (node_p->prev == NULL) {
		// head node
		list_head = node_p->next;
		node_p->next->prev = NULL;
	} else if (node_p->next == NULL) {
		// tail node
		list_tail = node_p->prev;
		node_p->prev->next = NULL;
	} else {
		node_p->prev->next = node_p->next;
		node_p->next->prev = node_p->prev;
	}

	return node_p;
}

msg_node_p list_fetch_idle_node(void)
{
	msg_node_p node_p;
	unsigned int msg_id;

	pthread_mutex_lock(&list_mutex);
	node_p = list_head;
	if ((node_p == NULL) || (node_p->udp_packet_p == NULL))
		goto err_exit;

	msg_id = node_p->udp_packet_p->message_id;
	if ((msg_id < MIN_MSG_ID) || (msg_id > MAX_MSG_ID)) {
		log_err("invalid msg-id %d\n", msg_id);
		goto err_exit;
	}

	if (list_state != LIST_STATE_INITED) {
		log_err("invalid list state %u\n", list_state);
		goto err_exit;
	}

	pthread_mutex_lock(&list_state_mutex);
	while (msg_state[msg_id-1] != MSG_STATE_IDLE) {

		log_inf("Busy node (%u, %d)\n\n", msg_id, node_p->udp_packet_p->message_len);
		node_p = node_p->next;

		if ((node_p == NULL) || (node_p->udp_packet_p == NULL)) {
			node_p = NULL;
			break;
		}
	}

	if (node_p != NULL) {
		// get an idle node and change its state as busy
		msg_state[msg_id-1] = MSG_STATE_BUSY;
	}
	pthread_mutex_unlock(&list_state_mutex);

	if (node_p != NULL) {
		// fetch the node from list
		node_p = list_fetch_node(node_p);
	}

	pthread_mutex_unlock(&list_mutex);
	return node_p;

err_exit:
	pthread_mutex_unlock(&list_mutex);
	return NULL;
}

void list_print(msg_node_p head)
{
	msg_node_p node_p = head;

	if (node_p == NULL)
		node_p = list_head;

	printf("dump list:\n");
	pthread_mutex_lock(&list_mutex);
	while (node_p != NULL) {
		printf("[%d, %d, %p]->", \
			node_p->udp_packet_p->message_id, \
			node_p->udp_packet_p->message_len, \
			node_p->udp_packet_p->data);
		node_p = node_p->next;
	}
	pthread_mutex_unlock(&list_mutex);
	printf(" over!\n\n");
}

void list_reverse_print(msg_node_p tail)
{
	msg_node_p node_p = tail;

	if (node_p == NULL)
		node_p = list_tail;

	printf("dump inverted list:\n");
	pthread_mutex_lock(&list_mutex);
	while (node_p != NULL) {
		printf("[%d, %d, %p]->", \
			node_p->udp_packet_p->message_id, \
			node_p->udp_packet_p->message_len, \
			node_p->udp_packet_p->data);
		node_p = node_p->prev;
	}
	pthread_mutex_unlock(&list_mutex);
	printf(" over!\n\n");
}

int list_init(void)
{
	msg_state = (unsigned char *) malloc(MSG_COUNT);
	if (msg_state == NULL) {
		log_err("No enough memeory to \'msg_state\'\n");
		return -1;
	}

	memset(msg_state, MSG_STATE_IDLE, MSG_COUNT);

	list_head = NULL;
	list_tail = NULL;
	pthread_mutex_init(&list_mutex, NULL);
	pthread_mutex_init(&list_state_mutex, NULL);
	list_state = LIST_STATE_INITED;

	return 0;
}

int list_deinit(void)
{
	list_state = LISG_STATE_UNINIT;

	pthread_mutex_destroy(&list_mutex);
	pthread_mutex_destroy(&list_state_mutex);

	while (list_head != NULL) {
		list_remove_node(list_head);
	}

	if (msg_state != NULL)
		free(msg_state);

	return 0;
}

int change_msg_state(unsigned int id, unsigned int state)
{
	if (list_state != LIST_STATE_INITED) {
		log_err("invalid list state %u\n", list_state);
		return -1;
	}

	if ((id < MIN_MSG_ID) || (id > MAX_MSG_ID)) {
		log_err("invalid msg-id %u\n", id);
		return -1;
	}

	if ((state != MSG_STATE_BUSY) && (state != MSG_STATE_IDLE)) {
		log_err("invalid msg-state %u\n", state);
		return -1;
	}

	pthread_mutex_lock(&list_state_mutex);
	msg_state[id-1] = state;
	pthread_mutex_unlock(&list_state_mutex);

	return 0;
}



#ifdef PRIV_LIST_TEST
#define LIST_APPEND_MODE_APPENDING	0
#define LIST_APPEND_MODE_INSERT_BF_HEAD	1
#define LIST_APPEND_MODE_INSERT_BF_TAIL	2

static void list_create(int mode)
{
	msg_node_p node_p;

	if (list_state != LIST_STATE_INITED) {
		log_err("invalid list state %u\n", list_state);
		return;
	}

	printf("Please print the data you want: (end is 0)\n");
	while (1) {

		node_p = alloc_new_node();
		if (node_p == NULL) {
			return;
		}

		scanf("%d", (int *) &node_p->udp_packet_p->message_len);

		if (node_p->udp_packet_p->message_len == 0)
			break;

		list_num++;
		node_p->udp_packet_p->message_id = list_num;

		switch (mode) {
			case LIST_APPEND_MODE_INSERT_BF_HEAD:
				list_insert_node(list_head, node_p);
				break;
			case LIST_APPEND_MODE_INSERT_BF_TAIL:
				list_insert_node(list_tail, node_p);
				break;
			case LIST_APPEND_MODE_APPENDING:
			default:
				list_append_node(node_p);
				break;
		}
	}

	free_node(node_p);
}

int main(void)
{
	if (list_init() < 0) {
		log_err("fail to init list\n");
		return -1;
	}

	printf("\n\ncreate list (appending):\n");
	list_create(LIST_APPEND_MODE_APPENDING);
	list_print(list_head);
	list_reverse_print(list_tail);

	printf("\n\ncreate list (head-inserted):\n");
	list_create(LIST_APPEND_MODE_INSERT_BF_HEAD);
	list_print(list_head);
	list_reverse_print(list_tail);

	printf("\n\ncreate list (tail-inserted):\n");
	list_create(LIST_APPEND_MODE_INSERT_BF_TAIL);
	list_print(list_head);
	list_reverse_print(list_tail);

	printf("\n\nremove node from head:\n");
	while (list_num > 0) {
		list_remove_node(list_head);
		list_print(list_head);
		list_reverse_print(list_tail);

		list_num--;
	}

	list_deinit();
	return 0;
}
#endif // #ifdef PRIV_LIST_TEST

