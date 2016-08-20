/*************************************************************************
	> File Name: socketpair.c
	> Author: ma6174
	> Mail: ma6174@163.com 
	> Created Time: 2016年08月06日 星期六 10时36分37秒
 ************************************************************************/

#include<stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>


int main()
{
	int fd[2];
	int ret = socketpair(PF_LOCAL,SOCK_STREAM,0,fd);
	if(ret < 0)
	{
		perror("socketpair");
		return 1;
	}
	pid_t id = fork();
	while(1)
	{
	if(id < 0)
	{
		perror("fork");
		return 2;
	}
	else if(id == 0)
	{
		//child
		close(fd[0]);
		char * msg = "i am child";
		char buf[1024];
		write(fd[1],msg,strlen(msg));
		ssize_t _s = read(fd[1],buf,sizeof(buf));
		if(_s < 0)
		{
			perror("read");
			return 2;
		}
		else if(_s > 0)
		{
			buf[_s] = '\0';
		}
		sleep(1);
		printf("child recv #:%s\n",buf);
	}
	else
	{
		//father
		close(fd[1]);
		char * msg = "i am father";
		char buf[1024];
		ssize_t _s = read(fd[0],buf,sizeof(buf));
		if(_s < 0)
		{
			perror("read");
			return 3;
		}
		else if(_s > 0)
		{
			buf[_s] = '\0';
		}
		write(fd[0],msg,strlen(msg));
		sleep(1);
		printf("server recv #:%s\n",buf);
	}
	}
	return 0;
}
