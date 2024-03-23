run: msocket.h
	gcc -Wall -o user1 -I. -L.  user1.c -lmsocket
	gcc -Wall -o user2 -I. -L. user2.c -lmsocket
	gcc -Wall -o user4 -I. -L. user4.c -lmsocket

everything: clean
	make -f libmsocket.mk library
	make -f initmsocket.mk init
	make -f run.mk run

clean:
	rm -f *.o *.a init user* a.out
	./a.sh