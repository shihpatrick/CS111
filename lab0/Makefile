default: 
	gcc -o lab0 -g lab0.c

check: 	checkTest checkValidArg checkSegFault checkOutput checkInput checkIOtest
	if [[ -s logerr.txt ]]; then \
	echo "Tests failed - check logerr.txt"; \
	fi;

checkTest:
	echo "check test" >> input.txt; \
	./lab0 --input=input.txt --output=output.txt; \
	if [[ $$? -ne 0 ]]; then \
	echo "input/output streams not valid" >> logerr.txt; \
	fi;
	
checkValidArg:
	./lab0 --badArg; \
	if [[ $$? -ne 1 ]]; then \
	echo "Bad argument not recognized" >> logerr.txt; \
	fi;

checkSegFault:
	./lab0 --segfault --catch; \
	if [[ $$? -ne 4 ]]; then \
	echo "Segmentation Fault not recognized" >> logerr.txt; \
	fi;

checkOutput:
	touch nonexist.txt; \
	chmod u-w nonexist.txt; \
	./lab0 --output=nonexist.txt; \
	if [[ $$? -ne 3 ]]; then \
	echo "Bad output not recognized" >> logerr.txt; \
	fi;	

checkInput:
	./lab0 --input=input_nonexist.txt --output=output.txt; \
	if [[ $$? -ne 2 ]]; then \
	echo "Bad input not recognized" >> logerr.txt; \
	fi;

checkIOtest:
	echo "checking successful copy" >> input.txt; \
	./lab0 --input=input.txt --output=output.txt; \
	diff input.txt output.txt > check.txt; \
	if [[ -s check.txt ]]; then \
	echo "File failed to copy correctly" >> logerr.txt; \
	fi;
	
clean: 
	rm -f *.tar.gz lab0 *.txt
	
dist: 
	tar -cvf lab0-604580648.tar.gz *.png lab0.c Makefile README
