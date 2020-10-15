#include "hub.h"
#include "message.h"

// 服务端接收事件
void rev_event(int client_fd,struct session_hub * hub, const void *buffer){  
    MessageType type = {0};
    memcpy(&type,buffer,sizeof(type));
    UserModel user;
    bzero(user,sizeof(user));
    char message_buffer[1024];
    switch (type.type)
    {
    case 0:
        /* code */
        break;
    case 1:
        // 五子棋操作，广播到双方的客户端上
        hub->send(hub,0,&hub->group,buffer);
        break;
    case 2:
        // 聊天室信息
        hub->send(hub,client_fd,&hub->group,buffer);
        break;
    case 3:
        // 登录信息
        memcpy(&user,buffer+sizeof(type),sizeof(user)); // 复制到user对象中
        hub->session->logger->info(hub->session->logger,"用户：%s 登录",user.Name);
        bzero(&message_buffer,sizeof(message_buffer));
        sprintf(&message_buffer,"用户%s加入了游戏",message_buffer);
        hub->send(hub,client_fd,&hub->group,&message_buffer);
        break;
    default:
        break;
    }
}

int main(){
   HUB * hub = session_hub_init("聊天室",65536);
   hub->rev_event = &rev_event;
   hub->listen(hub);
   return 0;
}