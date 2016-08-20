#include <stdio.h>
#include <poll.h>

int main()
{
	struct pollfd poll_set[1];
	poll_set[0].fd = 0;
	poll_set[0].events = POLLIN;
	poll_set[0].revents = 0;

	int timeout = 5000;
	int max_fd = 0;
	while(1)
	{
		int num = 0;
		switch(num = poll(poll_set,max_fd+1,timeout))
		{
			case 0:
				printf("time out\n");
				break;
			case -1:
				printf("poll");
				break;
			default:
			{
				char buf[1024];
				ssize_t _s = read(0,buf,sizeof(buf));
				if(_s > 0)
				{
					buf[_s] = '\0';
					printf("echo #%s",buf);
				}
			}
			break;
		}
	}
	return 0;
}
