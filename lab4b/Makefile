default:
	gcc -std=gnu11 -O2 -lm -lmraa -pthread -o lab4b lab4b.c

dist:
	tar -cvf lab4b-604580648.tar.gz Makefile lab4b.c README Makefile check.sh

check:
	chmod u+x check.sh
	@-./check.sh

clean:
	rm -f lab4b lab4b-604580648.tar.gz

