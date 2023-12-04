#include <stdio.h>
#include <dlfcn.h>

int glob1=0;
int glob2 = 0;
int glob3 = 0;

int main() {

	//FILE *fp = fopen("input.txt", "r");
	int newval;
	int globid = 0;
	while(scanf("%d", &newval) != EOF) {



		if(globid == 0) {
			glob1 += newval;
		}
		if(globid == 1) {
			glob2 += newval;
		}
		if(globid == 2) {
			glob3 += newval;
		}

		globid = (globid + 1) % 3;

	}

	printf("glob1: %d, glob2: %d, glob3: %d\n", glob1, glob2, glob3);

}