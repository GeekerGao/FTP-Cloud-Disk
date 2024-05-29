#include <stdio.h>
#include <sys/types.h>          
#include <sys/socket.h>
//#include <linux/in.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include <sys/stat.h>
#include <fcntl.h>
 
// 根据命令字符串获取命令类型的整数值
int get_cmd_type(char *cmd) 
{
	if(!strcmp("ls",cmd))         return LS;
	if(!strcmp("quit",cmd))       return QUIT;
	if(!strcmp("pwd",cmd))        return PWD;
	if(strstr(cmd,"cd")!=NULL)    return CD;
	if(strstr(cmd,"get")!=NULL)   return GET;
	if(strstr(cmd,"put")!=NULL)   return PUT;
 
	return 100;
}
// 获取目标目录路径 
char *getDesDir(char *cmsg)
{
	char *p;
	p = strtok(cmsg," ");
	p = strtok(NULL," ");
	return p;
}
// 处理客户端消息
void msg_handler(struct Msg msg, int fd)
{
	char dataBuf[1024]={0};// 用于存储文件内容的缓冲区
	char *file = NULL;// 用于存储要操作的文件名
	int fdfile;
	
	printf("cmd:%s\n",msg.data);// 打印接收到的命令
	int ret = get_cmd_type(msg.data);// 获取命令类型的整数值
	
	switch(ret){
		case LS:
		case PWD:
			msg.type = 0;// 设置消息类型，用于不同的响应
			FILE *r = popen(msg.data,"r");// 执行 shell 命令并创建子进程
			fread(msg.data,sizeof(msg.data),1,r);// 从子进程读取数据到缓冲区
			write(fd, &msg,sizeof(msg)); // 发送数据给客户端
			break;
		case CD:
			msg.type = 1;
			char *dir = getDesDir(msg.data);// 获取要切换的目标目录
			printf("dir:%s\n",dir);
			chdir(dir);// 切换当前工作目录
			break;
		case GET:
			file = getDesDir(msg.data);// 获取要下载的文件名
			
			if(access(file,F_OK) == -1){ //判断文件是否存在
				strcpy(msg.data,"No This File!");
				write(fd,&msg,sizeof(msg));// 发送消息给客户端
			}else{
				msg.type = DOFILE;// 设置消息类型为发送文件内容
				
				fdfile = open(file,O_RDWR);// 打开文件以供读取
				read(fdfile,dataBuf,sizeof(dataBuf));// 读取文件内容到缓冲区
				close(fdfile);//关闭文件
 
				strcpy(msg.data,dataBuf);// 将文件内容复制到消息数据中
				write(fd,&msg,sizeof(msg));// 向客户端发送包含文件内容的消息
			}
			break;
		case PUT:
			fdfile = open(getDesDir(msg.data),O_RDWR|O_CREAT,0666);//打开/创建文件
			write(fdfile, msg.secondBuf, strlen(msg.secondBuf));// 将客户端发送的文件数据写入文件
			close(fdfile);//关闭文件
			break;
		case QUIT:
			printf("client quit!\n");// 打印客户端退出信息
			exit(-1);// 退出程序
	}
}
 
 
int main(int argc, char **argv)
{
	int s_fd;
	int c_fd;
	int n_read;
	char readBuf[128];
 
	struct sockaddr_in s_addr;
	struct sockaddr_in c_addr;
	struct Msg msg;
 
	if(argc != 3){
		printf("param is not good\n");
		exit(-1);
	}
 
	memset(&s_addr,0,sizeof(struct sockaddr_in));
	memset(&c_addr,0,sizeof(struct sockaddr_in));//数据清空
 
	//1. socket
	s_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(s_fd == -1){
		perror("socket");
		exit(-1);
	}
 
	s_addr.sin_family = AF_INET;
	s_addr.sin_port = htons(atoi(argv[2]));
	inet_aton(argv[1],&s_addr.sin_addr);
 
 
	//2. bind
	bind(s_fd,(struct sockaddr *)&s_addr,sizeof(struct sockaddr_in));
 
	//3. listen
	listen(s_fd,10);
	//4. accept
	int clen = sizeof(struct sockaddr_in);
	while(1){
		c_fd = accept(s_fd,(struct sockaddr *)&c_addr,&clen);
		if(c_fd == -1){
			perror("accept");
		}
 
		printf("get connect: %s\n",inet_ntoa(c_addr.sin_addr));
 
		if(fork() == 0){ // 创建子进程处理客户端连接
			while(1){
				memset(msg.data,0,sizeof(msg.data));//清除数据
				n_read = read(c_fd, &msg, sizeof(msg));	// 从客户端读取消息
				if(n_read == 0){ //判断读取的字节数
					printf("client out\n");// 打印客户端断开连接的信息
					break;
				}else if(n_read > 0){
					msg_handler(msg,c_fd);// 处理客户端消息
				}
			}
		}
 
	}
	close(c_fd);//关闭客户端socket
	close(s_fd);//关闭服务端socket
	return 0;
}