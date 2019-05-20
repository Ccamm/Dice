#!/usr/bin/env python3
"""
This is a simple example of a client program written in Python.
Again, this is a very basic example to complement the 'basic_server.c' example.


When testing, start by initiating a connection with the server by sending the "init" message outlined in
the specification document. Then, wait for the server to send you a message saying the game has begun.

Once this message has been read, plan out a couple of turns on paper and hard-code these messages to
and from the server (i.e. play a few rounds of the 'dice game' where you know what the right and wrong
dice rolls are). You will be able to edit this trivially later on; it is often easier to debug the code
if you know exactly what your expected values are.

From this, you should be able to bootstrap message-parsing to and from the server whilst making it easy to debug.
Then, start to add functions in the server code that actually 'run' the game in the background.
"""

import socket
import sys
from time import sleep


moves_dict = {"EVEN" : "EVEN", "ODD" : "ODD", "DOUBLE" : "DOUB", "CONTAINS" : "CON"}

def parse_move(move, p_id):
    try:
        movelist = move.split()
        movemsg = moves_dict[movelist[0]]
        if(movemsg == moves_dict["CONTAINS"] and len(movelist) == 2):
            movemsg = movemsg + "," + movelist[1]
        elif(movemsg == moves_dict["CONTAINS"]):
            raise KeyError
        return str(p_id)+",MOV,"+movemsg
    except KeyError:
        print(move + " is an invalid move.")
        move_redo = input("Redo your move again!\n")
        return parse_move(move_redo, p_id)


# Create a TCP/IP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Connect the socket to the port where the server is listening
server_str = input("input the server address\nuse the format [IP]:[PORT]\n").split(':')
try:
    server_address = (server_str[0], int(server_str[1]))
    print ('connecting to %s port %s' % server_address)
    sock.connect(server_address)
except:
    print(server_str + " is not a valid string!\nExiting now.")
    exit()

count=0
message = 'INIT'.encode()
sock.sendall(message)
player_id = 0
try:
    while True:

        exit = False

        # Look for the response
        amount_received = 0
        amount_expected = len(message)

        while amount_received < amount_expected:
            data = sock.recv(1024)
            amount_received += len(data)
            mess = data.decode()
            print("Received " + mess)
            if "WELCOME" in mess:
                print("The games have begun")
                player_id = int(mess.split(',')[-1])
            move = input("What is your prediction of the dice roll?\nWill it be EVEN, ODD, DOUBLE or CONTAINS [Number 1-6]?\n")
            mess = parse_move(move,player_id)
            print("Sent " + mess)
            sock.sendall(mess.encode()) # Client has ID 231
        if exit:
            break
finally:
    print ('closing socket')
    sock.close()
