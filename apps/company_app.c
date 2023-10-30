#include <stdio.h>
#include <string.h>

#define NUM_USER 2
struct user {
	int user_id;
	char* username;
	char* password;
};

int current_user = 0;

struct user users[NUM_USER];


int main() {

	// initialize user information
	users[0].user_id = 1;
	users[0].username = "ceo";
	users[0].password = "ceo123";

	users[1].user_id = 1000;
	users[1].username = "employee";
	users[1].password = "employee123";

	// login

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

				current_user = users[temp].user_id;
				printf("logged in as %s\n", users[temp].username);
			}
		}

		if(current_user == 0) {

			printf("invalid username or password\n");
		}

	} while(current_user == 0);

	printf("\n\n");
	fflush(stdout);

	while(1) {

		char command[20];

		printf("What do you want to do?\n");
		fflush(stdout);

		scanf("%s", command);
		

		if(strcmp(command, "sell_stocks") == 0) {
			if(current_user != 1) {
				printf("Unauthorized to do this operation\n");
			} else {
				printf("Selling all stocks...\n");
				printf("Transfering money to bank account...\n");
			}
		}

		if(strcmp(command, "statistics") == 0) {
			printf("displaying analytics...\n");
		}

		printf("\n\n");
		fflush(stdout);
	}


}