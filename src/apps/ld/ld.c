#include <stdio.h>

int main(int argc, char *argv[])
{
	printf("hi, what is your name? ");
	fflush(stdout);

	char name[100] = {0};
	scanf("%s", name);
	printf("\nyour name is %s\n", name);

	return 0;
}
