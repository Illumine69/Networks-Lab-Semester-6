run: user1.c user2.c
	gcc -Wall -o user1 user1.c
	gcc -Wall -o user2 user2.c
	./user1
	./user2