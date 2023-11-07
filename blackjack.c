#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


#define SHOE_SIZE 312
#define HAND_SIZE 22
#define MAX_HANDS 4
#define CARD_SIZE 3


void create_shoe(char** shoe);
void shuffle(char ** array, size_t n, int( * rand_func)(void));
char* pop_shoe(char ** shoe);
void reset_hands(void);
void print_shoe(char ** shoe);
void print_player_hand(int hand_index);
void player_turn(char ** shoe);
void print_house_hand(void);
void print_house_hand_hidden(void);
int hand_value_player(int hand_index);
int hand_value_house(void);
void house_turn(char ** shoe);


//create the player struct
struct player {
    int value[MAX_HANDS]; //the total value of each hand
    int result[MAX_HANDS]; //the result of each hand (bust or stood)
    int money; //players total money
    int bet_size; //the size of the most recent bet
    int has_split; //keep track if the player has split yet
    char hand[MAX_HANDS][HAND_SIZE][CARD_SIZE]; //array of each card in each hand
    int hand_index; //index for keeping track of split hands
    int next_slot[MAX_HANDS]; //keep track of the next empty card slot for each hand
};


//create the house struct
struct house {
    int value; //the total value of the hand
    int result; //the result of the hand (bust or stood)
    int next_slot; //keep track of the next empty card slot
    char hand[HAND_SIZE][CARD_SIZE]; //array of each card in each hand
};

struct player player;
struct house house;



int main(void) {
    int i, j, x, bet_size, error_check, all_hands_bust, player_value, house_value;
    char* shoe[SHOE_SIZE]; //array that stores 6 decks of cards

    //create a shoe of 6 decks and then shuffle the shoe randomly
    create_shoe(shoe);
    srand(time(NULL));
	shuffle(shoe, SHOE_SIZE, rand);

    //initialize player and house structures
    player.money = 1500;
    player.bet_size = 0;
    player.has_split = 0;
    player.hand_index = 0;
    house.value = 0;
    house.result = 0;
    house.next_slot = 0;
    for(i=0; i<MAX_HANDS; i++) {
        player.value[i] = 0;
        player.result[i] = 0;
        player.next_slot[i] = 0;
        for (j=0;j<HAND_SIZE;j++) {
            for (x=0; x<CARD_SIZE; x++) {
                player.hand[i][j][x] = *"";
            }
        }
    }

    printf("Welcome to the best blackjack simulator!\n");
    
    //run the game until the player is bankrupt
    while(player.money > 0) {
        all_hands_bust = 1;
        printf("\n *** new hand ***\n");
        reset_hands();

        //get user input for how much they would like to bet
        while(1) {
            printf("You have $%d, please enter a bet size: ", player.money);
            error_check = scanf("%d", &bet_size);
            if (error_check != 1) {
                printf("Error: Invalid input. Exiting now.\n");
                exit(0);
            }

            //check the bet size is valid, if so break the loop
            if((bet_size < 1) || (bet_size>player.money)) {
                printf("Please enter a number between 1 and %d\n", player.money);
            } else {
                player.bet_size = bet_size;
                player.money -= bet_size;
                break;
            }
        }

        //deal the cards to the player and the house
        for (i=0; i<2; i++) {
            if(!strcpy(&player.hand[player.hand_index][i][0], pop_shoe(shoe))) {
                printf("Error dealing the cards. Exiting now.\n");
                exit(0);
            }

            if(!strcpy(&house.hand[i][0], pop_shoe(shoe))) {
                printf("Error dealing the cards. Exiting now.\n");
                exit(0);
            }
        }
        player.next_slot[player.hand_index] += 2;
        house.next_slot += 2;
        print_house_hand_hidden();
        print_player_hand(player.hand_index);


        //have the player take their turn
        player_turn(shoe);

        //check if the player stood on any hands
        for(i=0; i<MAX_HANDS; i++) {
            if(player.result[i] == 1) {
                all_hands_bust = 0;
            }
        }

        //have the house take it's turn if the player did not bust
        if(all_hands_bust) {
            printf("The player has bust, dealer automatically wins\n");
        } else {
            house_turn(shoe);
        }

        //check who won each hand, award money on win
        house_value = house.value;
        for(i=0; i<player.hand_index+1; i++) {
            player_value = player.value[i];
            if(player_value > 21 ) { //if the player bust
                printf("Player has lost to the house with hand %d\n",i+1);
            } else if (house_value > 21) { //if the house bust
                printf("Player has beat the house with hand %d\n",i+1);
                player.money += (player.bet_size * 2);
            } else if (player_value > house_value) { //if the player beat the house
                printf("Player has beat the house with hand %d\n",i+1);
                player.money += (player.bet_size * 2);
            } else if (player_value == house_value) { //if the player and house hand's have the same numerical value
                if(player.next_slot[i] == 2 && house.next_slot!=2 && player_value == 21) {
                    //the player has blackjack and the house does not
                    printf("Player has beat the house with blackjack with hand %d\n",i+1);
                    player.money += (player.bet_size * 2);
                } else if (player.next_slot[i] != 2 && house.next_slot==2 && player_value == 21) {
                    //the house has blackjack and the player does not
                    printf("Player has lost to the house's blackjack with hand %d\n",i+1);
                } else {
                    printf("Player has tied with the house with hand %d\n",i+1);
                    player.money += player.bet_size;
                }
            } else {
                printf("Player has lost to the house with hand %d\n",i+1);
            }
        }
    }

    //end the game
    printf("You have run out of money. Thank you for playing!\n");
    return 1;
}


