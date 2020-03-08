Name: Daniel Raz 

to activate program: open linux terminal, navigate to ex2 executeable file location using "cd" command (confirm it using ls command) and type 
valgrind ./client client [-p ] [-r n < pr1=value1 pr2=value2 …>] when -p and -r are optional.


list of submitted files:
client.c : Implementation of the client side with the communication with the server.


private function:
checkRParam() - this function for check if the arguments after -r flag are valid.

validNumOfR() - this function check if a number shows after -r flag, return -1 if not.

indexOf() - return the index where the char 'c' showing or -1 if not exist.
