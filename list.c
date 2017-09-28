
#include "list.h"


static int list_num = 0;

List *list_head = NULL;
List *list_tail = NULL;


List *alloc_new_node(void)
{
	List *p1;

	p1 = (List *) malloc(sizeof(List));
	if (p1 == NULL) {
		printf("out of memory\n");
	}

	return p1;
}

void list_append_node(List *pnode)
{

	if (pnode == NULL) {
		return;
	}

	pnode->priv = list_tail;
	pnode->next = NULL;

	if (list_head == NULL)
		list_head = pnode;

	if (list_tail != NULL)
		list_tail->next = pnode;

	list_tail = pnode;
}

void list_insert_node(List *pnode, List *new_node)
{
	if (pnode->priv == NULL)
		list_head = new_node;
	else
		pnode->priv->next = new_node;

	new_node->priv = pnode->priv;
	new_node->next = pnode;
	pnode->priv = new_node;
}

void list_remove_node(List *pnode)
{
	if (pnode == NULL)
		return;

	if ((pnode->priv == NULL) && (pnode->next == NULL)) {
		list_head = NULL;
		list_tail = NULL;
		free(pnode);
		return;
	}

	if (pnode->priv == NULL) {
		list_head = pnode->next;
		pnode->next->priv = NULL;
	} else if (pnode->next == NULL) {
		list_tail = pnode->priv;
		pnode->priv->next = NULL;
	} else {
		pnode->priv->next = pnode->next;
		pnode->next->priv = pnode->priv;
	}

	free(pnode);
}

List * list_fetch_node(List *head_tail_node)
{
	List *pnode = head_tail_node;

	if (pnode == NULL)
		return pnode;

	// only one node in list
	if ((pnode->priv == NULL) && (pnode->next == NULL)) {
		list_head = NULL;
		list_tail = NULL;
		return pnode;
	}

	if (pnode->priv == NULL) {
		// head node
		list_head = pnode->next;
		pnode->next->priv = NULL;
	} else if (pnode->next == NULL) {
		// tail node
		list_tail = pnode->priv;
		pnode->priv->next = NULL;
	} else {
		pnode->priv->next = pnode->next;
		pnode->next->priv = pnode->priv;
	}

	return pnode;
}


void list_print(List *head)
{
	List *p = head;

	printf("dump list with normal order:\n");
	while (p != NULL) {
		printf("[%d, %d]->", p->id, p->data);
		p = p->next;
	}

	printf("\nover!\n\n");
}

void list_reverse_print(List *tail)
{
	List *p = tail;

	printf("dump list with reverse order:\n");
	while (p != NULL) {
		printf("[%d, %d]->", p->id, p->data);
		p = p->priv;
	}

	printf("\nover!\n\n");
}


static void list_create(void)
{
	List *p1;

	p1 = (List *) malloc(sizeof(List));
	if (p1 == NULL) {
		printf("out of memory\n");
		return;
	}

	printf("Please print the data you want:(end is 0)\n");
	scanf("%d", &p1->data);

	while (p1->data != 0)
	{
		list_num++;
		p1->id = list_num;
		if (list_num == 1) {
			list_head = p1;
			p1->priv = NULL;
			p1->next = NULL;
		} else {
			list_tail->next = p1;

			p1->priv = list_tail;
			p1->next = NULL;
		}

		list_tail = p1;
		p1 = (List *) malloc(sizeof(List));
		if (p1 == NULL) {
			printf("out of memory\n");
			return;
		}
		scanf("%d", &p1->data);
	}

	free(p1);
}

static void list_create_append(void)
{
	List *p1;

	p1 = (List *) malloc(sizeof(List));
	if (p1 == NULL) {
		printf("out of memory\n");
		return;
	}

	printf("Please print the data you want:(end is 0)\n");
	scanf("%d", &p1->data);

	while (p1->data != 0) {
		p1->id = ++list_num;
		list_append_node(p1);

		p1 = (List *) malloc(sizeof(List));
		if (p1 == NULL) {
			printf("out of memory\n");
			return;
		}

		scanf("%d", &p1->data);
	}

	free(p1);
}

static void list_create_insert_head(void)
{
	List *p1;

	p1 = (List *) malloc(sizeof(List));
	if (p1 == NULL) {
		printf("out of memory\n");
		return;
	}

	printf("Please print the data you want:(end is 0)\n");
	scanf("%d", &p1->data);

	while (p1->data != 0) {
		p1->id = ++list_num;
		list_insert_node(list_head, p1);

		p1 = (List *) malloc(sizeof(List));
		if (p1 == NULL) {
			printf("out of memory\n");
			return;
		}

		scanf("%d", &p1->data);
	}

	free(p1);
}

static void list_create_insert_tail(void)
{
	List *p1;

	p1 = (List *) malloc(sizeof(List));
	if (p1 == NULL) {
		printf("out of memory\n");
		return;
	}

	printf("Please print the data you want:(end is 0)\n");
	scanf("%d", &p1->data);

	while (p1->data != 0) {
		p1->id = ++list_num;
		list_insert_node(list_tail, p1);

		p1 = (List *) malloc(sizeof(List));
		if (p1 == NULL) {
			printf("out of memory\n");
			return;
		}

		scanf("%d", &p1->data);
	}

	free(p1);
}




int list_test(void)
//int main(void)
{
	printf("\n\nnormally create list:\n");
	list_create();
	list_print(list_head);
	list_reverse_print(list_tail);

	printf("\n\ncreate list with appending mode:\n");
	list_create_append();
	list_print(list_head);
	list_reverse_print(list_tail);

	printf("\n\ncreate list with inserting head:\n");
	list_create_insert_head();
	list_print(list_head);
	list_reverse_print(list_tail);

	printf("\n\ncreate list with inserting tail:\n");
	list_create_insert_tail();
	list_print(list_head);
	list_reverse_print(list_tail);

	printf("\n\nremove node from head:\n");
	while (list_num > 0) {
		list_remove_node(list_head);
		list_print(list_head);
		list_reverse_print(list_tail);

		list_num--;
	}

	return 0;
}


