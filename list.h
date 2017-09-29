
#ifndef __LIST_H__
#define __LIST_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>
#include <pthread.h>


#if defined(MULTITHREAD_TEST) || defined(PRIV_LIST_TEST)
#define MAX_DATA_LEN 256
typedef struct can_udp_packet {
	unsigned int message_id;
	char message_len;
	char data[MAX_DATA_LEN];
} can_udp_packet_t, *can_udp_packet_p;

#else
#include "server_common.h"
#endif


typedef struct msg_node
{
	can_udp_packet_p udp_packet_p;
	struct msg_node *prev;
	struct msg_node *next;
} msg_node_t, *msg_node_p;


#define MAX(a,b) ((a)>(b))?(a):(b)

#define MAX_MSG_ID	10000
#define MIN_MSG_ID	1
#define MSG_NULL_ID	0
#define MSG_COUNT	MAX((MAX_MSG_ID - MIN_MSG_ID + 1), 2000)

#define MAX_MSG_LEN	100
#define MIN_MSG_LEN	20

#define MSG_STATE_BUSY	1
#define MSG_STATE_IDLE	0

#define LIST_STATE_INITED	1
#define LISG_STATE_UNINIT	0

#define log_dbg(fmt, args ...) printf("%s():%d -- " fmt, __func__, __LINE__, ##args)
#define log_err(fmt, args ...) printf("%s():%d -- Error! " fmt, __func__, __LINE__, ##args)
#define log_inf(fmt, args ...) printf("%s():%d -- " fmt, __func__, __LINE__, ##args)


msg_node_p alloc_new_node(void);
int free_node(msg_node_p node_p);

int list_append_node(msg_node_p node_p);
int list_insert_node(msg_node_p node_p, msg_node_p new_node_p);
int list_remove_node(msg_node_p node_p);

/**
 * list_fetch_node() self is executed with no lock, so it should be invoked in safe context.  
 **/
msg_node_p list_fetch_node(msg_node_p head_tail_node);
msg_node_p list_fetch_idle_node(void);

void list_print(msg_node_p head);
void list_reverse_print(msg_node_p tail);

int list_init(void);
int list_deinit(void);

int change_msg_state(unsigned int id, unsigned int state);



#endif // __LIST_H__
