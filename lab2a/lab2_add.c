/*
 * NAME: Patrick Shih
 * EMAIL: patrickkshih@gmail.com
 * ID: 604580648
*/

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <getopt.h>
#include <string.h>

int opt_yield = 0;
char opt_sync;
int num_iter = 1;
pthread_mutex_t mutex;

void add(long long *pointer, long long value){
	long long sum = *pointer + value;
	if (opt_yield)
		sched_yield();
	*pointer = sum;
}

void add_c(long long *counter, long long value){
	long long prev;
	long long total = 0;
	do{
		prev = *counter;
		total = prev + value;
		if (opt_yield)
			sched_yield();
	} while(__sync_val_compare_and_swap(counter, prev, total) != prev);
}

void* add_helper(void* counter){
	int spin_lock = 0;
	int i;
	long long prev, sum;
	for (i = 0; i < num_iter; i++){
		switch(opt_sync){
			case 'm':
				pthread_mutex_lock(&mutex);
				add((long long*) counter, 1);
				pthread_mutex_unlock(&mutex);
				break;
			case 'c':
				add_c((long long*)counter, 1);
				break;
			case 's':
				while (__sync_lock_test_and_set(&spin_lock, 1));
				add ((long long*)counter, 1);
				__sync_lock_release(&spin_lock);
				break;
			default:
				add((long long*) counter, 1);
		}
	}

	for (i = 0; i < num_iter; i++){
		switch(opt_sync){
			case 'm':
				pthread_mutex_lock(&mutex);
				add((long long*)counter, -1);
				pthread_mutex_unlock(&mutex);
				break;
			case 'c':
				add_c((long long*) counter, -1);
				break;
			case 's':
				while (__sync_lock_test_and_set(&spin_lock, 1));
				add ((long long*) counter, -1);
				__sync_lock_release(&spin_lock);
				break;
			default:
				add((long long*)counter, -1);
		}
	}
	return NULL;
}

int main(int argc, char** argv){
	int num_thrd = 1;
	long long total = 0;
	int rc;

	static struct option long_opts[] = {
		{"threads", optional_argument, 0, 't'},
		{"yield", no_argument, 0, 'y'},
		{"sync", required_argument, 0, 's'},
		{"iterations", optional_argument, 0, 'i'}
	};

	int opt;
	while((opt = getopt_long(argc, argv, "a:b:", long_opts, NULL)) != -1){
		switch(opt){
			case 't':
				num_thrd = atoi(optarg);
				break;
			case 'y':
				opt_yield = 1;
				break;
			case 's':
				if (strlen(optarg) != 1 || !(optarg[0] == 'm' || optarg[0] == 'c' || optarg[0] == 's')){
					fprintf(stderr, "Invalid argument: sync=[mcs]\n");
					exit(1);
				}
				opt_sync = *optarg;
				if (opt_sync == 'm')
					pthread_mutex_init(&mutex, NULL);
				break;
			case 'i':
				num_iter = atoi(optarg);
				break;
			default:
				fprintf(stderr, "Invalid argument: ./lab2_add [--iterations=#][--threads=#][--yield][--sync]\n");
				exit(1);
		}
	}
	pthread_t *threads = malloc(num_thrd * sizeof(pthread_t));
	struct timespec start, stop;

	rc = clock_gettime(CLOCK_MONOTONIC, &start);
	if (rc == -1){
		fprintf(stderr, "Starting clock failure\n");
	}

	int i;
	for (i = 0; i < num_thrd; i++){
		if (pthread_create(threads + i, NULL, add_helper, &total)){
			fprintf(stderr, "Failure to add thread \n");
		}
	}

	for (i = 0; i < num_thrd; i++){
		if (pthread_join(*(threads+i), NULL)){
			fprintf(stderr, "Failure to join thread\n");
		}
	}

	rc = clock_gettime(CLOCK_MONOTONIC, &stop);
	if (rc == -1){
		fprintf(stderr, "Stopping clock failure\n");
	}

	long total_time = 1000000000 * (stop.tv_sec - start.tv_sec) + (stop.tv_nsec - start.tv_nsec);
	int total_ops = num_thrd * num_iter * 2;
	long avg_time = total_time/total_ops;

	char str[20];
	strcpy(str,"add-");
	if (opt_yield)
		strcat(str,"yield-");

	switch(opt_sync){
		case 'm':
			strcat(str,"m");
			break;
		case 's':
			strcat(str,"s");
			break;
		case 'c':
			strcat(str,"c");
			break;
		default:
			strcat(str,"none");
	}

	fprintf(stdout,"%s,%d,%d,%d,%ld,%ld,%d\n", str, num_thrd, num_iter, total_ops, total_time, avg_time,total);

	exit(0);
}
