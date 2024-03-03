home: mailclient.c
	gcc -Wall mailclient.c -o mailclient
	./mailclient 10.147.156.120 24000 32000

runclient: mailclient.c
	gcc -Wall mailclient.c -o mailclient
	./mailclient 10.145.51.179 24000 20000

runserver: smtpmail.c
	gcc -Wall smtpmail.c -o smtpmail
	./smtpmail 24000

runpop: popserver.c
	gcc -Wall popserver.c -o popserver
	./popserver 32000

clean:
	rm mailclient smtpmail
