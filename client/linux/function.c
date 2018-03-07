﻿#include"function.h"

INFORMATION_QUEUE* pSend_queue;
INFORMATION_QUEUE* receive_queue;


//功能选择
void function(int sockfd)
{
	int rt;
	pthread_t send_thread, receive_thread;
	
	pSend_queue = init_queue();
	receive_queue = init_queue();

	PARAMETER  parameter;
	parameter.callBack = handler_receive;
	parameter.sockfd = sockfd;
	
	pthread_create(&send_thread, NULL, send, (void *)sockfd);
	pthread_create(&receive_thread, NULL, receive, (void *)&parameter);
	
	//显示注册登陆菜单
	login_menu();
	while (-1 == (rt = log_in()));

	//显示主菜单
	while(1)
	{
		main_menu();
		choose_function();
	}	
}

/*处理接收消息函数:
* 11:注册成功,  10:注册失败.   22:登陆成功,  20:登陆失败
* 33:收到单聊信息,  44:查找到好友  40:查找好友失败
*/
int handler_receive(void)
{
	AGREEMENT receive_data;

	memset(&receive_data, 0, sizeof(AGREEMENT));
	if(false == dequeue(receive_queue, &receive_data))
	{
		printf("消息出队队失败!");
		return -1;
	}
	switch(receive_data.order)
	{
		case 11:
			printf("   注册成功, 您的账号是: %d\n", receive_data.friend_id);
			break;
		case 10:
			printf("   注册失败，请重新注册!\n");
			break;
		case 22:
			printf("   登陆成功!\n");
			break;
		case 20:
			printf("   登陆失败! 请重新登陆!\n");
			break;
		case 33:
			printf("   收到%s(%d)发来的消息: %s\n", 
				  	receive_data.friend_nickname, receive_data.friend_id,
				  	receive_data.information);
			break;
		case 44:
			//printf("   好友列表:\n");
			
			
			break;
		case 40:

			break;
	}
}


//发送线程
void *_send(void * arg)
{
	int cnt;
	int sockfd = (int)arg;
	fd_set wset;
	struct timeval timevalue = {1, 0};
	AGREEMENT send_data;
	int send_data_len = sizeof(AGREEMENT);

	while(1)
	{
		// 清空描述符集	
		FD_ZERO(&wset);		

		// 设置描述符到描述符集
		FD_SET(sockfd, &wset);
		//检测是否有消息可读
		while( (-1 == (cnt = select(sockfd+1,NULL, &wset,
				NULL, &timevalue))) && (EINTR == errno));
		if(-1 == cnt)
		{
			perror("监测可写消息失败!");
			sleep(1);
			continue;
		}

		memset(&send_data, 0, send_data_len);

		//消息出队
		if(false == dequeue(pSend_queue, &send_data))
		{
			printf("消息出队队失败!");
			continue;
		}		
		cnt = recv(sockfd, &send_data, send_data_len, 0);
		if(-1 == cnt)
		{
			printf("发送消息失败!\n");
		}
		else if(cnt > 0)
		{
			printf("发送请求(%d)成功!\n", send_data.order);
		}
		else{
			printf("发送了0个消息!\n");
		}		
	}	
}

//接收线程
void *receive(void *arg)
{
	int cnt, num = 3;
	PARAMETER *parameter = (PARAMETER *)arg;
	fd_set rset;
	struct timeval timevalue = {1, 0};
	AGREEMENT recv_data;
	int recv_data_len = sizeof(AGREEMENT);
	
	while(1)
	{
		// 清空描述符集	
		FD_ZERO(&rset);

		// 设置描述符到描述符集
		FD_SET(parameter->sockfd, &rset);
		//检测是否有消息可读
		while( (-1 == (cnt = select(parameter->sockfd+1, &rset, NULL,
				 NULL, &timevalue))) && (EINTR == errno));
		if(-1 == cnt)
		{
			perror("监测可读消息失败!");
			sleep(1);
			continue;
		}

		memset(&recv_data, 0, recv_data_len);
		cnt = recv(parameter->sockfd, &recv_data, recv_data_len, 0);
		if(-1 == cnt)
		{
			printf("读消息失败!\n");
			continue;
		}
		else if(cnt > 0)
		{
			printf("接收到服务器消息(回应命令%d): %s\n", recv_data.order,
				recv_data.information);
			
			//消息入队
			if(false == enqueue(receive_queue, recv_data))
			{
				printf("消息入队失败!");
				continue;
			}

			parameter->callBack();
		}
	}
}


