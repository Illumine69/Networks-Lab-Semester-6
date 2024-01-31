home:
	gcc -Wall mailclient.c -o mailclient
	./mailclient 10.147.225.68 24000 1221



mailclient: mailclient.c
	gcc -Wall mailclient.c -o mailclient
	./mailclient 10.147.225.68 24000 1221

smtpmail: smtpmail.c
	gcc -Wall smtpmail.c -o smtpmail
	./smtpmail 24000

clean:
	rm mailclient smtpmail
