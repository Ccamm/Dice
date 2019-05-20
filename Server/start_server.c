/**
* Based on code found at https://github.com/mafintosh/echo-servers.c (Copyright (c) 2014 Mathias Buus)
* Copyright 2019 Nicholas Pritchard, Ryan Bunney
**/

/**
 * @brief A simple example of a network server written in C implementing a basic echo
 *
 * This is a good starting point for observing C-based network code but is by no means complete.
 * We encourage you to use this as a starting point for your project if you're not sure where to start.
 * Food for thought:
 *   - Can we wrap the action of sending ALL of out data and receiving ALL of the data?
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

typedef enum { CONNECT, MOVE, NONE } MSG_TYPE;
typedef enum { EVEN, ODD, DOUB, CON } MOVE_TYPE;

#define CONNECT_MSG "INIT"
#define MOVE_MSG "MOV"

#define EVEN_MSG "EVEN"
#define ODD_MSG "ODD"
#define DOUB_MSG "DOUB"
#define CON_MSG "CON"

#define WELCOME "WELCOME,%d"
#define REJECT "REJECT"
#define START "START,%d,%d"
#define CANCEL "CANCEL"
#define PASS "%d,PASS"
#define FAIL "%d,FAIL"
#define ELIM "%d,ELIM"
#define VICT "%d,VICT"

#define INITIAL_LIVES 5
#define NUM_OF_DICE 2
#define SIDES_OF_DIE 6

typedef struct player {
  int id;
  int lives;
  MSG_TYPE msgtype;
  MOVE_TYPE move;
  int containsint;
} PLAYER;

//Assigns id to players
int cid = 0;
/** TO BE DELETED TO SUPPORT MULTIPLE PLAYERS!
* Stores data of player that is connected
**/
PLAYER p;
int die[NUM_OF_DICE];
#define BUFFER_SIZE 1024

void teardown_game( void ) {

}

void setup_game( void ) {

}

void play_game_round( void ) {
  for(int i = 0; i < NUM_OF_DICE; i++) {
    die[i] = rand()%SIDES_OF_DIE+1;
  }
}

int send_message( void ) {
  char *buf = calloc(BUFFER_SIZE, sizeof(char));
  int err;
  if(p.msgtype == CONNECT) {
    buf[0] = '\0';
    sprintf(buf, WELCOME, p.id);
    err = send(p.id, buf, strlen(buf), 0);
    if (err < 0){
        fprintf(stderr,"Client write failed\n");
        exit(EXIT_FAILURE);
    }
    p.lives = INITIAL_LIVES;
    return 0;
  } else {
    play_game_round();
    int total = 0;
    for(int i=0;i < NUM_OF_DICE; i++) {
      total += die[i];
    }
    switch (p.move) {
      case EVEN:
        buf[0] = '\0';
        if(total % 2 == 0) {
          sprintf(buf, PASS, p.id);
        } else {
          sprintf(buf, FAIL, p.id);
	  p.lives = p.lives - 1;
        }
        break;

      case ODD:
        buf[0] = '\0';
        if(total % 2 == 1) {
          sprintf(buf, PASS, p.id);
        } else {
          sprintf(buf, FAIL, p.id);
	  p.lives = p.lives - 1;
        }

      case DOUB:
        if(die[0] == die[1]) {
          sprintf(buf, PASS, p.id);
        } else {
          sprintf(buf, FAIL, p.id);
	  p.lives = p.lives - 1;
        }

      case CON:
        if(die[0] == p.containsint || die[1] == p.containsint) {
          sprintf(buf, PASS, p.id);
        } else {
          sprintf(buf, FAIL, p.id);
	  p.lives = p.lives - 1;
        }
    }
    if(p.lives <= 0){
	buf = calloc(BUFFER_SIZE, sizeof(char));
	sprintf(buf,ELIM,p.id);
	err = send(p.id, buf, strlen(buf), 0);
    } else {
    	err = send(p.id, buf, strlen(buf), 0);
    }
    if (err < 0){
        fprintf(stderr,"Client write failed\n");
        exit(EXIT_FAILURE);
    }
    free(buf);
    return 0;
  }
  return -1;
}

