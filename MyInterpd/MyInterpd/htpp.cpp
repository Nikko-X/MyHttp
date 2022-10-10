#include <stdio.h>
// 网络通信需要包含的头文件、以及加载的库文件
#include <WinSock2.h>
#pragma comment(lib, "WS2_32.lib")

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
//处理用户请求的线程函数
DWORD WINAPI accept_request(LPVOID arg) {

	return 0;
}

int main(void) {
	unsigned short port = 0;
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
