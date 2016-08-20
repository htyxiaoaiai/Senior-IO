#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/epoll.h>


#define  _MAX_SIZE_ 128

void Usge(const char* proc)
{
	printf("Usge %s [IP] [PORT] \n",proc);
}

//create socket
int startUp(char* _ip,char* _port)
{
	int sock =socket(AF_INET,SOCK_STREAM,0);
	if(sock < 0)
	{
		perror("socket");
		exit(1);
	}

	struct sockaddr_in local;
	local.sin_family = AF_INET;
	local.sin_port = htons(atoi(_port));
	local.sin_addr.s_addr = inet_addr(_ip);

	int opt =1;
	setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
	if(bind(sock,(struct sockaddr*)&local,sizeof(local)) < 0)
	{
		perror("bind");
		exit(2);
	}


	if(listen(sock,5) < 0)
	{
		perror("listen");
		exit(3);
	}

	return sock;
}

int main(int argc, char* argv[])
{
	if(argc != 3)
	{
		Usge(argv[0]);
	}

	int listen_sock = startUp(argv[1],argv[2]);
	
	int epfd = epoll_create(256);
	if(epfd < 0)
	{
		perror("epoll_create");
		close(listen_sock);
		return 1;
	}

	struct epoll_event ev,revs[128];
	int size = sizeof(revs)/sizeof(revs[0]);
	int j = 0;
	for(; j<size; j++)
	{
		revs[j].events = 0;
		revs[j].data.fd = -1;
	}

	ev.data.fd = listen_sock;
	ev.events = EPOLLIN;

	if(epoll_ctl(epfd,EPOLL_CTL_ADD,listen_sock,&ev) < 0)
	{
		perror("epoll_ctl");
		close(listen_sock);
		close(epfd);
		return 2;
	}
	
	int timeout = -1;
	int done = 0;
	while(!done)
	{
		int num = epoll_wait(epfd,revs,_MAX_SIZE_,timeout);
		switch(num)
		{
			case 0:
				printf("timeout...\n");
				break;
			case -1:
				perror("epoll_wait");
				close(listen_sock);
				close(epfd);
				return 3;
				break;
			default:
				{
					struct sockaddr_in peer;
					socklen_t len = sizeof(peer);
					int i =0;
					for(; i<num; i++)
					{
						int rsock = revs[i].data.fd;
						//accept
						if(revs[i].events & EPOLLIN && (rsock == listen_sock))
						{
							int new_fd = accept(listen_sock,(struct sockaddr*)&peer,&len);
							if(new_fd < 0)
							{
								perror("accept");
								return 4;
							}
							printf("get a new client IP :%s PORT : %d\n",inet_ntoa(peer.sin_addr), ntohs(peer.sin_port));
							ev.events = EPOLLIN;
							ev.data.fd = new_fd;
							epoll_ctl(epfd,EPOLL_CTL_ADD,new_fd,&ev);
						}
						//read
						else if(revs[i].events & EPOLLIN)
						{
							char buf[1024];
							ssize_t _s = read(rsock,buf,sizeof(buf));
							if(_s > 0)
							{
								buf[_s] = '\0';
								printf("client # %s",buf);
								ev.events = EPOLLOUT;//modify write
								epoll_ctl(epfd,EPOLL_CTL_MOD,rsock,&ev);
							}
							else if (_s == 0)
							{
								printf("client is closed...\n");
								epoll_ctl(epfd,EPOLL_CTL_DEL,rsock,NULL);
								close(rsock);
							}
						}
						//write
						else if(revs[i].events & EPOLLOUT)
						{
							const char* msg = "HTTP/1.1 200 OK\r\n\r\n<html><h1>hello world ++_++||</h1></html>\r\n";
							write(rsock,msg,strlen(msg));
							//ev.events = EPOLLIN;
							//epoll_ctl(epfd,EPOLL_CTL_MOD,rsock,&ev);
							epoll_ctl(epfd,EPOLL_CTL_DEL,rsock,NULL);
							close(rsock);
						}
					}
				}
				break;
		}
	}
	
	return 0;
}
