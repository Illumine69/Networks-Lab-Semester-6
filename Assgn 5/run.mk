run: msocket.h
	gcc -Wall -o user1 -I. -L.  user1.c -lmsocket
	gcc -Wall -o user2 -I. -L. user2.c -lmsocket

clean:
	rm -f user1 user2 *.o *.a init