#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <poll.h>

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

	struct pollfd pfds[_MAX_SIZE_];
	int nSock = 0;
	for(; nSock< _MAX_SIZE_; nSock++)
	{
		pfds[nSock].fd = -1;
		pfds[nSock].events = 0;
		pfds[nSock].revents = 0;
	}
	pfds[0].fd = listen_sock;
	pfds[0].events = POLLIN;

	int max_fd = listen_sock;

	int done = 0;
	while(!done)
	{
		int i = 0;
		for(; i< _MAX_SIZE_; i++)
		{
			if(pfds[i].fd != -1)
			{
				max_fd = max_fd > pfds[i].fd ? max_fd:pfds[i].fd;

			}
		}

		int timeout = 5000;
		int ret = poll(pfds,max_fd+1,timeout);
		switch(ret)
		{
			case 0:
				printf("time out\n");
				break;
			case -1:
				perror("poll");
				break;
			default:
				{			
					int index = 0;
					for(; index < _MAX_SIZE_; index++)
					{
						if(index == 0 && pfds[index].fd == listen_sock && pfds[index].events == POLLIN)
						{
							//accept
							struct sockaddr_in peer;
							socklen_t len = sizeof(peer);
							int new_fd = accept(listen_sock,(struct sockaddr *)&peer,&len);
							if(new_fd < 0)
							{
								perror("accept");
								return 1;
							}
							else
							{
								printf("get a new client :ip : %s port :%d \n",inet_ntoa(peer.sin_addr),ntohs(peer.sin_port));
								int i =0;
								for(i =0; i < _MAX_SIZE_; i++)
								{
									if(pfds[i].fd==-1)
									{
										pfds[i].fd = new_fd;
										break;
									}
								}
							}
						}
						else if(pfds[index].fd != -1 && pfds[index].events == POLLIN) 
						{
							char buf[1024];
							ssize_t _s = read(pfds[index].fd,buf,sizeof(buf));
							if(_s > 0)
							{
								buf[_s] = '\0';
								printf("client fd :%d #:%s",pfds[index].fd,buf);
							}
							else if( _s == 0)
							{
								printf("client is closed...\n");
								pfds[index].events = 0;
								close(pfds[index].fd);
								pfds[index].fd = -1;
							}
							else
							{
								perror("read");
							}
						}
					}
				}
				break;
		}
	}
	return 0;
}
