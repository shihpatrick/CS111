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
#include "SortedList.h"

SortedList_t *slist;
SortedListElement_t* s_nodes;
pthread_mutex_t mutex;

int opt_yield = 0;
char opt_sync;
int num_iter = 1;
int num_thrd = 1;
int total_ops = 0;

void* thread_func(void* tid){
	int spin_lock = 0;
	int i;
	for (i = *(int*)tid ; i < total_ops; i+=num_thrd){
		switch(opt_sync){
			case 'm':
				pthread_mutex_lock(&mutex);
				SortedList_insert(slist, &s_nodes[i]);
				pthread_mutex_unlock(&mutex);
				break;
			case 's':
				while (__sync_lock_test_and_set(&spin_lock, 1));
				SortedList_insert(slist, &s_nodes[i]);
				__sync_lock_release(&spin_lock);
				break;
			default:
				SortedList_insert(slist, &s_nodes[i]);
		}
	}

	switch(opt_sync){
		case 'm':
			pthread_mutex_lock(&mutex);
			SortedList_length(slist);
			pthread_mutex_unlock(&mutex);
			break;
		case 's':
			while (__sync_lock_test_and_set(&spin_lock, 1));
			SortedList_length(slist);
			__sync_lock_release(&spin_lock);
			break;
		default:
			SortedList_length(slist);
	}

	SortedListElement_t *node;
	int j;
	for (j = *(int*)tid ; j < total_ops; j+=num_thrd){
		switch(opt_sync){
			case 'm':
				pthread_mutex_lock(&mutex);
				node = SortedList_lookup(slist, s_nodes[j].key);
				SortedList_delete(node);
				pthread_mutex_unlock(&mutex);
				break;
			case 's':
				while (__sync_lock_test_and_set(&spin_lock, 1));
				node = SortedList_lookup (slist, s_nodes[j].key);
				SortedList_delete(node);
				__sync_lock_release(&spin_lock);
				break;
			default:
				node = SortedList_lookup(slist, s_nodes[j].key);
				SortedList_delete(node);
		}
	}
	return NULL;
}

int main(int argc, char** argv){
	int i;
	static struct option long_opts[] = {
		{"threads", optional_argument, 0, 't'},
		{"yield", required_argument, 0, 'y'},
		{"sync", required_argument, 0, 's'},
		{"iterations", optional_argument, 0, 'i'}
	};
	int opt;
	char *ch;
	char str[20];
	int yield_flag = 0;
	strcpy(str,"list-");
		
	while((opt = getopt_long(argc, argv, "a:b:", long_opts, NULL)) != -1){
		switch(opt){
			case 't':
				num_thrd = atoi(optarg);
				break;
			case 'y':
				ch = optarg;
				yield_flag = 1;	
				while(*ch != '\0'){
					if(*ch == 'l'){
						strcat(str,"l");	
						opt_yield |= LOOKUP_YIELD;
					}
					else if (*ch == 'd'){
						strcat(str,"d");
						opt_yield |= DELETE_YIELD;
					}
					else if (*ch == 'i'){
						strcat(str,"i");
						opt_yield |= INSERT_YIELD;
					}
					else{
						fprintf(stderr, "Invalid argument: yield=[ldi]\n");
						exit(1);
					}
					ch=ch+1;
				}			
				break;
			case 's':
				if (strlen(optarg) != 1 || !(optarg[0] == 'm' || optarg[0] == 's')){
					fprintf(stderr, "Invalid argument: sync=[ms]\n");
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

	total_ops = num_thrd * num_iter;
	//Init List
	slist = malloc(sizeof(SortedList_t));
	slist->key = NULL;
	slist->next = slist;
	slist->prev = slist;

	//generate keys
	s_nodes = malloc(total_ops*sizeof(SortedListElement_t));
	static const char alpha[] = "ABCDEFGHIJKLMNQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	int j;
	srand(time(NULL));
	for (i = 0; i < total_ops; i++){
		char* key  = malloc(6* sizeof(char));
		for (j = 0; j < 5; j++)
			key[j] = alpha[rand() % (int) (sizeof alpha -1)];
		key[5] = '\0';
		s_nodes[i].key = key;
	}

	pthread_t *threads = malloc(num_thrd * sizeof(pthread_t));
	struct timespec start, stop;

	clock_gettime(CLOCK_MONOTONIC, &start);
	int retvals[num_thrd];
	for (i = 0; i < num_thrd; i++){
		retvals[i]=i;
		if (pthread_create(threads + i, NULL, thread_func, &retvals[i])){
			fprintf(stderr, "Failure to add thread \n");
			exit(1);
		}
	}

	for (i = 0; i < num_thrd; i++){
		if (pthread_join(*(threads+i), NULL)){
			fprintf(stderr, "Failure to join thread\n");
			exit(1);
		}
	}

	clock_gettime(CLOCK_MONOTONIC, &stop);

	long total_time = 1000000000 * (stop.tv_sec - start.tv_sec) + (stop.tv_nsec - start.tv_nsec);
	total_ops = total_ops * 3;
	long avg_time = total_time/total_ops;
	
	if (!yield_flag)
		strcat(str,"none");

	switch(opt_sync){
		case 'm':
			strcat(str,"-m");
			break;
		case 's':
			strcat(str,"-s");
			break;
		default:
			strcat(str,"-none");
	}

	fprintf(stdout,"%s,%d,%d,1,%d,%ld,%ld\n",str,num_thrd, num_iter, total_ops, total_time, avg_time);

	exit(0);
}
