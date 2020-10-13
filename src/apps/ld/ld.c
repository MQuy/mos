#include <shared/ioctls.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	printf("what is your name: ");
	fflush(stdout);

	char name[100] = {0};
	scanf("%s", name);
	printf("\nyour name is %s", name);

	return 0;
}