//have the house take it's turn, set result to 2 on bust and 1 on stand
void house_turn(char ** shoe) {
    while(1) {
        house.value = hand_value_house();
        //check if the dealer has busted or needs to stand
        if(house.value > 21) {
            print_house_hand();
            house.result = 2;
            printf("Dealer busts!\n");
            return;
        } else if (house.value >= 17) {
            printf("Dealer stands at ");
            print_house_hand();
            house.result = 1;
            return;
        }

        //the dealer must hit, so deal the next card
        if(!strcpy(house.hand[house.next_slot], pop_shoe(shoe))) {
                printf("Error dealing the next card exiting now\n");
                exit(0);
            }
            house.next_slot += 1;
    }
}


//have the player take their turn, set result to 2 on bust and 1 on stand
void player_turn(char ** shoe) {
    int hand_index = 0, error_check, num_splits = 0, i;
    char action[12];
    while(1) {
        //get user input for their desired action
        printf("please take an action (hit, double_down, split, stand): ");
        error_check = scanf("%11s", action);
        if (error_check != 1) {
                printf("Error: Invalid input. Exiting now.\n");
                exit(0);
        }

        if(!strcmp(action, "hit")) { 
            //deal the next card
            if(!strcpy(player.hand[hand_index][player.next_slot[hand_index]], pop_shoe(shoe))) {
                printf("Error dealing the next card exiting now\n");
                exit(0);
            }
            player.next_slot[hand_index] += 1;
            player.value[hand_index] = hand_value_player(hand_index);
            if(player.value[hand_index] > 21) { // if the player busted
                print_player_hand(hand_index);
                printf("bust!\n");
                player.result[hand_index] = 2;
            
                //check if this is the last hand the player has
                if (hand_index == num_splits) {
                    return;
                }
                //there are more hands, deal with these
                hand_index += 1;
            } else if (player.next_slot[hand_index] == 2 && !strncmp(player.hand[hand_index][0], "A", 1)) {
                print_player_hand(hand_index);

                //if the player is hitting after splitting aces the hand is over
                printf("hand over as you split aces\n");
                player.result[hand_index] = 1;
                //check if this is the last hand the player has
                if (hand_index == num_splits) {
                    return;
                }
                //there are more hands, deal with these
                hand_index += 1;
            }
        } else if(!strcmp(action, "double_down")) {
            //check the player can actually split
            if(player.has_split) {
                printf("You cannot double down after splitting\n");
                continue;
            } else if (hand_value_player(hand_index) == 21) {
                printf("You cannot double down on 21\n");
                continue;
            } else if(player.next_slot[hand_index] != 2) {
                printf("You cannot double down after hitting\n");
                continue;
            } else if(player.bet_size > player.money) {
                printf("You do not have enough money to double down\n");
                continue;
            }

            //the player can double down, deal one last card
            player.money -= player.bet_size;
            player.bet_size += player.bet_size;
            if(!strcpy(player.hand[hand_index][player.next_slot[hand_index]], pop_shoe(shoe))) {
                printf("Error dealing the next card exiting now\n");
                exit(0);
            }
            player.next_slot[hand_index] += 1;
            print_player_hand(hand_index); 
            player.value[hand_index] = hand_value_player(hand_index);

            if(player.value[hand_index] > 21) { // if the player busted
                printf("bust!\n");
                player.result[hand_index] = 2;
            } else {
                player.result[hand_index] = 1;
            }
            return;
        } else if(!strcmp(action, "stand")) {
            player.result[hand_index] = 1;
            player.value[hand_index] = hand_value_player(hand_index);
            //check if this is the last hand the player has
            if (hand_index == num_splits) {
                return;
            }
            //there are more hands, deal with these
            hand_index += 1;
        }  else if(!strcmp(action, "split")) {
            printf("cards: %s %s\n", &player.hand[hand_index][0][0], &player.hand[hand_index][1][0]);

            //check the player is able to split
            if(player.next_slot[hand_index] != 2) {
                printf("You do not have the right amount of cards to split\n");
                continue;
            } else if ((strncmp(&player.hand[hand_index][0][0], &player.hand[hand_index][1][0], 1)) && hand_value_player(hand_index) != 20) {
                printf("You cannot split this hand\n");
                continue;
            } else if(num_splits == 3) {
                printf("You have already split the max amount of times\n");
                continue;
            } else if (player.bet_size > player.money) {
                printf("You do not have enough money to split\n");
                continue;
            }

            //split the hand
            player.money -= player.bet_size;
            i=1;
            while(strcmp(player.hand[hand_index+i][0], "")){ //find the next available hand array
                i++;
            }
            if(!strcpy(player.hand[hand_index+i][0], player.hand[hand_index][1])) {
                printf("Error splitting the hand, now exiting\n");
                exit(0);
            }
            if(!strcpy(player.hand[hand_index][player.next_slot[hand_index]-1], "")) {
                printf("Error splitting the hand, exiting now\n");
                exit(0);
            }
            num_splits++;
            player.hand_index++;
            player.next_slot[hand_index] = 1;
            player.next_slot[hand_index+i] = 1;
            player.has_split = 1;
        } else {
            printf("you did not choose a valid input\n");
        }
        print_player_hand(hand_index);
    }
}


