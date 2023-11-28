#include <stdio.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <string.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <errno.h>
#define SERVER_PORT 80

static int static_debug = 1;//调试使用

//处理http请求
void do_http_request(int  client_sock);
//处理读取一行http的报文
int  get_line(int sock, char *buf, int size);
//http服务器的响应
void do_http_response(int client_socket);
void do_http_response(int client_socket,const char* path);
//发送http的头部
int headers(int client_socket,FILE *resource);
//发送http的内容
void cat(int client_socket, FILE* resource);
//http服务器404响应
void not_found(int client_socket);
//服务器内部错误
void inner_error(int client_socket);
int main(void) {
	//1创建连接的套接字
	int sock;	
	sock = socket(AF_INET, SOCK_STREAM, 0);//IPV4,TCP方式,默认协议

	//2设置标签(地址,端口号)
	struct sockaddr_in server_addr;
	bzero(&server_addr, sizeof(server_addr));			//将server_addr的内容清零

	server_addr.sin_family = AF_INET;					//协议组IPV4
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);	//监听本地所有ip
	server_addr.sin_port = htons(SERVER_PORT);			//设置端口号

	//3绑定标签
	bind(sock,(struct sockaddr*)&server_addr,sizeof(server_addr));

	//4设置监听并指定监听最大数量
	listen(sock,128);

	//5等待连接
	printf("等待客户端连接请求\n");
	int done = 1;
	while (done) {
		//5-1有连接则重新分配套接字接受连接
		struct sockaddr_in clinet;
		int  client_sock, len;			//客户端的 套接字
		char client_ip[64];				//客户端的 ip
		char buf[256];					//客户端的 发送的内容
		
		socklen_t client_addr_len;		//套接字地址结构长度
		client_addr_len = sizeof(clinet);

		//5-2接受分配的套接字
		client_sock = accept(sock,(struct sockaddr*)&clinet,&client_addr_len);

		//5-3查看客户端IP和端口号
		inet_ntop(AF_INET, &clinet.sin_addr.s_addr, client_ip, sizeof(client_ip));
		printf("客户端IP:%s	,端口号:%d\n", client_ip, ntohs(clinet.sin_port));


		do_http_request(client_sock);
		close(client_sock);
	
	}
	close(sock);
	return 0;
}

//处理http请求,
void do_http_request(int  client_sock) {
	//读取客户端的http请求
	//1读取请求行
	int len = 0;
	char buf[256];				//请求行+请求头部
	char method[32];			//获取方法GET 或 POST
	char url[256];				//url资源内容
	char path[256];				//资源的路径
	struct stat st;				//请求文件的状态
	len = get_line(client_sock, buf, sizeof(buf));//读取一行
	if ( len>0 )//读到了请求行
	{
		int i = 0, j = 0;			//j控制buf  i控制method或url
		while (!isspace(buf[j]) && i<sizeof(method)-1)
		{
			method[i] = buf[j];
			i++;
			j++;
		
		}
		method[i] = '\0';
		if (static_debug) printf("请求的方法:%s\n", method);

		if (strncmp(method, "GET", strlen(method))==0) {//处理get请求 不区分大小写

			if (static_debug) printf("GET方法的请求\t");

			while (isspace(buf[j++]));
			i = 0;

			while (!isspace(buf[j]) && i < sizeof(url) - 1)
			{
				url[i] = buf[j];
				i++;
				j++;
			}
			url[i] = '\0';
			if (static_debug) printf("url内容:%s\n\r", url);

			//读取请求头部
			do {
				len = get_line(client_sock, buf, sizeof(buf));//读取一行
				if (static_debug) printf("请求头:%s\n", buf);
			} while (len > 0);

			//定位本地的html文件
			char* pos = strchr(url,'?');
			if (pos)
			{
				*pos = '\0';
				if (static_debug) printf("处理后的url:%s\n",url);
			}
			sprintf(path,"./html_docs/%s",url);
			if (static_debug) printf("请求的资源路径:%s\n",path);

			//执行http的响应
			//判断客户端要的文件存不存在
			if (stat(path, &st) == -1) {//文件不存在
				//404 找不到资源
				fprintf(stderr,"stat %s failed,reason:%s\n",path,strerror(errno));
				not_found(client_sock);
			}
			else//文件存在
			{
				if (S_ISDIR(st.st_mode)) {
					strcat(path,"/1.html");
				
				}
				//响应客户端请求的文件
				do_http_response(client_sock,path);
			}
			
		}
		else//非GET请求
		{
			fprintf(stderr,"警告:收到%s请求!\n",method);
			do
			{
				len = get_line(client_sock, buf, sizeof(buf));
				if (static_debug) printf("请求头:%s\n", buf);
			} while (len>0);


			//501响应.....
			
		}
		
	}
	else//请求出错
	{
		//fprintf(stderr, "警告:收到%s请求!\n", method);
	}
	
}

