#ifndef _CSESSION_H
#define _CSESSION_H


#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>

#include "logging.h"

#define DEFAULT_PORT 5539
#define SESSION_BUFFER_SIZE 1024

typedef struct client_session{
    int sockfd; // 客户端套接字
    pthread_t pt; // 线程
    struct hostent * host; // 远程主机
    LOG * logger; // 客户端日志
    size_t buffer_size;
    pthread_mutex_t send_mutex;
    void (*send)(struct client_session * self,void * buffer); // 发送给远程服务器
    void (*rev_event)(struct client_session * self,void * buffer); // 接收到服务器的回调事件
} CLIENT_SESSION;

void * client_session_listen(void * args){ //监听远程服务器发送的消息
    CLIENT_SESSION * session = (CLIENT_SESSION * )args;
    char buffer[session->buffer_size];
    bzero(&buffer,sizeof(buffer));
    for (;;){
        if (recv(session->sockfd,&buffer,session->buffer_size,0)>0){
            if (strlen(buffer)>0){
                session->logger->info(session->logger,"接收到来自服务端的信息:%s",buffer);
                if (session->rev_event){ // 回调事件
                    session->rev_event(session,buffer);
                }
            }
            bzero(&buffer,sizeof(buffer));
        }
    }
}

void client_session_send(CLIENT_SESSION * self,void * buffer){
    pthread_mutex_lock(&self->send_mutex);
    if (send(self->sockfd,buffer,self->buffer_size,0)==-1){
        self->logger->warning(self->logger,"客户端消息%s发送失败",buffer);
    }
    pthread_mutex_unlock(&self->send_mutex);
}

CLIENT_SESSION * client_session_init(char * hostname,int port){
    CLIENT_SESSION * session = (CLIENT_SESSION *)malloc(sizeof(CLIENT_SESSION)); // 创建会话
    bzero(session,sizeof(CLIENT_SESSION));
    session->logger = logging_create(stdout);
    session->logger->info(session->logger,"初始化客户端");
    pthread_mutex_init(&session->send_mutex,NULL);
    if ((session->host = gethostbyname(hostname))==NULL){
        session->logger->error(session->logger,"客户端主机名错误: %s",hostname);
        exit(1);
    }
    if ((session->sockfd=socket(AF_INET,SOCK_STREAM,0))==-1){
        session->logger->error(session->logger,"套接字创建失败");
        exit(1);
    }
    session->buffer_size = SESSION_BUFFER_SIZE;
    struct sockaddr_in server_addr;
    bzero(&server_addr,sizeof(server_addr));
    server_addr.sin_family  = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr= *((struct in_addr *)session->host->h_addr);
    if (connect(session->sockfd,(struct sockaddr *)(&server_addr),sizeof(struct sockaddr))==-1){
        session->logger->error(session->logger,"连接远程服务器失败");
        exit(1);
    }
    session->send = &client_session_send;
    // 注册监听事件
    pthread_create(&session->pt,NULL,&client_session_listen,session);
    return session;
}


void client_session_release(CLIENT_SESSION * self){
    logging_release(self->logger);
    close(self->sockfd);
    free(self);
}

#endif