//get the numerical value of the corresponding hand
int hand_value_player(int hand_index) {
    int i, ace = 0, value = 0;

    for(i=0; i<HAND_SIZE; i++) {
        if(!strncmp(&player.hand[hand_index][i][0], "", 1)) { //if the card slot is empty
            break;
        } else if (!strncmp(&player.hand[hand_index][i][0], "A", 1)) {
            value += 11;
            ace += 1; //keep track of the number of aces
        } else if ((!strncmp(&player.hand[hand_index][i][0], "K", 1)) || (!strncmp(&player.hand[hand_index][i][0], "Q", 1)) || (!strncmp(&player.hand[hand_index][i][0], "J", 1)) || (!strncmp(&player.hand[hand_index][i][0], "T", 1))) {
            value += 10;
        } else {
            value += atoi(&player.hand[hand_index][i][0]);
        }
    }

    while (value > 21 && ace > 0) {
        ace -= 1;
        value -= 10;
    }
    return value;
}


//get the numerical value of the corresponding hand
int hand_value_house(void) {
    int i, ace = 0, value = 0;

    for(i=0; i<HAND_SIZE; i++) {
        if(!strncmp(&house.hand[i][0], "", 1)) { //if the card slot is empty
            break;
        } else if (!strncmp(&house.hand[i][0], "A", 1)) {
            value += 11;
            ace += 1; //keep track of the number of aces
        } else if ((!strncmp(&house.hand[i][0], "K", 1)) || (!strncmp(&house.hand[i][0], "Q", 1)) || (!strncmp(&house.hand[i][0], "J", 1)) || (!strncmp(&house.hand[i][0], "T", 1))) {
            value += 10;
        } else {
            value += atoi(&house.hand[i][0]);
        }
    }

    while (value > 21 && ace > 0) {
        ace -= 1;
        value -= 10;
    }
    return value;
}


//print the corresponding player's hand 
void print_player_hand(int hand_index) {
    int i;

    printf("player hand: ");
    for(i=0; i<HAND_SIZE; i++) {
        printf("%s ", player.hand[hand_index][i]);
    }
    printf("\n");
}


//print the house's hand
void print_house_hand(void) {
    int i;

    for(i=0; i<HAND_SIZE; i++) {
        printf("%s ", house.hand[i]);
    }
    printf("\n");
}


