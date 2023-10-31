#include <stdio.h>
#include <dlfcn.h>

int glob1;
int glob2 = 10;

int main() {

	int x;
	glob1 = 6969;
	printf("inputted: %d\n", glob1);

	scanf("%d", &glob1);

	printf("second glob: %d\n", glob1);

	glob1 = 7766;

	glob2 = glob1 * 20;
	printf("%d %d %016llx %016llx\n", glob1, glob2, &glob1, &glob2);
}