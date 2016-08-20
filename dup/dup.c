#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int main()
{
	int fd = open("./log",O_CREAT|O_RDWR,0644);
	if(fd < 0)
	{
		perror("open");
		return 1;
	}
	close(1);
	int new_fd = dup(fd);
	if(dup < 0)
	{
		perror("dup");
		return 2;
	}
	close(fd);

	char * msg = "hello world";
	int count = 5;
	while(count)
	{
		printf("%s %d\n",msg,count--);
	}
	fflush(stdout);
	close(new_fd);
	return 0;
}