//print the house's hand but hide their first card
void print_house_hand_hidden(void) {
    printf("house's hand: ? %s\n", house.hand[1]);
}


//reset the player and house data between hands
void reset_hands(void) {
    int i,j,x;

    player.has_split = 0;
    player.hand_index = 0;
    house.value = 0;
    house.result = 0;
    house.next_slot = 0;
    for(i=0; i<MAX_HANDS; i++) {
        player.value[i] = 0;
        player.result[i] = 0;
        player.next_slot[i] = 0;
        for (j=0;j<HAND_SIZE;j++) {
            if(j<3) {
                house.hand[i][j] = *"";
            }
            for (x=0; x<CARD_SIZE; x++) {
                player.hand[i][j][x] = *"";
            }
        }
    }
}

//pop the next card off the shoe. if the shoe is empty create and shuffle
//a new shoe and pop the card off that
char* pop_shoe(char ** shoe) {
    int i;
    char* tmp = shoe[0]; //get the next card

    //if the shoe has run out of cards, reshuffle the cards
    if(!strcmp(tmp, "\0")) {
        printf("The shoe has run out. Reshuffling now.\n");
        create_shoe(shoe);
        srand(time(NULL));
	    shuffle(shoe, SHOE_SIZE, rand);
        tmp = shoe[0]; //get the next card
    }
    
    //move the rest of the cards one closer to the top
    for(i=0; i< SHOE_SIZE-1; i++) {
        shoe[i] = shoe[i+1];
    }
    shoe[SHOE_SIZE-1] = ("\0");

    return tmp;
}


//debugging function, used to print all the cards in the shoe
void print_shoe(char ** shoe) {
    int i;

    printf("shoe: ");
    for(i=0; i<SHOE_SIZE; i++) {
        printf("%s ",shoe[i]);
    }
    printf("\n");
}


//shuffle a shoe of cards randomly. This function is borrowed from 
//www.w3resource.com
void shuffle(char ** shoe, size_t n, int( * rand_func)(void)) {
  for (int i = n - 1; i > 0; i--) {
    int j = rand_func() % (i + 1);
    char* tmp = shoe[i];
    shoe[i] = shoe[j];
    shoe[j] = tmp;
  }
}