//读取客户端发送的数(返回值  :-1读取出错,  0读到空行,  大于0成功读取一行)
int get_line(int sock, char* buf, int size) {
	int count = 0;		//已经读取的字符个数
	char ch = '\0';		//读取的字符
	int len = 0;

	while ((count<size-1) && ch!='\n' )
	{
		len = read(sock,&ch,1);
		if (len == 1) {//读取一个字符成功
			if (ch == '\r') {			//读取一个回车符,则忽略
				continue;
			}
			else if(ch == '\n')			//读取一个换行符,则替换为'\0'
			{
				break;
			}
			buf[count++] = ch;
		}
		else if (len==-1)//读取一个字符失败
		{
			perror("read failed");		//在标准错误输出上生成一条消息，描述调用系统或库函数时遇到的最后一个错误。
			count = -1;					//出错啦
			break;
		}
		else			//客户端自己关闭连接
		{
			fprintf(stderr,"客户端关闭连接啦!\n");
			count = -1;					//出错啦
			break;
		}
	}
	if (count>=0) 
	{
		buf[count] = '\0';
	}
	return count;						//成功:返回读取的字节个数;	失败:返回-1;
}


void do_http_response(int client_socket) {
	const char* main_header = "HTTP/1.0 200 OK\r\nSrever: JXF Server\r\nContent-Type:text/html\r\nConnection: Close\r\n";
	const char* welcome_content = "\
			<html lang=\"zh-CN\">\n\
				<head>\n\
					<meta content=\"text/html; charset=utf-8\" http-equiv=\"Content-Type\">\n\
					<title>This is a test</title>\n\
				</head>\n\
				<body>\n\
					<div align=center height=\"500px\">\n\
						<br /><br /><br />\n\
						<h2>请问,你的支付宝账号和密码是多少?</h2><br /><br />\n\
						<form action=\"commit\" method=\"post\">\n\
								账号: <input type=\"text\" name=\"name\" />\n\
								<br />\n\
								密码:  <input type=\"password\" name=\"age\" />\n\
								<br /><br /><br />\n\
								<input type=\"submit\" value=\"提交\"/>\n\
								<input type=\"reset\"  value=\"重置\"/>\n\
						</form>\n\
					</div>\n\
				</body>\n\
			</html>\n\
			";
	//1发送main_header
	int len = write(client_socket, main_header,strlen(main_header));
	if (static_debug) fprintf(stdout,"...do_http_response...\n");
	if (static_debug) fprintf(stdout, "write[%d]:%s",len, main_header);

	//2生成Content-Length
	char send_buf[64];
	int wc_len = strlen(welcome_content);
	len = snprintf(send_buf,64,"Content-Length: %d\r\n\r\n", wc_len);
	len = write(client_socket, send_buf, len);
	if (static_debug) fprintf(stdout, "write[%d]:%s", len, send_buf);

	//3发送主体的html内容welcome_content
	len = write(client_socket, welcome_content, wc_len);
	if (static_debug) fprintf(stdout, "write[%d]:%s", len, welcome_content);

}

