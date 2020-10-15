#ifndef _SESSION_H
#define _SESSION_H

#define DEFAULT_PORT 5538
#define MAX_CONNECTIONS 2
#include <stdlib.h>
#include "logging.h"
#include <sys/socket.h> 
#include <netinet/in.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>

typedef struct server_session
{
    int sockfd; // socket 标识
    int port;
    LOG * logger;
} SESSION;

// 初始化session带端口
SESSION * session_init_with_port(int port){
    SESSION * session = (SESSION *)malloc(sizeof(SESSION));
    session->logger = logging_create(stdout);
    session->port = port;
    if ((session->sockfd = socket(AF_INET,SOCK_STREAM,0))==-1){
        session->logger->error(session->logger,"创建socket失败");
        exit(1);
    }
    struct sockaddr_in server_addr;
    bzero(&server_addr,sizeof(struct sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(session->port);   
    if (bind(session->sockfd,(struct sockaddr *)(&server_addr),sizeof(struct sockaddr))==-1){
        session->logger->error(session->logger,"无法绑定指定端口 %d",port);
        exit(1);
    }
    if (listen(session->sockfd,MAX_CONNECTIONS)==-1){
        session->logger->error(session->logger,"监听连接错误");
        exit(1);
    }
    session->logger->info(session->logger,"建立连接，端口%d",session->port);
    return session;
}

// 初始化socket，并返回socket
SESSION * session_init(){
    return session_init_with_port(DEFAULT_PORT);
}

// 更换logger
void session_set_logger(SESSION * self,LOG * logger){
    self->logger = logger;
}

// 更换默认logger
void session_set_logger_with_release(SESSION * self,LOG * logger){
    logging_release(self->logger);
    session_set_logger(self,logger);
}

void session_release(SESSION * self){
    self->logger->info(self->logger,"释放连接，端口%d",self->port);
    logging_release(self->logger);
    close(self->sockfd);
    free(self);
}

#endif