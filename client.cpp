//#include<stdlib.h>
//#include<stdio.h>
//#include<string.h>
//#include<unistd.h>
//#include<sys/socket.h>
//#include<arpa/inet.h>
//
//#define SERVER_PORT 6666
//#define SERVER_IP "127.0.0.1"
//
//int main(int argc,char **argv) {
//	int sockfd;
//	char* message;
//	struct sockaddr_in servaddr;
//	int n;
//	char buf[64];
//	if (argc!=2)//参数不合法
//	{
//		printf("参数不合法!  ./xxxx.exe  message\n");
//		exit(-1);
//	}
//	
//	message = argv[1];//获取要发送的消息
//	printf("待发送的数据:%s\n",message);
//
//	//创建套接字
//	sockfd = socket(AF_INET,SOCK_STREAM,0);
//	//将server_addr的内存初始化
//	memset(&servaddr,'\0',sizeof(struct sockaddr_in));
//	servaddr.sin_family = AF_INET;
//	
//	inet_pton(AF_INET,SERVER_IP,&servaddr.sin_addr);
//	servaddr.sin_port=htons(SERVER_PORT);
//	//连接服务器
//	connect(sockfd, (struct sockaddr*)&servaddr,sizeof(servaddr));
//	//写数据
//	write(sockfd,message,strlen(message));
//	//读数据
//	n = read(sockfd,buf,sizeof(buf)-1);
//	if (n>0) {
//		buf[n] = '\0';
//		printf("收到的数据:%s\n",buf);
//	}
//	else {
//	
//		perror("error!!!");
//	
//	}
//	printf("finished\n");
//	close(sockfd);
//	return 0;
//}
