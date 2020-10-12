#include "hub.h"

void rev_event(struct session_hub * hub, const void *buffer){
    hub->send(hub,&hub->group,buffer);
}

int main(){
   HUB * hub = session_hub_init("聊天室",65536);
   hub->rev_event = &rev_event;
   hub->listen(hub);
   return 0;
}