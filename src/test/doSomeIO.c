#include <stdio.h>

#define LINES 10000
#define IO_FILE "ioTest.txt"

int main()
{
	int i;
	FILE *f;
	while(1)
	{
		f = fopen(IO_FILE, "rw");
		if(f == NULL)
			continue;
		
		for(i = 0; i < LINES; ++i)
			fprintf(f, "jhudssuhfksljhspokoipewjsmyhbahjdshgghbashjwsjhdsbfjbhsjjhdfvhdvbdfbvsdhjvbdfjhvnxnbgjhsbdjhfuzhshbfhdvhjfuzuhgjhdnb vgdsgkjndfjvhbfdhvnkhdbvjdkbvjdgbvhjdbhvhdfbhbxcahnjvchjwsgy<hgscvaskmhsdahjmcdhbjashjhsnakjasdliahefjsljdfckmslhifd.b\n");
		
		fclose(f);
	}
	
}