int parse_message(char *msg) {
  MSG_TYPE msg_type = NONE;
  if(strstr(msg, CONNECT_MSG) != NULL) {
    msg_type = CONNECT;
  } else if(strstr(msg, MOVE_MSG) != NULL) {
    msg_type = MOVE;
  }

  if(msg_type != NONE) {
    p.msgtype = msg_type;

    if(msg_type == CONNECT) {
      p.lives = INITIAL_LIVES;
    }

    if(msg_type == MOVE) {
      MOVE_TYPE move_type;
      if(strstr(msg, EVEN_MSG) != NULL) {
        move_type = EVEN;
      } else if(strstr(msg, ODD_MSG) != NULL) {
        move_type = ODD;
      } else if(strstr(msg, DOUB_MSG) != NULL) {
        move_type = DOUB;
      } else if(strstr(msg, CON_MSG) != NULL) {
        move_type = CON;
        char *str_contains_int = msg + strlen(msg);
        while(*str_contains_int != ',' && str_contains_int-- != msg);
        p.containsint = atoi(++str_contains_int);
      }
      p.move = move_type;
    }
    return send_message();
  }
  return -1;
}

int main (int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr,"Usage: %s [port]\n",argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);

    int server_fd, client_fd, err, opt_val;
    struct sockaddr_in server, client;
    char *buf;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0){
        fprintf(stderr,"Could not create socket\n");
        exit(EXIT_FAILURE);
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    opt_val = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof opt_val);

    err = bind(server_fd, (struct sockaddr *) &server, sizeof(server));
    if (err < 0){
        fprintf(stderr,"Could not bind socket\n");
        exit(EXIT_FAILURE);
    }

    err = listen(server_fd, 128);
    if (err < 0){
        fprintf(stderr,"Could not listen on socket\n");
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on %d\n", port);

    while (true) {
        socklen_t client_len = sizeof(client);
        // Will block until a connection is made
        client_fd = accept(server_fd, (struct sockaddr *) &client, &client_len);

        if (client_fd < 0) {
            fprintf(stderr,"Could not establish new connection\n");
            exit(EXIT_FAILURE);
        }
        p.id = client_fd;

        /**
        The following while loop contains some basic code that sends messages back and forth
        between a client (e.g. the socket_client.py client).

        The majority of the server 'action' will happen in this part of the code (unless you decide
        to change this dramatically, which is allowed). The following function names/definitions will
        hopefully provide inspiration for how you can start to build up the functionality of the server.

        parse_message(char *){...} :
            * This would be a good 'general' function that reads a message from a client and then
            determines what the required response is; is the client connecting, making a move, etc.
            * It may be useful having an enum that is used to track what type of client message is received
            (e.g. CONNECT/MOVE etc.)

        send_message() {...}:
            * This would send responses based on what the client has sent through, or if the server needs
            to send all clients messages

        play_game_round() {...}: Implements the functionality for a round of the game
            * 'Roll' the dice (using a random number generator) and then check if the move made by the user
            is correct
            * update game state depending on success or failure.

        setup_game/teardown_game() {} :
            * this will set up the initial state of the game (number of rounds, players
            etc.)/ print out final game results and cancel socket connections.

        Accepting multiple connections (we recommend not starting this until after implementing some
        of the basic message parsing/game playing):
            * Whilst in a while loop
                - Accept a new connection
                - Create a child process
                - In the child process (which is associated with client), perform game_playing functionality
                (or read the messages)
        **/

        while (true) {
            buf = calloc(BUFFER_SIZE, sizeof(char)); // Clear our buffer so we don't accidentally send/print garbage
            int read = recv(client_fd, buf, BUFFER_SIZE, 0);    // Try to read from the incoming client

            if (read < 0){
                fprintf(stderr,"Client read failed\n");
                exit(EXIT_FAILURE);
            }
            err = parse_message(buf);
            if( err < 0 ) {
              fprintf(stderr, "Failed to send message\n");
              exit(EXIT_FAILURE);
            }
            free(buf);
        }

    }
}
