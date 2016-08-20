#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

int main()
{
	int fd = open("./tmp",O_CREAT|O_RDWR, 0644);
	if(fd < 0)
	{
		perror("open");
		return 1;
	}

	int new_fd = dup2(fd,1);

	char* msg ="hello world";
	int count = 5;
	while(count)
	{
		printf("%s %d\n",msg,count--);
	}
	fflush(stdout);
	close(new_fd);
	return 0;
}
