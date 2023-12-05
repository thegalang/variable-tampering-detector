#include "stdio.h"

int glob1 = 0;
int glob2 = 0;
int glob3 = 0;

void c(int x) {
	int y = x;
	glob3 = x;
}

void a(int x) {
	int y = x;
	glob1 = x;
} 

void b(int x) {
	int y = x;
	glob2 = x;
}

void d(int x) {
	int y = x;
	glob3 = x;
}

int main() {
	a(10);
	b(20);
	c(30);

	printf("%d %d %d\n", glob1, glob2, glob3);

	a(40);
	b(50);
	c(60);

	printf("%d %d %d\n", glob1, glob2, glob3);

	d(10);

	printf("%d %d %d\n", glob1, glob2, glob3);
}