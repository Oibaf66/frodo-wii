#include "slist.h"

/**********************************************************************************************************/
S_List *S_CreateList(int Sorted)
{
	S_List *List;

	List = (S_List*)malloc(sizeof(S_List));
	if (!List)
		return NULL;
	List->Elements = 0;
	List->List = NULL;
	List->Sorted = Sorted;
	return List;

}
/**********************************************************************************************************/
int S_AddToList(S_List* List, char *String)
{
	char **NewList;

	List->Elements++;
	NewList = (char**)realloc(List->List, (size_t)(List->Elements * sizeof(char *)));
	if (!NewList)
	{
		List->Elements--;
		return 0;
	}
	List->List = NewList;
	List->List[List->Elements - 1] = (char *)malloc((size_t)(strlen(String) + 1));
	if (!List->List[List->Elements - 1])
	{
		List->Elements--;
		List = (S_List*)realloc(List->List, (size_t)(List->Elements * sizeof(char *)));
		return 0;
	}
	strcpy(List->List[List->Elements - 1], String);
	return 1;
}
/**********************************************************************************************************/
void S_DestroyList(S_List* List)
{
	int nCount;

	if (List)
	{
		for (nCount = 0; nCount < (int)List->Elements; nCount ++)
		{
			if (List)
				free(List->List[nCount]);
		}
		if (List->List)
			free(List->List);
		free(List);
		List = NULL;

	}

}
