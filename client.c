#include "csession.h"

int main(){
    CLIENT_SESSION * session = client_session_init("localhost",5538);
    char buffer[1024];
    for (;;){
        scanf("%s",buffer);
        session->send(session,buffer);
    }
    return 0;
}