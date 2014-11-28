#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <ctime>

#include "bptree.h"

int main(int argc, char** argv) {
    BPTree tree;
    for (int i=1; i<=4000000; i++) {
        char* value = (char*)malloc(20);
        sprintf(value, "key: %i", i);
        tree.insert(i, (void*)value);
    }
    printf("depth: %li\n", tree.depth());
    clock_t start = clock();
    for (int i=1; i<=4000000; i+=4) {
        void* foo;
        tree.search(i, &foo);
    }
    printf("time: %i\n", (int)(clock() - start));
    return 0;
}
