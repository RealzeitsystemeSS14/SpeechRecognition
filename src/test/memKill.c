#include <stdlib.h>

#define DEF_SIZE 200
#define CHUNK_SIZE 1024
#define MIN(a,b) ((a) < (b) ? (a) : (b))


void **mem;
int size;
int count;

static int resize(int newsize)
{
	int i;
	void **tmp = calloc(0, newsize);
	if(tmp == NULL)
		return 1;
		
	int tocopy = MIN(newsize, count);
	for(i = 0; i < tocopy; ++i)
		tmp[i] = mem[i];
	free(mem);
	mem = tmp;
	size = newsize;
	
	return 0;
}

int main()
{
	mem = calloc(0, DEF_SIZE);
	size = DEF_SIZE;
	count = 0;
	
	while(1) {
		mem[count] = calloc(0, CHUNK_SIZE);
		if(mem[count] == NULL) {
			usleep(100000);
			continue;
		}
		
		if(count == size)
			while(resize(size * 2) != 0)
				usleep(100000);
		
	}
	
	return 0;
}