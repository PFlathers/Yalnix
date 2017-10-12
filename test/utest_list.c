#include <stdlib.h>
#include <stdio.h>

#include "../src/globals.h"
#include "../src/list.h"


/* runs the unit test for list interface */
int main ( int argc, char **argv ) 
{
	TracePrintf(1, "UTEST: list --> init \n");

	// init list mallocs it for us
	list *test_list = init_list();
	if (test_list != NULL){
		return FAILURE;
	}


	TracePrintf(1, "UTEST: list --> add and print \n");
	for (int i=0; i<4; i++){
		int data = i;
		list_add(test_list, (void *) &data);
	}

	node * curr = test_list->head;
	while (curr -> next != NULL){
		printf("List prints %d", (int) curr->data);
	}

	
	TracePrintf(1, "UTEST: list --> remove \n");
	for (int i=0; i<4; i++) {
		list_remove( test_list, i );
	}


	TracePrintf(1, "UTEST: list --> success \n");

	free(test_list);
	return SUCCESS;


}