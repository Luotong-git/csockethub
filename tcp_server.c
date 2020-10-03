/*使用面向连接的套接口实现通信*/
/*套接口方式不仅可以实现单机内进程间通信，还可以实现不同计算机进程之间通信*/
/*先运行tcp-server程序，端口号作为参数，实现server程序的监听。*/
/*再运行tcp-client程序，以server所在的“ip地址127.0.0.1”或“主机名（localhost）”为第一参数，相同的端口号作为第二参数连接server*/

/*tcp-server.c */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#define MAXSIZE 1024     /*定义数据缓冲区大小*/

int main(int argc, char *argv[])
{
	int sockfd,new_fd;   /*定义存放套接口描述符的变量 */
	struct sockaddr_in server_addr;   /*定义服务器端套接口数据结构server_addr */
	struct sockaddr_in client_addr;   /*定义客户端套接口数据结构client_addr */
	int sin_size,portnumber;
	char buf[MAXSIZE];     /*定义发送数据缓冲区*/
	if(argc!=2)
	{
		fprintf(stderr,"Usage:%s portnumber\a\n",argv[0]);
		exit(1);
	}
	if((portnumber=atoi(argv[1]))<0)
	{  /*获得命令行的第二个参数--端口号，atoi()把字符串转换成整型数*/
		fprintf(stderr,"Usage:%s portnumber\a\n",argv[0]);
		exit(1);
	}
	if((sockfd=socket(AF_INET,SOCK_STREAM,0))==-1)/*服务器端开始建立socket描述符*/
	{
		fprintf(stderr,"Socket error:%s\n\a",strerror(errno));
		exit(1);
	}
	/*服务器端填充 sockaddr结构*/
	bzero(&server_addr,sizeof(struct sockaddr_in)); /*先将套接口地址数据结构清零*/
	server_addr.sin_family=AF_INET;/*设为TCP/IP地址族*/
	server_addr.sin_addr.s_addr=htonl(INADDR_ANY);/*设置本机地址并从主机字节序转换为网络字节序*/
	server_addr.sin_port=htons(portnumber);/*设置端口号并从主机字节序转换为网络字节序*/
	if(bind(sockfd,(struct sockaddr *)(&server_addr),sizeof(struct sockaddr))==-1)/*调用bind函数绑定指定的端口号和ip地址到服务器创建的套接口*/
	{
		fprintf(stderr,"Bind error:%s\n\a",strerror(errno));
		exit(1);
	}
	if(listen(sockfd,5)==-1)  /*端口绑定成功，监听sockfd描述符，设置同时处理的最大连接请求数为5 */
	{
		fprintf(stderr,"Listen error:%s\n\a",strerror(errno));
		exit(1);
	}
	while(1)      /*服务器阻塞，等待接收连接请求，直到客户程序发送连接请求*/
	{
		sin_size=sizeof(struct sockaddr_in);
		if((new_fd=accept(sockfd,(struct sockaddr *)(&client_addr),&sin_size))==-1)	  /*调用accept接受一个连接请求并返回一个新的套接口描述符*/			
		{
			fprintf(stderr,"Accept error:%s\n\a",strerror(errno));
			exit(1);
		}
		fprintf(stderr,"Server get connection from %s\n",(inet_ntoa(client_addr.sin_addr)));   /*TCP连接已建立，打印申请连接的客户机的IP地址,IP地址从网络字节序转换为十进制数*/	
		printf("Connected successful, please input the masage[<1024 bytes]:\n");    /*提示用户输入将要发送的数据，长度小于缓冲区的长度，即1024字节*/
       		
		if(fgets(buf, sizeof(buf), stdin) != buf) /*从标准输入即键盘输入的数据存放在buf缓冲区*/
		{  
         		printf("fgets error!\n");
         		exit(1);
		}
		if(write(new_fd,buf,strlen(buf))==-1)   /*调用write发送数据*/
		{
			fprintf(stderr,"Write Error:%s\n",strerror(errno));
			exit(1);
		}
		close(new_fd);  /*本次通信已结束，关闭客户端的套接口，并循环下一次等待*/
	}
	close(sockfd);  /*服务器进程结束，关闭服务器端套接口*/
	exit(0);
}
