default: tls_server
	gcc -o lab4c_tcp lab4c_tcp.c -lssl -lmraa -lm -pthread

tls_server:
	gcc -o lab4c_tls lab4c_tls.c -lssl -lmraa -lm -pthread -lcrypto

clean:
	rm -f lab4c-604580648.tar.gz lab4c_tcp lab4c_tls

dist:
	tar -cvf lab4c-604580648.tar.gz lab4c_tls.c lab4c_tcp.c Makefile README 
