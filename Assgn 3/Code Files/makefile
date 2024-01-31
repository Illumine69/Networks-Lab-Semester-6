home: mailclient.c
	gcc -Wall mailclient.c -o mailclient
	./mailclient 10.147.225.68 24000 1221

runclient: mailclient.c
	gcc -Wall mailclient.c -o mailclient
	./mailclient 10.145.118.146 24000 1221

runserver: smtpmail.c
	gcc -Wall smtpmail.c -o smtpmail
	./smtpmail 24000

clean:
	rm mailclient smtpmail
