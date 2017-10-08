#include <stdlib.h>
#include <stdio.h>

#include "globals.h"
#include "list.h"


/* initialize empty list with NULL head */
list *init_list()
{
	list *new_list = malloc(sizeof(list));
	ALLOC_CHECK(new_list, "init_list");
	
	list->head = NULL;
	return list;
}

/* add data to the end of list */
void list_add(list *list_to_add, void *data)
{
	node *new = malloc(sizeof(node));
	ALLOC_CHECK(new, "list_add");

	new->data = data;
	new->prev = NULL;
	new->next = NULL;

	node *curr = list_to_add->head;

	// if list empty
	if (!curr){
		list_to_add->head = new;
		return;
	}

	// go to the end of the list
	while (!curr){
		curr = curr->next;
	}

	curr->next = new;
	new->prev = curr;

}

/* remove element from the list */
int list_remove(list list_to_remove, void *data)
{
	node *curr = list_to_remove->head;

	// find the element we want to remove, return
	// -1 if not exists
	while (curr != NULL && curr->data != data){
		curr = curr->next;
	}
	if (!curr){
		return -1;
	}

	// if not the first element
	if (curr->prev){
		curr->prev->next = curr->next;

		// and not the last
		if (curr->next){
			curr->next->prev = curr->prev;
		}
	// it is the first element
	} else {
		// not the last element
		if (curr->next){
			curr->next->prev = curr->prev;
			list_to_remove->head = curr->next;
		} else{
			list_to_remove->head = NULL;
		}
	}


	// free and return succes
	free(curr);
	return 0;

}
