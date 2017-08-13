default: lab2_list.o SortedList.o 
	gcc -pthread -g -o lab2_list SortedList.c lab2_list.c
	gcc -o profile.out -pthread -g -L /u/eng/class/classkih/lib/ -lprofiler lab2_list.o SortedList.o

profile: lab2_list Makefile
	-rm -f ./raw.gperf
	LD_PRELOAD=/u/eng/class/classkih/lib/libprofiler.so.0 CPUPROFILE=./raw.gperf ./lab2_list --threads=12 --iterations=1000 --sync=s
	pprof --text ./lab2_list ./raw.gperf > profile.out
	pprof --list=threadFunc ./lab2_list ./raw.gperf >> profile.out
	rm -f ./raw.gperf

clean:
	rm -f lab2_list *.png *.csv lab2b-604580648.tar.gz

dist:
	tar -cvf lab2b-604580648.tar.gz lab2_list.c SortedList.h SortedList.c Makefile README *.out *.png *.csv

graphs:
	gnuplot ./lab2_list.gp

tests:
	@./lab2_list --threads=1  --iterations=1000 --sync=m >> lab2_list.csv
	@./lab2_list --threads=2  --iterations=1000 --sync=m >> lab2_list.csv
	@./lab2_list --threads=4  --iterations=1000 --sync=m >> lab2_list.csv
	@./lab2_list --threads=8  --iterations=1000 --sync=m >> lab2_list.csv
	@./lab2_list --threads=12 --iterations=1000 --sync=m >> lab2_list.csv
	@./lab2_list --threads=16 --iterations=1000 --sync=m >> lab2_list.csv
	@./lab2_list --threads=24 --iterations=1000 --sync=m >> lab2_list.csv
	@./lab2_list --threads=1  --iterations=1000 --sync=s >> lab2_list.csv
	@./lab2_list --threads=2  --iterations=1000 --sync=s >> lab2_list.csv
	@./lab2_list --threads=4  --iterations=1000 --sync=s >> lab2_list.csv
	@./lab2_list --threads=8  --iterations=1000 --sync=s >> lab2_list.csv
	@./lab2_list --threads=12 --iterations=1000 --sync=s >> lab2_list.csv
	@./lab2_list --threads=16 --iterations=1000 --sync=s >> lab2_list.csv
	@./lab2_list --threads=24 --iterations=1000 --sync=s >> lab2_list.csv

	@-./lab2_list --lists=4 --threads=1  --iterations=1   --yield=id >> lab2_list.csv
	@-./lab2_list --lists=4 --threads=1  --iterations=2   --yield=id >> lab2_list.csv
	@-./lab2_list --lists=4 --threads=1  --iterations=4   --yield=id >> lab2_list.csv
	@-./lab2_list --lists=4 --threads=1  --iterations=8   --yield=id >> lab2_list.csv
	@-./lab2_list --lists=4 --threads=1  --iterations=16   --yield=id >> lab2_list.csv

	@-./lab2_list --lists=4 --threads=4  --iterations=1   --yield=id >> lab2_list.csv
	@-./lab2_list --lists=4 --threads=4  --iterations=2   --yield=id >> lab2_list.csv
	@-./lab2_list --lists=4 --threads=4  --iterations=4   --yield=id >> lab2_list.csv
	@-./lab2_list --lists=4 --threads=4  --iterations=8   --yield=id >> lab2_list.csv
	@-./lab2_list --lists=4 --threads=4  --iterations=16   --yield=id >> lab2_list.csv

	@-./lab2_list --lists=4 --threads=8  --iterations=1   --yield=id >> lab2_list.csv
	@-./lab2_list --lists=4 --threads=8  --iterations=2   --yield=id >> lab2_list.csv
	@-./lab2_list --lists=4 --threads=8  --iterations=4   --yield=id >> lab2_list.csv
	@-./lab2_list --lists=4 --threads=8  --iterations=8   --yield=id >> lab2_list.csv
	@-./lab2_list --lists=4 --threads=8  --iterations=16   --yield=id >> lab2_list.csv

	@-./lab2_list --lists=4 --threads=12  --iterations=1   --yield=id >> lab2_list.csv
	@-./lab2_list --lists=4 --threads=12  --iterations=2   --yield=id >> lab2_list.csv
	@-./lab2_list --lists=4 --threads=12  --iterations=4   --yield=id >> lab2_list.csv
	@-./lab2_list --lists=4 --threads=12  --iterations=8   --yield=id >> lab2_list.csv
	@-./lab2_list --lists=4 --threads=12  --iterations=16   --yield=id >> lab2_list.csv

	@-./lab2_list --lists=4 --threads=16  --iterations=1   --yield=id >> lab2_list.csv
	@-./lab2_list --lists=4 --threads=16  --iterations=2   --yield=id >> lab2_list.csv
	@-./lab2_list --lists=4 --threads=16  --iterations=4   --yield=id >> lab2_list.csv
	@-./lab2_list --lists=4 --threads=16  --iterations=8   --yield=id >> lab2_list.csv
	@-./lab2_list --lists=4 --threads=16  --iterations=16   --yield=id >> lab2_list.csv

	@-./lab2_list --lists=4 --threads=1  --iterations=10   --sync=s --yield=id >> lab2_list.csv
	@-./lab2_list --lists=4 --threads=1  --iterations=20   --sync=s --yield=id >> lab2_list.csv
	@-./lab2_list --lists=4 --threads=1  --iterations=40   --sync=s --yield=id >> lab2_list.csv
	@-./lab2_list --lists=4 --threads=1  --iterations=80   --sync=s --yield=id >> lab2_list.csv

	@-./lab2_list --lists=4 --threads=4  --iterations=10   --sync=s --yield=id >> lab2_list.csv
	@-./lab2_list --lists=4 --threads=4  --iterations=20   --sync=s --yield=id >> lab2_list.csv
	@-./lab2_list --lists=4 --threads=4  --iterations=40   --sync=s --yield=id >> lab2_list.csv
	@-./lab2_list --lists=4 --threads=4  --iterations=80   --sync=s --yield=id >> lab2_list.csv

	@-./lab2_list --lists=4 --threads=8  --iterations=10   --sync=s --yield=id >> lab2_list.csv
	@-./lab2_list --lists=4 --threads=8  --iterations=20   --sync=s --yield=id >> lab2_list.csv
	@-./lab2_list --lists=4 --threads=8  --iterations=40   --sync=s --yield=id >> lab2_list.csv
	@-./lab2_list --lists=4 --threads=8  --iterations=80   --sync=s --yield=id >> lab2_list.csv

	@-./lab2_list --lists=4 --threads=12  --iterations=10   --sync=s --yield=id >> lab2_list.csv
	@-./lab2_list --lists=4 --threads=12  --iterations=20   --sync=s --yield=id >> lab2_list.csv
	@-./lab2_list --lists=4 --threads=12  --iterations=40   --sync=s --yield=id >> lab2_list.csv
	@-./lab2_list --lists=4 --threads=12  --iterations=80   --sync=s --yield=id >> lab2_list.csv

	@-./lab2_list --lists=4 --threads=16  --iterations=10   --sync=s --yield=id >> lab2_list.csv
	@-./lab2_list --lists=4 --threads=16  --iterations=20   --sync=s --yield=id >> lab2_list.csv
	@-./lab2_list --lists=4 --threads=16  --iterations=40   --sync=s --yield=id >> lab2_list.csv
	@-./lab2_list --lists=4 --threads=16  --iterations=80   --sync=s --yield=id >> lab2_list.csv

	@-./lab2_list --lists=4 --threads=1  --iterations=10   --sync=m --yield=id >> lab2_list.csv
	@-./lab2_list --lists=4 --threads=1  --iterations=20   --sync=m --yield=id >> lab2_list.csv
	@-./lab2_list --lists=4 --threads=1  --iterations=40   --sync=m --yield=id >> lab2_list.csv
	@-./lab2_list --lists=4 --threads=1  --iterations=80   --sync=m --yield=id >> lab2_list.csv

	@-./lab2_list --lists=4 --threads=4  --iterations=10   --sync=m --yield=id >> lab2_list.csv
	@-./lab2_list --lists=4 --threads=4  --iterations=20   --sync=m --yield=id >> lab2_list.csv
	@-./lab2_list --lists=4 --threads=4  --iterations=40   --sync=m --yield=id >> lab2_list.csv
	@-./lab2_list --lists=4 --threads=4  --iterations=80   --sync=m --yield=id >> lab2_list.csv

	@-./lab2_list --lists=4 --threads=8  --iterations=10   --sync=m --yield=id >> lab2_list.csv
	@-./lab2_list --lists=4 --threads=8  --iterations=20   --sync=m --yield=id >> lab2_list.csv
	@-./lab2_list --lists=4 --threads=8  --iterations=40   --sync=m --yield=id >> lab2_list.csv
	@-./lab2_list --lists=4 --threads=8  --iterations=80   --sync=m --yield=id >> lab2_list.csv

	@-./lab2_list --lists=4 --threads=12  --iterations=10   --sync=m --yield=id >> lab2_list.csv
	@-./lab2_list --lists=4 --threads=12  --iterations=20   --sync=m --yield=id >> lab2_list.csv
	@-./lab2_list --lists=4 --threads=12  --iterations=40   --sync=m --yield=id >> lab2_list.csv
	@-./lab2_list --lists=4 --threads=12  --iterations=80   --sync=m --yield=id >> lab2_list.csv

	@-./lab2_list --lists=4 --threads=16  --iterations=10   --sync=m --yield=id >> lab2_list.csv
	@-./lab2_list --lists=4 --threads=16  --iterations=20   --sync=m --yield=id >> lab2_list.csv
	@-./lab2_list --lists=4 --threads=16  --iterations=40   --sync=m --yield=id >> lab2_list.csv
	@-./lab2_list --lists=4 --threads=16  --iterations=80   --sync=m --yield=id >> lab2_list.csv

	@-./lab2_list --lists=4 --threads=1  --iterations=1000 --sync=m >> lab2_list.csv
	@-./lab2_list --lists=4 --threads=2  --iterations=1000 --sync=m >> lab2_list.csv
	@-./lab2_list --lists=4 --threads=4  --iterations=1000 --sync=m >> lab2_list.csv
	@-./lab2_list --lists=4 --threads=8  --iterations=1000 --sync=m >> lab2_list.csv
	@-./lab2_list --lists=4 --threads=12 --iterations=1000 --sync=m >> lab2_list.csv

	@-./lab2_list --lists=8 --threads=1  --iterations=1000 --sync=m >> lab2_list.csv
	@-./lab2_list --lists=8 --threads=2  --iterations=1000 --sync=m >> lab2_list.csv
	@-./lab2_list --lists=8 --threads=4  --iterations=1000 --sync=m >> lab2_list.csv
	@-./lab2_list --lists=8 --threads=8  --iterations=1000 --sync=m >> lab2_list.csv
	@-./lab2_list --lists=8 --threads=12 --iterations=1000 --sync=m >> lab2_list.csv

	@-./lab2_list --lists=16 --threads=1  --iterations=1000 --sync=m >> lab2_list.csv
	@-./lab2_list --lists=16 --threads=2  --iterations=1000 --sync=m >> lab2_list.csv
	@-./lab2_list --lists=16 --threads=4  --iterations=1000 --sync=m >> lab2_list.csv
	@-./lab2_list --lists=16 --threads=8  --iterations=1000 --sync=m >> lab2_list.csv
	@-./lab2_list --lists=16 --threads=12 --iterations=1000 --sync=m >> lab2_list.csv

	@-./lab2_list --lists=4 --threads=1  --iterations=1000 --sync=s >> lab2_list.csv
	@-./lab2_list --lists=4 --threads=2  --iterations=1000 --sync=s >> lab2_list.csv
	@-./lab2_list --lists=4 --threads=4  --iterations=1000 --sync=s >> lab2_list.csv
	@-./lab2_list --lists=4 --threads=8  --iterations=1000 --sync=s >> lab2_list.csv
	@-./lab2_list --lists=4 --threads=12 --iterations=1000 --sync=s >> lab2_list.csv

	@-./lab2_list --lists=8 --threads=1  --iterations=1000 --sync=s >> lab2_list.csv
	@-./lab2_list --lists=8 --threads=2  --iterations=1000 --sync=s >> lab2_list.csv
	@-./lab2_list --lists=8 --threads=4  --iterations=1000 --sync=s >> lab2_list.csv
	@-./lab2_list --lists=8 --threads=8  --iterations=1000 --sync=s >> lab2_list.csv
	@-./lab2_list --lists=8 --threads=12 --iterations=1000 --sync=s >> lab2_list.csv

	@-./lab2_list --lists=16 --threads=1  --iterations=1000 --sync=s >> lab2_list.csv
	@-./lab2_list --lists=16 --threads=2  --iterations=1000 --sync=s >> lab2_list.csv
	@-./lab2_list --lists=16 --threads=4  --iterations=1000 --sync=s >> lab2_list.csv
	@-./lab2_list --lists=16 --threads=8  --iterations=1000 --sync=s >> lab2_list.csv
	@-./lab2_list --lists=16 --threads=12 --iterations=1000 --sync=s >> lab2_list.csv


