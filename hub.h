#ifndef _HUB_H
#define _HUB_B

#include "session.h"
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <arpa/inet.h>

#define MAX_CLIENTS 25

typedef struct client
{
    char *identity; // 客户端标识
    int sockfd;     // 客户端套接字
    bool closed;    // 客户端关闭
    pthread_t pt;   // 客户端监听线程
} CLIENT;

typedef struct client_group
{
    char *name;                  // 分组标识名
    CLIENT clients[MAX_CLIENTS]; // 客户端数组
    pthread_mutex_t clients_lock[MAX_CLIENTS]; // 客户端锁
    int count;                   // 客户端数量
    int lastcount;               // 历史客户端连接数量
    pthread_mutex_t lock;
} GROUP;

typedef struct session_hub
{
    GROUP group;
    SESSION *session;
    size_t buffer_size;
    void (*send)(struct session_hub *hub, GROUP *group, const void *buffer); // 向分组发送消息
    void (*listen)(struct session_hub *hub);                                 // 监听客户端，并将客户端加入分组
    void (*rev_event)(struct session_hub *hub, const void *buffer);          // 服务器收到客户端发送的信息后的回调函数
} HUB;

void session_hub_send(HUB *hub, GROUP *group, const void *buffer);
void session_hub_listen(HUB *hub);

HUB *session_hub_init(char *name, size_t size)
{
    HUB *hub = (HUB *)malloc(sizeof(HUB));
    hub->session = session_init();
    hub->group.name = name;
    hub->group.count = 0;
    hub->group.lastcount++;
    for (int index = 0; index < MAX_CLIENTS; ++index)
    {
        hub->group.clients[index].closed = true;
    }
    if (pthread_mutex_init(&(hub->group.lock), NULL) != 0)
    {
        hub->session->logger->error(hub->session->logger, "客户组互斥锁创建失败");
        exit(1);
    }
    for (int i=0;i<MAX_CLIENTS;++i){
        if (pthread_mutex_init(&(hub->group.clients_lock[i]),NULL)!=0){
            hub->session->logger->error(hub->session->logger,"客户端锁创建失败");
            exit(1);
        }
    }
    hub->buffer_size = size;
    hub->send = &session_hub_send;
    hub->listen = &session_hub_listen;
    return hub;
}

void session_hub_send(HUB *hub, GROUP *group, const void *buffer)
{
    int alive = 0; // 存活的客户端
    for (int index = 0; index < MAX_CLIENTS; ++index)
    {
        CLIENT *client = &(group->clients[index]);
        if (alive > group->count)
            break;
        if (!client->closed)
        {
            ++alive;
            pthread_mutex_lock(&group->clients_lock[index]);
            if (send(client->sockfd, buffer, hub->buffer_size, 0) == -1)
            {
                LOG *logger = hub->session->logger;
                logger->warning(logger, "发送给客户端 %s 消息失败，连接关闭", client->identity);
                close(client->sockfd);
                // 分组临界资源上锁
                if (pthread_mutex_lock(&(group->lock)) != 0)
                {
                    hub->session->logger->warning(hub->session->logger, "分组%s未能上锁", group->name);
                    continue;
                };
                group->count--;
                client->closed = true;
                pthread_mutex_unlock(&(group->lock));
            }
            pthread_mutex_unlock(&group->clients_lock[index]);
        }
    }
}

typedef struct hub_with_client
{
    HUB *hub;
    CLIENT *client;
} HUB_CLIENT;

// 监听客户端（死循环）
void *session_hub_rev(void *args)
{
    struct hub_with_client *hubc = (struct hub_with_client *)(args);
    HUB *hub = hubc->hub;
    CLIENT *client = hubc->client;
    // free(args); // 释放指针变量
    hub->session->logger->info(hub->session->logger, "服务端开始接收%s客户端的消息", client->identity);
    char buffer[hub->buffer_size];
    bzero(buffer, sizeof(buffer));
    for (;;)
    {
        if (recv(client->sockfd, &buffer, sizeof(buffer), 0) != -1)
        {
            if (strlen(buffer) > 0)
            {
                hub->session->logger->info(hub->session->logger, "接收到%s发送的信息:%s", client->identity, buffer);
                if (hub->rev_event)
                {
                    hub->rev_event(hub, buffer);
                };
                bzero(buffer,sizeof(buffer));
            }
        }
    }
}

// 仓库监听（死循环）
void session_hub_listen(HUB *hub)
{
    hub->session->logger->info(hub->session->logger, "服务器开始监听");
    for (;;)
    {
        if (hub->group.count < MAX_CLIENTS)
        {
            struct sockaddr_in client_addr;
            size_t c_len = sizeof(client_addr);
            int c_fd = accept(hub->session->sockfd, (struct sockaddr *)&client_addr, (socklen_t * __restrict) & c_len);
            // 分组临界资源上锁
            if (pthread_mutex_lock(&(hub->group.lock)) != 0)
            {
                hub->session->logger->warning(hub->session->logger, "分组%s未能上锁", hub->group.name);
                continue;
            };
            // 加入分组
            CLIENT *client;
            for (int index = 0; index < MAX_CLIENTS; ++index)
            {
                if (hub->group.clients[index].closed)
                {
                    client = &(hub->group.clients[index]);
                    client->closed = false;
                    client->identity = inet_ntoa(client_addr.sin_addr);
                    client->sockfd = c_fd;
                    hub->group.count++;
                    hub->group.lastcount++;
                    break;
                }
            }
            pthread_mutex_unlock(&(hub->group.lock));
            // 创建监听线程
            if (client)
            {
                struct hub_with_client *hubc = (struct hub_with_client *)malloc(sizeof(struct hub_with_client));
                hubc->client = client;
                hubc->hub = hub;
                pthread_create(&client->pt, NULL, &session_hub_rev, hubc);
            }
        }
        sleep(2);
    }
}

void session_hub_release(HUB *hub)
{
    free(hub);
}

#endif