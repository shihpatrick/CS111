#NAME: Patrick Shih
#Email: patrickkshih@gmail.com
#ID: 604580648

default: client server

client:
	gcc -o lab1b-client lab1b-client.c -lmcrypt

server:
	gcc -o lab1b-server lab1b-server.c -lmcrypt

clean: 
	rm -f *.tar.gz lab1b-client lab1b-server *.txt

dist:
	tar -cvf lab1b-604580648.tar.gz lab1b-server.c lab1b-client.c README Makefile my.key
