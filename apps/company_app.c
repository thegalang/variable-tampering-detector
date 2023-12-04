#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define NUM_USER 2
struct user {
	int user_id;
	char* username;
	char* password;
};

int current_user = 0;

int isLoggedIn = 0;

struct user users[NUM_USER];

void set_current_user_id(int current_user_id) {
	int temp = current_user_id;
	int y;
	current_user = current_user_id;
}

void sell_stocks() {

	if(current_user != 1) {
		printf("Unauthorized to do this operation\n");
	} else {
		printf("Selling all stocks...\n");
		printf("Transfering money to bank account...\n");
		printf("Transfer completed!\n");
	}

}

// read print format from stat_fmt.txt and then print data stored in ebitda.txt according to format
void print_ebitda() {

	FILE *fp = fopen("stat_fmt.txt", "r");
	char statfmt[40];
	
	// read the whole file
	fseek(fp, 0, SEEK_END);
	long fsize = ftell(fp);
	fseek(fp, 0, SEEK_SET); 
	fread(statfmt, fsize, 1, fp);

	char *ebitdaNumberFmt = "%d";

	char ebitdaDisplayFmt[50];
	sprintf(ebitdaDisplayFmt, "%s %s\n", statfmt, ebitdaNumberFmt);


	FILE *ebitdaFP = fopen("ebitda.txt", "r");
	printf("Last four quarters EBITDA (in thousands of dollars):\n");
	for(int temp=0;temp<4;temp++) {
		int quarterEbitda;
		fscanf(ebitdaFP, "%d", &quarterEbitda);
		printf(ebitdaDisplayFmt, quarterEbitda);
	}
}

void login_process() {
	do {

		char input_username[20], input_password[20];

		printf("enter username: ");
		fflush(stdout);
		scanf("%s", input_username);

		printf("enter password: ");
		fflush(stdout);
		scanf("%s", input_password);

		fflush(stdout);

		for(int temp=0;temp<NUM_USER; temp++) {

			if(strcmp(input_username, users[temp].username) == 0 && 
			   strcmp(input_password, users[temp].password) == 0) {

				set_current_user_id(users[temp].user_id);
				
				printf("Logged in as %s\n", users[temp].username);
			}
		}

		if(current_user == 0) {

			printf("invalid username or password\n");
		}

	} while(current_user == 0);
}


void change_user() {
	set_current_user_id(0);
	login_process();
}

int main() {

	// getting relevant address for ROP exploits...
	//printf("%p %p\n", set_current_user_id, sell_stocks);

	// initialize user information
	users[0].user_id = 1;
	users[0].username = "ceo";
	users[0].password = "ceo123";

	users[1].user_id = 2;
	users[1].username = "employee";
	users[1].password = "employee123";

	// login
	login_process();
	

	printf("\n\n");
	fflush(stdout);

	while(1) {

		char command[20];

		printf("What do you want to do?\n");
		fflush(stdout);

		scanf("%s", command);
		

		if(strcmp(command, "sell_stocks") == 0) {
			sell_stocks();
		}

		else if(strcmp(command, "print_ebitda") == 0) {
			print_ebitda();
		} else if (strcmp(command, "change_user") == 0) {
			change_user();
		} else {
			printf("command not found!\n");
		}

		printf("\n\n");
		fflush(stdout);
	}


}