//初始化队列
INFORMATION_QUEUE *init_queue(void)
{
	INFORMATION_QUEUE *pQueue = (INFORMATION_QUEUE *)malloc(sizeof(INFORMATION_QUEUE));
	if(NULL == pQueue)
	{
		perror("分配队列头内存失败!\n");
		return NULL;
	}
	pQueue->front = pQueue->rear = (QUEUE_NODE *)malloc(sizeof(QUEUE_NODE));
	if(NULL == pQueue->rear)
	{
		perror("分配队列头节点内存失败!\n");
		return NULL;
	}
	memset(&pQueue->front->data, 0, sizeof(AGREEMENT));
	pQueue->rear->pNext = NULL;
	
	return pQueue;
}
//入队
bool enqueue(INFORMATION_QUEUE *pQueue, AGREEMENT data)
{
	QUEUE_NODE *pNew = (QUEUE_NODE *)malloc(sizeof(QUEUE_NODE));
	if(NULL == pNew)
	{
		perror("分配节点内存失败!\n");
		return false;
	}
	pNew->data = data;
	pNew->pNext = NULL;
	pQueue->rear->pNext = pNew;
	pQueue->rear = pNew;

	return true;
}

//出队
bool dequeue(INFORMATION_QUEUE *pQueue, AGREEMENT *pData)
{
	if(pQueue->rear == pQueue->front)
	{
		return false;
	}
	QUEUE_NODE *pTmp = pQueue->front->pNext;
	*pData = pTmp->data;

	pQueue->front->pNext = pTmp->pNext;
	if(NULL == pQueue->front->pNext)
	{
		pQueue->rear = pQueue->front;
	}
	free(pTmp);
	return true;
}

/*功能选择
* 1:单聊        2:群聊       3:查找好友
* 4:添加好友    5:查找群     6:创建群  
* 7:添加群      8:发送文件   0:退出
*/
int choose_function(void)
{
	int num;
	char ch;
	while(1)
	{		
		while(scanf("%d", &num) <= 0)
		{
			printf("输入错误, 请重新输入!\n");
			while((ch = getchar()) != '\n' && ch != EOF);
		}
		if(num > 8 || num < 0)
		{
			printf("抱歉,没有这个选项, 请重新输入!\n");
		}
		else {
			break;
		}
	}
	switch(num)
	{
		case 0:
			printf("感谢使用, 88~~");
			exit(0);
		case 1:
			//single_chat();
			break;
		case 2:
			
			break;
		case 3:
			
			break;
		case 4:
			
			break;
		case 5:
			
			break;
		case 6:
			
			break;
		case 7:
			
			break;
		case 8:
			
			break;
		default:
			printf("抱歉,没有这个选项\n");
			break;
	}
}

int log_in(void)
{
	int num;
	int rt;
	while(1)
	{		
		while(scanf("%d", &num) <= 0)
		{
			printf("输入错误, 请重新输入!\n");
		}
		if(num > 3 || num < 0)
		{
			printf("抱歉,没有这个选项, 请重新输入!\n");
		}
		else {
			break;
		}
	}
	switch(num)
	{
		case 0: 
			printf("感谢使用, 88~~\n");
			return -1;
		case 1:
			//注册账号

			if(-1 == rt)
			{
				return -1;
			}
			break;
		case 2:
			//登陆
			
			if(-1 == rt)
			{
				return -1;
			}
			break;
	}
}
