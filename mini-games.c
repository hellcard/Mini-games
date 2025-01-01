#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h> // getuid();
#include "funcs.h"

#define DATAFILE "game.data"

struct user {
	int uid;
	int credits;
	int highscore;
	char username[100];
	int (*current_game) (); // pointer function
};

// void funcs

void register_new_user();
void update_user_data();
void show_score();
void jackpot();
void scan_name();
void play();
void fatal(char*);

// int funcs
int pick_num();
int dealer_no_match();
int take(int,int);
int get_user_data();

struct user player; // create player

void print_main_menu(){
	printf("---\n-+ MAIN MENU +-\n");
	printf("[ GAMES ]\n\t1 - guess the number\n\t2 - no matches\n");
	printf("[ INFO ]\n\t3 - current record\n\t4 - change username\n\t5 - return account to 100 credits\n\t6 - exit\n");
	printf("[ ABOUT %s ]\n\tCredits: %d\n---\n",player.username,player.credits);
}

int main(){
	int choice, last_game;

	srand(time(0)); // set rand generator

	if(get_user_data() == -1) register_new_user();

	while(choice != 6){
		print_main_menu();
		printf("Your choice: "); scanf("%d",&choice);

		if((choice < 1) || (choice > 7)) printf("there is no such choice\n");
		else if(choice < 3){
			if(choice != last_game){
				if(choice == 1) player.current_game = pick_num;
				else if(choice == 2) player.current_game = dealer_no_match;
			}
			play();
		}
		else if(choice == 3) show_score();
		else if(choice == 4){
			printf("=== Change username ===\n");
			printf("Enter new username: "); scan_name();
			printf("Username was succesful changed\n");
		}
		else if(choice == 5) player.credits = 100;
	}
	update_user_data();
	printf(":)\n");
	return 0;
}

int get_user_data(){
	int uid = getuid();
	struct user entry;

	FILE *f = fopen(DATAFILE,"r"); // open datafile in read mode
	if (f == NULL) return -1;

	while(fread(&entry,sizeof(struct user),1,f) == 1){
		if(entry.uid == uid){
			player = entry;
			fclose(f);
			return 1;
		}
	}
	fclose(f);
	return -1;
}


void register_new_user(){
	printf("---{ new user registration }---\n");
	printf("Enter your username: "); scan_name();

	player.uid = getuid();
	player.highscore = player.credits = 100;

	FILE *f = fopen(DATAFILE,"w");
	if(f == NULL) fatal("open writing file");

	fwrite(&player,sizeof(player),4,f);
	fclose(f);
	printf("Welcome, %s\n",player.username);
	printf("You have %d credits\n",player.credits);
}

void update_user_data(){
	int read_uid;

	FILE *f = fopen(DATAFILE,"r+b"); // open file in read+write mode
	if(f == NULL) fatal("open reading + writing file");
	struct user entry;

	while(fread(&entry,sizeof(struct user),1,f) == 1){
		if(player.uid == entry.uid){
			fseek(f,-sizeof(struct user), SEEK_CUR);

			entry.credits = player.credits;
			entry.highscore = player.highscore;
			strncpy(entry.username, player.username, 98);
			entry.username[99] = '\0';

			fwrite(&entry, sizeof(struct user), 1, f);
            break;
		}
	}
	fclose(f);
}

void show_score(){
	unsigned int top_score = 0;
	char top_name[100];
	struct user entry;

	printf("\n---Records---\n\n");

	FILE *f = fopen(DATAFILE,"r");
	if(f == NULL) fatal("open reading file");

	while(fread(&entry,sizeof(entry),1,f) == 1){
		if(entry.highscore > top_score){
			top_score = entry.highscore;
			strcpy(top_name, entry.username);
		}
	}
	fclose(f);

	if(top_score > player.highscore) printf("%s set a record %u\n",top_name,top_score);
	else printf("you now have %u record credits\n",player.highscore);

	printf("===\n");
}

void jackpot(){
	printf("\n---!!!JACKPOT!!!---\n\n");
	player.credits += 100;
	printf("you won 100 credits\n");
	printf("\n[INFO] You have %u credits\n\n",player.credits);
}

void scan_name(){
	char *namep, input_char = '\n';
	while(input_char == '\n') scanf("%c",&input_char);

	namep = (char*) player.username;
	while(input_char != '\n'){
		*namep = input_char;
		scanf("%c",&input_char);
		namep++;
	}
	*namep = 0;
}


int take(int available_credits,int prev_wager){
	int wager, total;

	printf("how many of your %d credits do you want to bet?\n>> ",available_credits);
	scanf("%d", &wager);

	if(wager < 1){
		printf("bet must be a positive number\n");
		return -1;
	}
	total = prev_wager + wager;
	if(total > available_credits){
		printf("you bet too high\n");
		return -1;
	}

	return wager;
}

void play(){
	int play_again = 1;
	int (*game) ();
	char selection;

	while(play_again){

		printf("[DEBUG] pointer %p\n",player.current_game);

		if(player.current_game() != -1){
			if(player.credits > player.highscore) player.highscore = player.credits;

			printf("!!--- Now you have %u credits ---!!\n",player.credits);
			update_user_data();

			printf("\ndo you want to play again?(y/n) ");
			selection = '\n';

			while (selection == '\n') scanf("%c",&selection);
			if(selection == 'n') play_again = 0;

		}
		else play_again = 0;
	}
}

int pick_num(){
	int pick, win_num;
	printf("\n---PICK A NUMBER---\n\n");
	printf("[ INFO ] This game costs 15 credits, choose a random number from 1 to 30 and if you guess correctly you will win a jackpot of 100 credits\n\n");

	win_num = (rand() % 30) + 1;
	if(player.credits < 10){
		printf("\n!!! You have %d credits, to participate in the game you need 10 credits, the highest credits are not enough to play :( !!\n\n",player.credits);
		return -1;
	}
	player.credits -= 15;

	printf("Credits: %d\n",player.credits);
	printf("Select a number from 1 to 30: "); scanf("%d",&pick);
	printf("\nWinning num: %d\n",win_num);

	if(pick == win_num) jackpot();
	else printf("- You didn't guess the number :(\n");
	return 0;
}

int dealer_no_match(){
	int j, numbers[16], wager = -1, match = -1;

	printf("\n***NO MATCHES***\n\n");
	printf("[ INFO ] In this game you can bet all the credits you have. \n\"Dealer\" choose random numbers from 1 to 100 and if they are all different, your double bet will be returned to you. \nOtherwise you lose the credits you bet.\n\n");

	if(player.credits == 0 || player.credits < 0){
		printf("you have no credits :(\n"); 
		return -1;
	}
	printf("Generation of 16 random numbers...\n");

	while(wager == -1){
		wager = take(player.credits, 0);
	}

	for(int i=0; i<16; i++){ // generation
			numbers[i] = (rand() % 100) + 1;
			printf("%3d\t",numbers[i]);
			if(i%8 == 7) printf("\n");
	}
	for(int i=0; i<15; i++){
		j = i + 1;
		while(j < 16){
			if(numbers[i] == numbers[j]) match = numbers[j];
			j++;
		}
	}
	if(match == -1){
		printf("!!! The number coincided %d!!!\n",match);
		printf("you lost %d credits :(\n",wager);
		player.credits -= wager;
	}
	else{
		printf("You won!! :))\n");
		player.credits += wager * 2;
	}
	return 0;
}