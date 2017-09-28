
#ifndef __LIST_H__
#define __LIST_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <malloc.h>

typedef struct msg
{
	int id;
	int data;

	struct msg *priv;
	struct msg *next;
} List;


List *alloc_new_node(void);

void list_append_node(List *pnode);
void list_insert_node(List *pnode, List *new_node);
void list_remove_node(List *pnode);
List * list_fetch_node(List *head_tail_node);

void list_print(List *head);
void list_reverse_print(List *tail);

int list_test(void);

#endif // __LIST_H__
