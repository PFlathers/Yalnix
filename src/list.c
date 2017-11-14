#include <stdlib.h>
#include "globals.h"
#include "list.h"

// create list
List *init_list()
{
	List *new_list = malloc(sizeof(List));
	ALLOC_CHECK(new_list, "init_list");

	new_list->head = NULL;
	new_list->tail = NULL;
        new_list->count = 0;

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
        list_to_add->count++;
}

// Adds a node with the data passed onto the head of the list
void list_push(List *list_to_push, void *data)
{
	Node *new_node = malloc(sizeof(Node));
	ALLOC_CHECK(new_node, "list_add");

	new_node->data = data;
	new_node->prev = NULL;
	new_node->next = NULL;

	// we have an empty list
	if(!list_to_push->tail){
		list_to_push->head = new_node;
		list_to_push->tail = new_node;
	// append onto begining of list
	} else {
                new_node->next = list_to_push->head;
                list_to_push->head->prev = new_node;
                list_to_push->head = new_node;
	}
        list_to_push->count++;
} 

// 0 if we found and removed the data. -1 if not exisit
// Remove the first node in the list containing the data passed.
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
        list_to_remove->count--;
	free(curr);
	return 0;
}

// 0 if we found and removed the data. -1 if not exisit
// Remove the first node in the list containing the data passed.
// delete reference to the data as well
int list_remove_delete(List *list_to_remove, void *data)
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
        list_to_remove->count--;
	free(curr->data);
	free(curr);
	return 0;
}

// returns the data for the first node in the list
void *list_pop(List *list_to_pop)
{
        //some checks on the list
        if(list_to_pop == NULL || list_to_pop->count == 0)
                return NULL;

        //pointer gymnatstics
	Node *pop_node = list_to_pop->head;
	void *ret_data = pop_node->data;
	list_to_pop->head = list_to_pop->head->next;

        //Does check so we do not dereference a null pointer if we have an 
        // empty list
        if(list_to_pop->head != NULL)
                list_to_pop->head->prev = NULL;

        list_to_pop->count--;
	free(pop_node);

	return ret_data;
}

// gets the count in a nice function
int list_count(List *list_to_count)
{
        return list_to_count->count;
}
