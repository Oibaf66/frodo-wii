#ifndef __slist_h
#define __slist_h

struct _S_LIST
{
	int Elements;
	char **List;
	int Sorted;
};
typedef struct _S_LIST S_List;

#define S_SORTED 	1
#define S_UNSORTED 	0


/************************************************************/
extern S_List*	S_CreateList(int Sorted);
extern int 		S_AddToList(S_List *List, char *String);
extern void 	S_DestroyList(S_List *List);
/************************************************************/

#endif /* __slist_h */
