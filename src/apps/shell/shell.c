#include <include/mman.h>
#include <libc/unistd.h>
#include <stdbool.h>

#define PREFIX_LINE "> "
#define CHARACTERS_PER_LINE 256

struct shell
{
	char cwd[256];
	gid_t foreground_gid;
};

struct shell *ishell;

int main()
{
	int32_t fd = shm_open("shell", 0, 0);
	ftruncate(fd, sizeof(struct shell));
	ishell = (struct shell *)mmap(NULL, sizeof(struct shell), PROT_WRITE | PROT_READ, MAP_SHARED, fd);
	write(1, PREFIX_LINE, sizeof(PREFIX_LINE) - 1);

	char line[CHARACTERS_PER_LINE] = {0};
	int ret;
	while (true)
	{
		ret = read(1, line, sizeof(line));
		if (ret < 0)
			continue;
	}
}
