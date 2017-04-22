build:
	gcc -g -o bin/recv recv.c nsocket.c -I.
	gcc -g -o bin/send send.c nsocket.c -I.
