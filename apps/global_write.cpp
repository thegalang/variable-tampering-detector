#include <stdio.h>

int glob1;
int glob2 = 10;

int main() {

	int x;
	glob1 = 6969;
	printf("inputted: %d\n", glob1);

	glob2 = glob1 * 20;
	printf("%d %d %016llx %016llx\n", glob1, glob2, &glob1, &glob2);
}