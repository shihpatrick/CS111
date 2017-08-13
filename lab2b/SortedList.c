#include "SortedList.h"
#include <pthread.h>
#include <string.h>

void SortedList_insert(SortedList_t *list, SortedListElement_t *element){
	if (element == NULL)
		return;
	if (list == NULL)
		return;

	SortedListElement_t *trav = list->next;
	while(trav != list){
		if (strcmp(element->key, trav->key) <= 0)
			break;
		trav = trav->next;
	}

	if (opt_yield & INSERT_YIELD)
		sched_yield();

	element->next = trav;
	element->prev = trav->prev;
	trav->prev->next = element;
	trav->prev = element;
}


int SortedList_delete( SortedListElement_t *element){
	if (element == NULL)
		return 1;

	if (element->next->prev == element->prev->next){
		if (opt_yield & DELETE_YIELD)
			sched_yield();
		element->prev->next = element->next;
		element->next->prev = element->prev;
		return 0;
	}

	return 1;
}


SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key){
	if (list == NULL)
		return NULL;
	if (key == NULL)
		return NULL;

	SortedListElement_t *trav = list->next;
	while (trav != list){
		if (strcmp(key, trav->key) == 0)
			return trav;
		if (opt_yield & LOOKUP_YIELD)
			sched_yield();
		trav = trav->next;
	}
	return NULL;
}

int SortedList_length(SortedList_t *list){
	if (list == NULL){
		return -1;
	}
	int len = 0;
	SortedListElement_t *trav = list->next;
	while (trav != list){
		if (opt_yield & LOOKUP_YIELD)
			sched_yield();
		trav = trav->next;
		len++;
	}
	return len;
}
