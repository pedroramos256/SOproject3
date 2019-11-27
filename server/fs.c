#include "fs.h"
#include "sync.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* functions for rename command */
#define min(a,b) ((a) <(b)) ? (a) : (b)
#define max(a,b) ((a) > (b)) ? (a) : (b)


int obtainNewInumber(tecnicofs* fs) {
	int newInumber = ++(fs->nextINumber);
	return newInumber;
}

tecnicofs* new_tecnicofs(){
	int i;
	tecnicofs*fs = malloc(sizeof(tecnicofs));
	if (!fs) {
		perror("failed to allocate tecnicofs");
		exit(EXIT_FAILURE);
	}
	fs->bstRoot = (node **) malloc(sizeof(node *) * numberBuckets);
	if (!fs->bstRoot) {
		perror("failed to allocate bstRoot");
		exit(EXIT_FAILURE);
	}
	/* begin every bucket with NULL (no bst) */
	for (i = 0; i < numberBuckets; i++) {
		fs->bstRoot[i] = NULL;
	}
	fs->nextINumber = 0;
	return fs;
}

void free_tecnicofs(tecnicofs* fs){
	int i;
	for (i = 0; i < numberBuckets; i++)
		free_tree(fs->bstRoot[i]);
	free(fs->bstRoot);
	free(fs);
}

void create(tecnicofs* fs, char *name, int inumber){
	int index;
	index = hash(name, numberBuckets);
	WRLOCK(index);
	fs->bstRoot[index] = insert(fs->bstRoot[index], name, inumber);
	UNLOCK(index);
}

void delete(tecnicofs* fs, char *name){
	int index;
	index = hash(name, numberBuckets);
	WRLOCK(index);
	fs->bstRoot[index] = remove_item(fs->bstRoot[index], name);
	UNLOCK(index);
}

int lookup(tecnicofs* fs, char *name){
	int index,inumber = -1;
	index = hash(name, numberBuckets);
	RDLOCK(index);
	node *searchNode = search(fs->bstRoot[index], name);
	if ( searchNode ) inumber = searchNode->inumber;
	UNLOCK(index);
	return inumber;								
}

void exchange(tecnicofs* fs, char* name1, char* name2, int inumber) {
	int index1, index2;
	index1 = hash(name1, numberBuckets);
	index2 = hash(name2, numberBuckets);

	if(index1 == index2) {
		WRLOCK(index1);
		fs->bstRoot[index1] = remove_item(fs->bstRoot[index1], name1);
		fs->bstRoot[index1] = insert(fs->bstRoot[index1], name2, inumber);
		UNLOCK(index1);
	}
	/*calc min and max to keep locking order always the same (ascending order)*/
	else {
		WRLOCK(min(index1,index2));
		WRLOCK(max(index1,index2));
		fs->bstRoot[index1] = remove_item(fs->bstRoot[index1], name1);
		fs->bstRoot[index2] = insert(fs->bstRoot[index2], name2, inumber);
		UNLOCK(max(index1,index2));
		UNLOCK(min(index1,index2));
	}
	
}

void print_tecnicofs_tree(FILE * fp, tecnicofs *fs){
	int i;
	for (i = 0; i < numberBuckets; i++){
		fprintf(fp,"\ntree %d:\n",i);
		print_tree(fp, fs->bstRoot[i]);
	}
}
