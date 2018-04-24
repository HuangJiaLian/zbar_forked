#include <stdio.h>
void main()
{
	int nedge = 20;
	nedge <<= 1;
	printf("nedge = 20, after  \"nedge <<= 1\" nedge = %d\n", nedge);
	
	nedge = 60;
	nedge >>= 2;
	printf("nedge = 60, after  \"nedge >>= 2\" nedge = %d\n", nedge);
	
	int num = 40; 
	int i = num >> 2;
	printf("i = %d, num = %d\n", i,num);
	printf("%d\n", num - ((num>>2)<<1));	
}
