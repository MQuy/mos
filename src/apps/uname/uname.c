#include <shared/ioctls.h>
#include <shared/mman.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	write(1, "mOS 0.0.1", 9);
	return 0;
}