//assign the cards to the array
void create_shoe(char** shoe) {
    int i;
    char* all_cards[SHOE_SIZE] = {"AH", "AH", "AH", "AH", "AH", "AH", "AD", "AD", "AD", "AD", "AD", "AD", "AC", "AC", "AC", "AC", "AC", "AC", "AS", "AS", "AS", "AS", "AS", "AS", "KH", "KH", "KH", "KH", "KH", "KH", "KD", "KD", "KD", "KD", "KD", "KD", "KC", "KC", "KC", "KC", "KC", "KC", "KS", "KS", "KS", "KS", "KS", "KS", "QH", "QH", "QH", "QH", "QH", "QH", "QD", "QD", "QD", "QD", "QD", "QD", "QC", "QC", "QC", "QC", "QC", "QC", "QS", "QS", "QS", "QS", "QS", "QS", "JH", "JH", "JH", "JH", "JH", "JH", "JD", "JD", "JD", "JD", "JD", "JD", "JC", "JC", "JC", "JC", "JC", "JC", "JS", "JS", "JS", "JS", "JS", "JS", "TH", "TH", "TH", "TH", "TH", "TH", "TD", "TD", "TD", "TD", "TD", "TD", "TC", "TC", "TC", "TC", "TC", "TC", "TS", "TS", "TS", "TS", "TS", "TS", "9H", "9H", "9H", "9H", "9H", "9H", "9D", "9D", "9D", "9D", "9D", "9D", "9C", "9C", "9C", "9C", "9C", "9C", "9S", "9S", "9S", "9S", "9S", "9S", "8H", "8H", "8H", "8H", "8H", "8H", "8D", "8D", "8D", "8D", "8D", "8D", "8C", "8C", "8C", "8C", "8C", "8C", "8S", "8S", "8S", "8S", "8S", "8S", "7H", "7H", "7H", "7H", "7H", "7H", "7D", "7D", "7D", "7D", "7D", "7D", "7C", "7C", "7C", "7C", "7C", "7C", "7S", "7S", "7S", "7S", "7S", "7S", "6H", "6H", "6H", "6H", "6H", "6H", "6D", "6D", "6D", "6D", "6D", "6D", "6C", "6C", "6C", "6C", "6C", "6C", "6S", "6S", "6S", "6S", "6S", "6S", "5H", "5H", "5H", "5H", "5H", "5H", "5D", "5D", "5D", "5D", "5D", "5D", "5C", "5C", "5C", "5C", "5C", "5C", "5S", "5S", "5S", "5S", "5S", "5S", "4H", "4H", "4H", "4H", "4H", "4H", "4D", "4D", "4D", "4D", "4D", "4D", "4C", "4C", "4C", "4C", "4C", "4C", "4S", "4S", "4S", "4S", "4S", "4S", "3H", "3H", "3H", "3H", "3H", "3H", "3D", "3D", "3D", "3D", "3D", "3D", "3C", "3C", "3C", "3C", "3C", "3C", "3S", "3S", "3S", "3S", "3S", "3S", "2H", "2H", "2H", "2H", "2H", "2H", "2D", "2D", "2D", "2D", "2D", "2D", "2C", "2C", "2C", "2C", "2C", "2C", "2S", "2S", "2S", "2S", "2S", "2S"};
    //char* all_cards[SHOE_SIZE] = {"AH", "TH", "AH", "TH", "AH", "AH", "AH", "AH", "AD", "AD", "AD", "AD", "AD", "AD", "AC", "AC", "AC", "AC", "AC", "AC", "AS", "AS", "AS", "AS", "AS", "AS", "KH", "KH", "KH", "KH", "KH", "KH", "KD", "KD", "KD", "KD", "KD", "KD", "KC", "KC", "KC", "KC", "KC", "KC", "KS", "KS", "KS", "KS", "KS", "KS", "QH", "QH", "QH", "QH", "QD", "QD", "QD", "QD", "QD", "QD", "QC", "QC", "QC", "QC", "QC", "QC", "QS", "QS", "QS", "QS", "QS", "QS", "JH", "JH", "JH", "JH", "JH", "JH", "JD", "JD", "JD", "JD", "JD", "JD", "JC", "JC", "JC", "JC", "JC", "JC", "JS", "JS", "JS", "JS", "JS", "JS", "TH", "TH", "TH", "TH", "TH", "TH", "TD", "TD", "TD", "TD", "TD", "TD", "TC", "TC", "TC", "TC", "TC", "TC", "TS", "TS", "TS", "TS", "TS", "TS", "9H", "9H", "9H", "9H", "9H", "9H", "9D", "9D", "9D", "9D", "9D", "9D", "9C", "9C", "9C", "9C", "9C", "9C", "9S", "9S", "9S", "9S", "9S", "9S", "8H", "8H", "8H", "8H", "8H", "8H", "8D", "8D", "8D", "8D", "8D", "8D", "8C", "8C", "8C", "8C", "8C", "8C", "8S", "8S", "8S", "8S", "8S", "8S", "7H", "7H", "7H", "7H", "7H", "7H", "7D", "7D", "7D", "7D", "7D", "7D", "7C", "7C", "7C", "7C", "7C", "7C", "7S", "7S", "7S", "7S", "7S", "7S", "6H", "6H", "6H", "6H", "6H", "6H", "6D", "6D", "6D", "6D", "6D", "6D", "6C", "6C", "6C", "6C", "6C", "6C", "6S", "6S", "6S", "6S", "6S", "6S", "5H", "5H", "5H", "5H", "5H", "5H", "5D", "5D", "5D", "5D", "5D", "5D", "5C", "5C", "5C", "5C", "5C", "5C", "5S", "5S", "5S", "5S", "5S", "5S", "4H", "4H", "4H", "4H", "4H", "4H", "4D", "4D", "4D", "4D", "4D", "4D", "4C", "4C", "4C", "4C", "4C", "4C", "4S", "4S", "4S", "4S", "4S", "4S", "3H", "3H", "3H", "3H", "3H", "3H", "3D", "3D", "3D", "3D", "3D", "3D", "3C", "3C", "3C", "3C", "3C", "3C", "3S", "3S", "3S", "3S", "3S", "3S", "2H", "2H", "2H", "2H", "2H", "2H", "2D", "2D", "2D", "2D", "2D", "2D", "2C", "2C", "2C", "2C", "2C", "2C", "2S", "2S", "2S", "2S", "2S", "2S"};

    for(i=0; i<SHOE_SIZE; i++) {
        shoe[i] = all_cards[i];
    }
}