void do_http_response(int client_socket, const char* path) {
	int res = 0;
	FILE* resource = NULL;
	resource = fopen(path, "r");//只读方式打开文件
	if (resource == NULL) {
		not_found(client_socket);//打开失败则响应未找到
		return;
	}

	//1发送http头部
	res = headers(client_socket, resource);
	//2发送http内容
	if (!res){
		cat(client_socket, resource);
	}
	fclose(resource);
}


void not_found(int client_socket) {
	const char* main_404 ="HTTP/1.0 404 NOT FOUND\r\nContent-Type:text/html\r\n\r\n\
	<html lang=\"zh-CN\">\r\n\
		<head>\r\n\
			<meta content=\"text/html; charset=utf-8\" http-equiv=\"Content-Type\">\r\n\
			<title>NOT FOUND</title>\r\n\
		</head>\r\n\
		<body>\r\n\
				<p>抱歉,此文件不存在!\r\n\
				<p>Sorry, this file does not exist!\r\n\
		</body>\r\n\
	</html>\r\n\
	";
	int len = write(client_socket, main_404, strlen(main_404));
	if (static_debug) printf("len:%d\n", len);
	if (static_debug) fprintf(stdout, main_404);
	if (len <= 0) {
		fprintf(stderr,"send reply failed, reason :%s\n",strerror(errno));
	}
}

void inner_error(int client_socket) {
	const char* main_500 = "HTTP/1.0 500 Internal Server Error\r\n\
	Content-Type:text/html\r\n\
	\r\n\
	<html lang=\"zh-CN\">\r\n\
		<head>\r\n\
			<title>Method Not Implemented</title>\r\n\
		</head>\r\n\
		<body>\r\n\
				<p>服务器内部出错!\r\n\
		</body>\r\n\
	</html>\r\n\
	";
	int len = write(client_socket, main_500, strlen(main_500));
	if (len <= 0) {
		fprintf(stderr, "send reply failed, reason :%s\n", strerror(errno));
	}
}


//发送http的头部(成功返回0 失败返回-1)
int headers(int client_socket, FILE* resource) {
	char buf[1024] = { 0 };
	char tmp[64];
	struct stat st;
	int fileid=0;

	strcpy(buf, "HTTP/1.0 200 OK\r\n");
	strcat(buf, "Srever: JXF Server\r\n");
	strcat(buf, "Content-Type:text/html\r\n");
	strcat(buf, "Connection: Close\r\n");
	
	//获取文件大小
	fileid = fileno(resource);		//从文件指针提取文件id
	if (fstat(fileid, &st) == -1) {	//返回有关文件的信息
		//返回失败	
		fprintf(stderr,"提取文件出错!,原因:%s\n",strerror(errno));
		inner_error(client_socket);	//500响应-内部错误
		return -1;
	}

	//返回成功
	snprintf(tmp,64,"Content-Length: %d\r\n\r\n", st.st_size);
	strcat(buf, tmp);
	if (static_debug) fprintf(stdout, "header:%s\n", buf);

	if (send(client_socket, buf, strlen(buf), 0)<0) {
		fprintf(stderr, "服务器内部出错!,原因:%s\n", strerror(errno));
		return -1;
	}
	return 0;
}


//发送http的内容
void cat(int client_socket, FILE* resource) {
	char buf[1024];
	int len = 0;

	fgets(buf,sizeof(buf), resource);
	while (!feof(resource)){
		len = write(client_socket,buf,strlen(buf));
		if (len<0) {
			fprintf(stderr, "发送http body错误,原因:%s\n", strerror(errno));
		}

		if (static_debug) fprintf(stdout, "body:%s\n", buf);
		fgets(buf, sizeof(buf), resource);
	}
}