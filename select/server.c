#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>

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

	int fds[_MAX_SIZE_];
	int nSock = 0;
	for(; nSock< _MAX_SIZE_; nSock++)
	{
		fds[nSock] = -1;
	}
	fds[0] = listen_sock;
	int max_fd = listen_sock;
	fd_set rfds;

	int done = 0;
	while(!done)
	{
		FD_ZERO(&rfds);
		int i = 0;
		for(; i< _MAX_SIZE_; i++)
		{
			if(fds[i] != -1)
			{
				FD_SET(fds[i],&rfds);
				max_fd = max_fd > fds[i] ? max_fd:fds[i];
			}
		}

		struct timeval timeout = {5,0};
		switch(select(max_fd+1,&rfds,NULL,NULL,NULL))
		{
			case 0:
				printf("time out\n");
				break;
			case -1:
				perror("select");
				break;
			default:
				{			
					int index = 0;
					for(; index < _MAX_SIZE_; index++)
					{
						if(index == 0 && fds[index] == listen_sock && FD_ISSET(listen_sock,&rfds))
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
									if(fds[i]==-1)
									{
										fds[i] = new_fd;
										break;
									}
								}
								if(i == _MAX_SIZE_)
								{
									printf("fds is full close new_fd...\n");
									close(new_fd);
								}
							}
						}
						else if(fds[index] != -1 && FD_ISSET(fds[index],&rfds)) 
						{
							char buf[1024];
							ssize_t _s = read(fds[index],buf,sizeof(buf));
							if(_s > 0)
							{
								buf[_s] = '\0';
								printf("client fd :%d #:%s",fds[index],buf);
							}
							else if( _s == 0)
							{
								printf("client is closed...\n");
								FD_CLR(fds[index],&rfds);
								close(fds[index]);
								fds[index] = -1;
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
	/*while(1)
	  {
		int new_fd = accept(listen_sock,(struct sockaddr *)&peer,&len);
		if(new_fd < 0)
		{
			perror("accept");
			}
		
		printf("get a new client :ip : %s port :%d \n",inet_ntoa(peer.sin_addr),ntohs(peer.sin_port));
		char buf[1024];
		while(1)
		{
			ssize_t _s = read(new_fd,buf,sizeof(buf));
			if(_s > 0)
			{
				buf[_s ] = '\0';
				printf("client # %s",buf);
				write(new_fd,buf,strlen(buf));
			}
			else if(_s == 0)
			{
				printf("client is closed\n");
				break;
			}
			else
			{
				break;
			}
		}
	}*/
	return 0;
}
