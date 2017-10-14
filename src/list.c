#include <stdlib.h>
#include "globals.h"
//#include "list.h"

// create list
List *init_list()
{
	List *new_list = malloc(sizeof(List));
	ALLOC_CHECK(new_list, "init_list");

	new_list->head = NULL;
	new_list->tail = NULL;

	return new_list;
}

// append to end of list
void list_add(List *list_to_add, void *data)
{
	Node *new_node = malloc(sizeof(Node));
	ALLOC_CHECK(new_node, "list_add");

	new_node->data = data;
	new_node->prev = NULL;
	new_node->next = NULL;

	// we have an empty list
	if(!list_to_add->tail){
		list_to_add->head = new_node;
		list_to_add->tail = new_node;
	// append onto end of list
	} else {
		list_to_add->tail->next = new_node;
		new_node->prev = list_to_add->tail;
		list_to_add->tail = new_node;
	}
}

// 0 if we found and removed the data. -1 if not exisit
int list_remove(List *list_to_remove, void *data)
{
	if (!list_to_remove->head){
		return -1;
	}
	Node *curr = list_to_remove->head;

	//find element to remove
	while (curr != NULL && curr->data != data){
		curr = curr->next;
	}

	//we didnt find the data
	if(!curr){
		return -1;
	}

	//if not the first element
	if(curr->prev){
		curr->prev->next = curr->next;

		// not the last element
		if(curr->next){
			curr->next->prev = curr->prev;
		// it is last element
		} else{
			//update last element if is last
			curr->prev->next = NULL;
			list_to_remove->tail = curr->prev;
		}
	// it is the first element
	} else {
		// and not the last element
		if(curr->next){
			curr->next->prev = curr->prev;
			list_to_remove->head = curr->next;
		//it is the last element
		} else {
			list_to_remove->head = NULL;
			list_to_remove->tail = NULL;
		}
	}

	free(curr);
	return 0;
}
