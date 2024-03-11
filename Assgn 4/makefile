run: peer.c
	gcc -o peer peer.c
	./peer $(port)

run_correct: peer_correct
	gcc -o peer_correct peer_correct.c
	./peer_correct $(port)

clean:
	rm -f peer peer_correct