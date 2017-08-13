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

SortedList_t *slist_arr;
SortedListElement_t* s_nodes;
int* hash_key;
pthread_mutex_t* mutex_arr;
int* spin_lock_arr;

int opt_yield = 0;
char opt_sync;
int num_iter = 1;
int num_thrd = 1;
int total_ops = 0;
int num_list = 1;
long* time_arr;

void* thread_func(void* tid){
	int spin_lock = 0;
	int i;
    struct timespec iStart, iStop;
    struct timespec lStart, lStop;
    struct timespec dStart, dStop;
    long time = 0;
	for (i = *(int*)tid ; i < total_ops; i+=num_thrd){
		switch(opt_sync){
			case 'm':
				clock_gettime(CLOCK_MONOTONIC, &iStart);
				pthread_mutex_lock(&mutex_arr[hash_key[i]]);
                clock_gettime(CLOCK_MONOTONIC, &iStop);
				SortedList_insert(&slist_arr[hash_key[i]], &s_nodes[i]);
				pthread_mutex_unlock(&mutex_arr[hash_key[i]]);
				break;
			case 's':
				clock_gettime(CLOCK_MONOTONIC, &iStart);
				while (__sync_lock_test_and_set(&spin_lock_arr[hash_key[i]], 1));
                clock_gettime(CLOCK_MONOTONIC, &iStop);
				SortedList_insert(&slist_arr[hash_key[i]], &s_nodes[i]);
				__sync_lock_release(&spin_lock_arr[hash_key[i]]);
				break;
			default:
				SortedList_insert(&slist_arr[hash_key[i]], &s_nodes[i]);
		}
        time += 1000000000L*(iStop.tv_sec - iStart.tv_sec) + (iStop.tv_nsec-iStart.tv_nsec);
	}
	int len = 0;
	switch(opt_sync){
		case 'm':
			for (i = 0; i < num_list; i++){
				clock_gettime(CLOCK_MONOTONIC, &lStart);
				pthread_mutex_lock(&mutex_arr[i]);
                clock_gettime(CLOCK_MONOTONIC, &lStop);
				len += SortedList_length(&slist_arr[i]);
				pthread_mutex_unlock(&mutex_arr[i]);
                time += 1000000000L*(lStop.tv_sec - lStart.tv_sec) + (lStop.tv_nsec-lStart.tv_nsec);
			}
				break;
		case 's':
			for (i = 0; i < num_list; i++){
				clock_gettime(CLOCK_MONOTONIC, &lStart);
				while (__sync_lock_test_and_set(&spin_lock_arr[i], 1));
                clock_gettime(CLOCK_MONOTONIC, &lStop);
				len += SortedList_length(&slist_arr[i]);
				__sync_lock_release(&spin_lock_arr[i]);
                time += 1000000000L*(lStop.tv_sec - lStart.tv_sec) + (lStop.tv_nsec-lStart.tv_nsec);
			}
			break;
		default:
			for (i = 0; i < num_list; i++){	
				len += SortedList_length(&slist_arr[i]);
			}
	}
	

	SortedListElement_t *node;
	for (i = *(int*)tid ; i < total_ops; i+=num_thrd){
		switch(opt_sync){
			case 'm':
				clock_gettime(CLOCK_MONOTONIC, &dStart);
				pthread_mutex_lock(&mutex_arr[hash_key[i]]);
                clock_gettime(CLOCK_MONOTONIC, &dStop);
				node = SortedList_lookup(&slist_arr[hash_key[i]], s_nodes[i].key);
				SortedList_delete(node);
				pthread_mutex_unlock(&mutex_arr[hash_key[i]]);
				break;
			case 's':
				clock_gettime(CLOCK_MONOTONIC, &dStart);
				while (__sync_lock_test_and_set(&spin_lock_arr[hash_key[i]], 1));
                clock_gettime(CLOCK_MONOTONIC, &dStop);
				node = SortedList_lookup (&slist_arr[hash_key[i]], s_nodes[i].key);
				SortedList_delete(node);
				__sync_lock_release(&spin_lock_arr[hash_key[i]]);
				break;
			default:
				node = SortedList_lookup(&slist_arr[hash_key[i]], s_nodes[i].key);
				SortedList_delete(node);
		}
        time += 1000000000L*(dStop.tv_sec - dStart.tv_sec) + (dStop.tv_nsec-dStart.tv_nsec);
	}
     time_arr[*(int*)tid] = time;
	return NULL;
}

