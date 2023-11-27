//#include <stdio.h>
//#include <unistd.h>
//#include <sys/types.h> 
//#include <sys/socket.h>
//#include <string.h>
//#include <ctype.h>
//#include <arpa/inet.h>
//
//#define SERVER_PORT 80
//
//int main(void) {
//	//1创建连接的套接字
//	int sock;
//	sock = socket(AF_INET, SOCK_STREAM, 0);//IPV4,TCP方式,默认协议
//
//	//2设置标签(地址,端口号)
//	struct sockaddr_in server_addr;
//	bzero(&server_addr, sizeof(server_addr));			//将server_addr的内容清零
//
//	server_addr.sin_family = AF_INET;					//协议组IPV4
//	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);	//监听本地所有ip
//	server_addr.sin_port = htons(SERVER_PORT);			//设置端口号
//
//	//3绑定标签
//	bind(sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
//
//	//4设置监听并指定监听最大数量
//	listen(sock, 128);
//
//	//5等待连接
//	printf("等待客户端连接请求\n");
//	int done = 1;
//	while (done) {
//		//5-1有连接则重新分配套接字接受连接
//		struct sockaddr_in clinet;
//		int  client_sock, len;			//客户端的 套接字
//		char client_ip[64];				//客户端的 ip
//		char buf[256];					//客户端的 发送的内容
//
//		socklen_t client_addr_len;		//套接字地址结构长度
//		client_addr_len = sizeof(clinet);
//
//		//5-2接受分配的套接字
//		client_sock = accept(sock, (struct sockaddr*)&clinet, &client_addr_len);
//
//		//5-3查看客户端IP和端口号
//		inet_ntop(AF_INET, &clinet.sin_addr.s_addr, client_ip, sizeof(client_ip));
//		printf("客户端IP:%s	,端口号:%d\n", client_ip, ntohs(clinet.sin_port));
//
//		//5-4客户端发来的数据
//		len = read(client_sock, buf, sizeof(buf) - 1);
//		buf[len] = '\0';
//		printf("收到客户端发来的长度为%d数据%s\n", len, buf);
//
//		//5-4服务端给客户写数据
//		len = write(client_sock, buf, len);
//		printf("已完成对客户端的写操作\n");
//		close(client_sock);
//
//	}
// 	close(sock);
//	return 0;
//}