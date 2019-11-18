#ifndef FS_H
#define FS_H
#include "sync.h"
#include "lib/bst.h"
#include "hash/hash.h"

typedef struct tecnicofs {
    node **bstRoot;
    int nextINumber;
} tecnicofs; 

int obtainNewInumber(tecnicofs* fs);
tecnicofs* new_tecnicofs();
void free_tecnicofs(tecnicofs* fs);
void create(tecnicofs* fs, char *name, int inumber);
void delete(tecnicofs* fs, char *name);
int lookup(tecnicofs* fs, char *name);
void exchange(tecnicofs* fs, char *name1, char *name2, int inumber);
void print_tecnicofs_tree(FILE * fp, tecnicofs *fs);


#endif /* FS_H */