int hash_func(const char* key){
	int i = 0;
	int sum = 0;
	int x = 1;
	for (i = 0; i < strlen(key); i++){
		sum += key[i]*x;
	}
	return sum % num_list;
}

int main(int argc, char** argv){
	int i;
	static struct option long_opts[] = {
		{"threads", optional_argument, 0, 't'},
		{"yield", required_argument, 0, 'y'},
		{"sync", required_argument, 0, 's'},
		{"iterations", optional_argument, 0, 'i'},
		{"lists", required_argument, 0, 'l'}
	};
	int opt;
	char *ch;
	char str[20];
	int yield_flag = 0;
	long lock_time = 0;
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
				break;
			case 'i':
				num_iter = atoi(optarg);
				break;
			case 'l':
				num_list = atoi(optarg);
				break;
			default:
				fprintf(stderr, "Invalid argument: ./lab2_ [--iterations=#][--threads=#][--yield][--sync]\n");
				exit(1);
		}
	}

	total_ops = num_thrd * num_iter;
	//Init List
	slist_arr = malloc(num_list*sizeof(SortedList_t));
	for (i = 0; i < num_list; i++){
		slist_arr[i].key = NULL;
		slist_arr[i].next = &slist_arr[i];
		slist_arr[i].prev = &slist_arr[i];
	}
	//generate lock  arrays
	if (opt_sync == 'm'){
		mutex_arr = malloc(num_list*sizeof(pthread_mutex_t));
		for (i = 0; i < num_list; i++)
			pthread_mutex_init(&mutex_arr[i], NULL);
	}
	else if (opt_sync == 's'){
		spin_lock_arr = malloc(num_list*sizeof(int));
		for (i = 0; i < num_list; i++)
			spin_lock_arr[i] = 0;
	}

	time_arr = malloc(num_thrd*sizeof(long));
	
	//generate keys
	s_nodes = malloc(total_ops*sizeof(SortedListElement_t));
	static const char alpha[] = "ABCDEFGHIJKLMNQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	int j;
	srand(time(NULL));
	for (i = 0; i < total_ops; i++){
		char* key  = malloc(6*sizeof(char));
		for (j = 0; j < 5; j++)
			key[j] = alpha[rand() % (int) (sizeof alpha -1)];
		key[5] = '\0';
		s_nodes[i].key = key;
	}


	//generate hashes 
	hash_key = malloc(total_ops*sizeof(int));
	for (i = 0; i < num_thrd; i++){
		hash_key[i] = hash_func(s_nodes[i].key);
	}

	pthread_t *threads = malloc(num_thrd * sizeof(pthread_t));
	int ret[num_thrd];
	struct timespec start, stop;
    
    	clock_gettime(CLOCK_MONOTONIC, &start);
	for (i = 0; i < num_thrd; i++){
		ret[i] = i;
		if (pthread_create(threads + i, NULL, thread_func, &ret[i])){
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
	long exec_time = 1000000000L * (stop.tv_sec - start.tv_sec) + (stop.tv_nsec - start.tv_nsec);
	total_ops = total_ops * 3;
	long avg_time = exec_time/total_ops;
	
	long total_time = 0;
    	for (i = 0; i < num_thrd; i++)
		total_time += time_arr[i];

   	if(opt_sync == 'm' || opt_sync == 's')
        	lock_time = total_time/total_ops;
   
	int len = 0;
	for (i = 0; i < num_list; i++)
		len += SortedList_length(&slist_arr[i]);

	if(len != 0){
		fprintf(stderr, "Length not zero");
		exit(2);
	}

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

	fprintf(stdout,"%s,%d,%d,%d,%d,%ld,%ld,%ld\n",str,num_thrd, num_iter, num_list, total_ops, exec_time, avg_time, lock_time);
	free(hash_key);
	free(spin_lock_arr);
	exit(0);
}
