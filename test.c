/**@brief 练习使用linux内核链表，功能包括：
 * 定义链表结构，创建链表、插入节点、删除节点、移动节点、遍历节点
 *
 *@auther Anker @date 2013-12-15
 **/
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <errno.h>
#include "test.h"
//定义app_info链表结构
typedef struct application_info
{
    uint32_t  app_id;
    uint32_t  up_flow;
    uint32_t  down_flow;
    struct    list_head app_info_node;//链表节点
} app_info;


app_info* get_app_info(uint32_t app_id, uint32_t up_flow, uint32_t down_flow)
{
    app_info *app = (app_info*)malloc(sizeof(app_info));
    if (app == NULL)
    {
	fprintf(stderr, "Failed to malloc memory, errno:%u, reason:%s\n",
	errno, strerror(errno));
	return NULL;
    }
    app->app_id = app_id;
    app->up_flow = up_flow;
    app->down_flow = down_flow;
    return app;
}

static void for_each_app(const struct list_head *head)
{
    struct list_head *pos;
    app_info *app;
    //遍历链表
    list_for_each(pos, head)
    {
	app = list_entry(pos, app_info, app_info_node);
	printf("ap_id: %u\tup_flow: %u\tdown_flow: %u\n",
	app->app_id, app->up_flow, app->down_flow);

    }
}

void destroy_app_list(struct list_head *head)
{
    struct list_head *pos = head->next;
    struct list_head *tmp = NULL;
    while (pos != head)
    {
	tmp = pos->next;
	list_del(pos);
	pos = tmp;
    }
}


int main()
{
    //创建一个app_info
    app_info * app_info_list = (app_info*)malloc(sizeof(app_info));
    app_info *app;

    if (app_info_list == NULL)
    {
	fprintf(stderr, "Failed to malloc memory, errno:%u, reason:%s\n",
	errno, strerror(errno));
	return -1;
    }

    //初始化链表头部
    struct list_head *head = &app_info_list->app_info_node;
    init_list_head(head);

    //插入三个app_info
    app = get_app_info(1001, 100, 200);
    list_add_tail(&app->app_info_node, head);
    app = get_app_info(1002, 80, 100);
    list_add_tail(&app->app_info_node, head);
    app = get_app_info(1003, 90, 120);
    list_add_tail(&app->app_info_node, head);
    printf("After insert three app_info: \n");

    for_each_app(head);

    //将第一个节点移到末尾
    printf("Move first node to tail:\n");
    list_move_tail(head->next, head);
    for_each_app(head);

    //删除最后一个节点
    printf("Delete the last node:\n");
    list_del(head->prev);
    for_each_app(head);

    destroy_app_list(head);
    free(app_info_list);
    return 0;
}
