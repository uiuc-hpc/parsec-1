#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

typedef struct {
    off_t         tname;
    off_t         accesses;
    int           nbsucc;
    int           succ[1];
} filenode_t;

typedef struct {
    int           nbnodes;
    off_t         nodes[1];
} filenode_header_t;

typedef struct node {
    char         *tname;
    char         *accesses;
    int           done;
    int           nbsucc;
    struct node **succ;
    int           nbpred;
    struct node **pred;
} node_t;

typedef struct {
    node_t  **node;
    int       size;
    int       allocated;
} nl_t;

static void nl_add(nl_t *s, node_t *n);

static void filenode_add_pred(node_t *n, node_t *p)
{
    n->nbpred++;
    n->pred = (node_t**)realloc( n->pred, n->nbpred * sizeof(node_t*) );
    n->pred[n->nbpred-1] = p;
}

static off_t load_single_node(char *m, off_t offset, int i, node_t *allnodes)
{
    filenode_t *n;
    node_t *t;
    int j;

    t = &allnodes[i];
    t->tname    = strdup( (char*)(m + offset) );
    offset += strlen( t->tname ) + 1;
    fprintf(stderr, " (%s, ", t->tname );

    t->accesses = strdup( (char*)(m + offset) );
    offset += strlen( t->accesses ) + 1;
    fprintf(stderr, " %s, ", t->accesses );

    t->nbsucc = ((int*)(m + offset))[0];
    offset += sizeof(int);
    fprintf(stderr, " %d) -> ", t->nbsucc );

    if( t->nbsucc > 0 ) {
        t->succ = (node_t**)malloc( t->nbsucc * sizeof(node_t*) );
        for(j = 0; j < t->nbsucc; j++) {
            int succ = ((int*)(m + offset))[0];
            offset += sizeof(int);
            t->succ[j] = &(allnodes[ succ ]);
            filenode_add_pred( &(allnodes[ succ ]), t );
            fprintf(stderr, " %d", succ );
        }
    } else {
        t->succ = NULL;
    }
    return offset;
}

static nl_t *load_filenode(const char *filename, int *nbnodes)
{
    int fd, i;
    struct stat s;
    char *m;
    filenode_header_t *h;
    nl_t *r = NULL;
    node_t *allnodes;
    off_t offset;

    if( (fd = open(filename, O_RDONLY)) == -1 ) {
        perror(filename);
        return r;
    }

    if( fstat(fd, &s) == -1 ) {
        perror(filename);
        close(fd);
        return r;
    }

    fprintf( stderr, "pagesize: %d, %d\n", (int)(s.st_size), getpagesize() );
    assert( s.st_size % getpagesize() == 0 );

    if( (m = mmap(NULL, s.st_size, PROT_READ, MAP_PRIVATE, fd, 0)) == MAP_FAILED ) {
        perror(filename);
        close(fd);
        return r;
    }
    close(fd);

    *nbnodes = ((int*)m)[0];
    fprintf(stderr, "nbnodes: %d\n", *nbnodes);

    r = (nl_t*)malloc(sizeof(nl_t));
    r->size = 0;
    r->allocated = 1;
    r->node = (node_t**)malloc(1 * sizeof(node_t*));

    allnodes = (node_t*)calloc(*nbnodes, sizeof(node_t));

    offset = sizeof(int);
    for(i = 0; i < *nbnodes; i++) {
        fprintf(stderr, "Noeud %d : ", i);
        offset = load_single_node(m, offset, i, allnodes);
        fprintf(stderr, "\n");
    }

    for(i = 0; i < h->nbnodes; i++) {
        if( allnodes[i].nbpred == 0 ) {
            nl_add(r, &allnodes[i]);
        }
    }

    munmap( m, s.st_size );

    return r;
}

static nl_t *nl_dup(const nl_t *s)
{
    nl_t *n = (nl_t*)malloc(sizeof(nl_t));
    n->node = (node_t**)malloc(s->allocated * sizeof(node_t*));
    memcpy(n->node, s->node, s->size * sizeof(node_t*));
    n->size = s->size;
    n->allocated = s->allocated;
    return n;
}

static void nl_free(nl_t *s)
{
    free(s->node);
    s->node = NULL;
    s->size = -1;
    s->allocated = -1;
    free(s);
}

static void nl_remove(nl_t *s, int p)
{
    assert( p>=0 && p < s->size );
    s->node[p] = s->node[s->size-1];
    s->size--;
}

static void nl_add(nl_t *s, node_t *n)
{
    if( s->size == s->allocated ) {
        s->allocated = s->allocated*2;
        s->node = (node_t**)realloc(s->node, s->allocated * sizeof(node_t*));
    }
    s->node[s->size] = n;
    s->size++;
}

static void display_node_list(nl_t *s)
{
    int i;
    for(i = 0; i < s->size; i++) {
        printf("%s[%s] ", s->node[i]->tname, s->node[i]->accesses);
    }
    printf("\n");
}

static void display_node_array(node_t **word, int len)
{
    int i;
    for(i = 0; i < len; i++) {
        printf("%s#%s# ", word[i]->tname, word[i]->accesses);
    }
    printf("\n");
}

static void walk(node_t **word, int pos, nl_t *ready) {
    int i, j, k;
    nl_t *myready;
    node_t *s, *e;

    //    printf("entering level %d with a list of size %d\n", pos, ready->size);

    if( ready->size == 0 ) {
        word[pos] = NULL;
        display_node_array(word, pos);
        return;
    }

    for(i = 0; i < ready->size; i++) {
        e = ready->node[i];
        e->done = 1;
        word[pos] = e;

        myready = nl_dup(ready);
        //        printf("Ready at this level: "); display_node_list(myready);
        nl_remove(myready, i);
        for(j = 0; j < e->nbsucc; j++) {
            s = e->succ[j];
            if( s->done == 1 ) {
                continue;
            }
            for(k = 0; k < s->nbpred; k++) {
                if( s->pred[k]->done == 0 )
                    break;
            }
            if( k == s->nbpred ) {
                //                printf("%s is now ready\n", s->tname);
                nl_add(myready, s);
            }
        }
        walk(word, pos+1, myready);
        nl_free(myready);

        e->done = 0;
    }
}

int main(int argc, char *argv[])
{
    nl_t *graph;
    node_t **word;
    int nb_nodes;
    graph = load_filenode("dummy.grp", &nb_nodes);
    if( NULL == graph )
        return 1;
    word = (node_t**)calloc(nb_nodes+1, sizeof(node_t*));
    walk(word, 0, graph);
    return 0;
}
