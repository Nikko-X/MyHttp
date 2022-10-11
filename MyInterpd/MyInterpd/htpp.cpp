#include <stdio.h>
#include <string.h>
// 网络通信需要包含的头文件、以及加载的库文件
#include <WinSock2.h>
#pragma comment(lib, "WS2_32.lib")

// 定义一个宏(相比与定义函数，宏节省开销)
// 可打印出当前函数，所在行，以及需打印的字符
#define PRINT(str)	printf("[%s - %d]"#str"=%s\n", __func__, __LINE__, str);

void error_die(const char* str) {
	perror(str);
	exit(1);
}

// 实现网络的初始化
// 返回值：套接字(服务器端的套接字)
// 端口
// 参数：port 表示端口
//		如果*port的值是0，那么就自动分配一个可用端口
//		用指针是为了向tinyhttp 致敬
int startup(unsigned short* port) {
	// 1、网络通信的初始化
	WSADATA data;
	int ret= WSAStartup(
		MAKEWORD(1, 1),	//1.1版本的协议
		&data);
	if (ret) {		// 返回值不是0，说明初始化错误
		return -1;
	}

	// 2、创建套接字
	int server_socket= socket(PF_INET,		// 套接字类型(网络套接字)
		SOCK_STREAM,	// 数据流
		IPPROTO_TCP);
	if (server_socket == -1) {
		// 打印错误提示，结束程序
		error_die("套接字");
	}

	// 3、设置端口可复用
	int opt = 1;
	ret= setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, 
		(const char*)&opt, sizeof(opt));
	if (ret == -1) {
		error_die("setsockopt");
	}
	// 配置服务器端的网络地址
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family= PF_INET;
	server_addr.sin_port= htons( * port);
	server_addr.sin_addr.s_addr= htonl(INADDR_ANY);	// 任何人都可以访问
	

	// 4、绑定套接字
	if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
		error_die("bind");
	}
	// 动态分配端口
	int name_len = sizeof(server_addr);
	if (*port == 0) {
		if (getsockname(server_socket,
			(struct sockaddr*)&server_addr, &name_len) < 0) {
			error_die("getsockname");
		}

		*port = server_addr.sin_port;
	}
	// 5、 创建监听队列
	if (listen(server_socket, 5) < 0) {
		error_die("listen");
	}

	return server_socket;

}

// 从指定的客户端套接字，读取一行数据，保存到buff中
// 返回实际读取的字节数
int get_line(int sock, char* buff, int size) {
	int i = 0;
	char c = 0; // '\0'

	while (i< size-1 && c != '\n') {
		int n = recv(sock, &c, 1, 0);
		if (n > 0) {
			if (c == '\r') {		// 一行结尾可能是/r /n
				n = recv(sock, &c, 1, MSG_PEEK);		// 查看下一个字符但不取出
				if (n > 0 && c == '\n') {
					recv(sock, &c, 1, 0);	// 正式读取
				}
				else {
					c = '\n';
				}
			}
			buff[i++] = c;
		}
		else {
			c = '/n';	// 读取失败,相当于break
		}
	}
	buff[i] = 0;
	return i;
}

void unimplement(int client) {
	// 向指定套接字，发送一个错误提示，显示不支持该功能

}

//处理用户请求的线程函数
DWORD WINAPI accept_request(LPVOID arg) {
	char buff[1024];
	int client = (SOCKET)arg;
	// 读取一行数据
	int numchars =  get_line(client, buff, sizeof(buff));
	PRINT(buff);
	char method[255];
	int j = 0, i= 0;
	while (!isspace(buff[j]) && i < sizeof(method) - 1) {
		method[i++] = buff[j++];
	}
	method[i] = 0;	// '\0'
	PRINT(method);

	//检查请求的方法，本服务器是否支持
	if (stricmp(method, "GET") && strcmp(method, "POST")) {
		// 向浏览器返回一个错误提示
		unimplement(client);
		return 0;
	}
	
	// 解析资源文件路径

	char url[255]; // 存放请求的资源的完整路径
	i = 0;
	// 跳过空格
	while (isspace(buff[j]) && j < sizeof(buff)) j++;

	while (!isspace(buff[j]) && i < sizeof(url) - 1 && j< sizeof(buff)) {
		url[i++] = buff[j++];
	}
	url[i] = 0;
	PRINT(url);

	// www.xh.com
	//127.0.0.1 这些是根目录
	// url	/
	// htdocs/

	return 0;
}

int main(void) {
	unsigned short port = 80;
	int server_sock= startup(&port);
	printf("httpd服务已经启动，正在监听 %d 端口", port);

	//system("pause");

	struct sockaddr_in client_addr;
	int client_addr_len = sizeof(client_addr);

	while (1){
		// 阻塞式等待用户通过浏览器访问网站
		int client_sock = accept(server_sock,
			(struct sockaddr*)&client_addr,
			& client_addr_len);
		if (client_sock == -1) {
			error_die("accept");
		}

		// 创建一个新线程
		DWORD threadID = 0;
		CreateThread(0, 0,
			accept_request, (void*)client_sock, 
			0, &threadID);

		// 使用client_sock 对用户进行访问

	}

	closesocket(server_sock);
	return 0;
}
