#include <stdio.h>
#include <sys/types.h>          
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#incl"Ze <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "config.h"
 
//从命令字符串中解析出参数部分，用于获取操作命令所涉及的文件路径
char * getdir(char *cmd)
{
	char *p;
 
	p = strtok(cmd, " " );
	p = strtok(NULL, " ");
 
	return p;
}
//根据输入的命令字符串判断命令类型，返回相应的整数标识
int get_cmd_type(char *cmd)
{
	if(strstr(cmd,"lcd"))   return LCD;
 
	if(!strcmp("quit",cmd))   return QUIT;
	if(!strcmp("ls",cmd))    return LS;
	if(!strcmp("lls",cmd))   return LLS;
	if(!strcmp("pwd",cmd))   return LS;
 
	if(strstr(cmd,"cd"))    return CD;
	if(strstr(cmd,"get"))   return GET;
	if(strstr(cmd,"put"))   return PUT;
 
	return -1;
}
 
int cmd_handler(struct Msg msg, int fd)
{
	char *dir = NULL;
	char buf[32];
	int ret;
	int filefd;
 
	ret = get_cmd_type(msg.data);// 根据命令类型获取对应的整数标识
 
	switch(R�td
		case LS:
		case CD:
		case PWD:
			msg.type = 0;
			write(fd,&msg,sizeof(msg));// 向服务器发送消息，请求执行列目录、切换目录、显示当前目录等操作
			break;
		case GET:
			msg.type = 2;
			write(fd,&msg,sizeof(msg));//向服务器发送消息，请求获取文件
			break;
		case PUT:
			strcpy(buf,msg.data);
			dir = getdir(buf);// 获取上传文件的路径
 
			if(access(dir,F_OK) == -1){ //判读要上传的文件是否存在
				printf("%s not exsit\n",dir);
			}else{	
				filefd = open(dir,O_RDWR);//打开文件
				read(filefd,msg.secondBuf,sizeof(msg.secondBuf)); // 读取文件内容到消息结构的缓冲区
				close(filefd);//关闭文件
 
				write(fd,&msg,sizeof(msg));// 将消息发送给服务器，执行上传文件操作
			}
			break;
		case LLS:
			system("ls");// 在客户端执行系统命令，显示本地目录内容
			break;
		case LCD:
			dir = getdir(msg.data);// 获取切换目录的路径
			chdir(dir);// 切换本地目录
			break;
		case QUIT:
			strcpy(msg.data,"quit");
			write(fd,&msg,sizeof(msg));// 向服务器发送退出消息
			close(fd);// 关闭连接
			exit(-1);
 
	}
	return ret;
}
//处理从服务器接收到的消息
void handler_server_message(int c_fd, struct Msg msg)
{
	int n_read;
	struct Msg msgget;
	int newfilefd;
 
	n_read = read(c_fd, &msgget, sizeof(msgget));// 从服务器读取消息响应
 
	if(n_read == 0){
		printf("server is out,quit\n");// 如果没有读取到数据，服务器可能已关闭
		exit(-1);
	}
	else if(msgget.type == DOFILE){
		char *p = getdir(msg.data);// 获取要保存文件的路径
		newfilefd = open(p,O_RDWR|O_CREAT,0600);// 创建本地文件	
		write(newfilefd,msgget.data,strlen(msgget.data));// 将服务器传来文件数据写本地文件
		putchar('>');// 输出命令提示符
		fflush(stdout);
	}
	else{
		printf("--------------------------------\n");
		printf("\n%s\n",msgget.data);// 打印服务器返回的数据，可能是目录列表等信息
		printf("--------------------------------\n");
 
		putchar('>');// 输出命令提示符
		fflush(stdout);
	}
 
}
 
int main(int argc, char **argv)
{
	int c_fd;
	struct sockaddr_in c_addr;
 
	struct Msg msg;
 
	memset(&c_addr,0,sizeof(struct sockaddr_in));
 
	if(argc != 3){
		printf("param is not good\n");
		exit(-1);
	}
	//1. socket
	c_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(c_fd == -1){
		perror("socket");
		exit(-1);
	}
 
	c_addr.sin_family = AF_INET;
	c_addr.sin_port = htons(atoi(argv[2]));
	inet_aton(argv[1],&c_addr.sin_addr);
	//2.connect	
	if(connect(c_fd, (struct sockaddr *)&c_addr,sizeof(struct sockaddr)) == -1){
		perror("connect");
		exit(-1);
	}
	printf("connect ...\n");
	int mark = 0;// 标记是否需要输出命令提示符	
	while(1){
		memset(msg.data,0,sizeof(msg.data));// 清除消息缓冲区
		if(mark == 0)  printf(">"); // 输出命令提示符
 
		gets(msg.data);// 获取用户输入的命令
 
		if(strlen(msg.data) == 0){
			if(mark == 1){
				printf(">");
			}
			continue;// 用户未输入命令，继续循环
		}
 
		mark = 1;// 标记已经有输入的命令
 
		int ret = cmd_handler(msg,c_fd);// 处理用户输入的命令
 
		if(ret>IFGO){
			putchar('>');
			fflush(stdout);
			continue; // 如果命令需要进一步处理，继续循环
		}
		if(ret==-1){
			printf("command not \n"); // 无法识别的命令
			printf(">");
			fflush(stdout);
			continue;// 继续循环等待下一个命令
		}
		handler_server_message(c_fd, msg);// 处理服务器响应的消息
	}
	return 0;
}