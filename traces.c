#include <stdio.h>
#include <stdlib.h>

#define CACHE_SIZE 32
#define NUM_SETS 4
#define SET_ASSOC 4

typedef struct {
    int valid;
    int tag;
    int counter;
} CacheLine;

CacheLine cache[CACHE_SIZE];
int hit = 0;
int access = 0;

void initCache() {
    for (int i = 0; i < CACHE_SIZE; i++) {
        cache[i].valid = 0;
        cache[i].tag = 0;
        cache[i].counter = 0;
    }
}

int getSetIndex(int address, int num_sets) {
    return (address / 4) % num_sets;
}

int getTag(int address) {
    return address / 16;
}

void randomReplace(int setIndex) {
    int index;
    do {
        index = setIndex * SET_ASSOC + rand() % SET_ASSOC;
    } while (cache[index].valid == 1);
    cache[index].valid = 1;
}

void lruReplace(int setIndex) {
    int oldest = access;
    int index;
    for (int i = 0; i < SET_ASSOC; i++) {
        index = setIndex * SET_ASSOC + i;
        if (cache[index].valid == 0) {
            cache[index].valid = 1;
            cache[index].counter = access;
            break;
        }
        if (cache[index].counter < oldest) {
            oldest = cache[index].counter;
            index = setIndex * SET_ASSOC + i;
        }
    }
    cache[index].valid = 1;
    cache[index].counter = access;
}

void accessCache(int address, char policy) {
    int setIndex = getSetIndex(address, NUM_SETS);
    int tag = getTag(address);
    int hitFlag = 0;
    for (int i = 0; i < SET_ASSOC; i++) {
        int index = setIndex * SET_ASSOC + i;
        if (cache[index].valid == 1 && cache[index].tag == tag) {
            hit++;
            hitFlag = 1;
            if (policy == 'l') {
                cache[index].counter = access;
            }
        }
    }
    if (hitFlag == 0) {
        access++;
        for (int i = 0; i < SET_ASSOC; i++) {
            int index = setIndex * SET_ASSOC + i;
            if (cache[index].valid == 0) {
                cache[index].valid = 1;
                cache[index].tag = tag;
                if (policy == 'l') {
                    cache[index].counter = access;
                }
                break;
            }
            if (i == SET_ASSOC - 1) {
                if (policy == 'r') {
                    randomReplace(setIndex);
                } else {
                    lruReplace(setIndex);
                }
                cache[setIndex * SET_ASSOC].tag = tag;
            }
        }
    }
}

int main() {
    initCache();
    FILE *file = fopen("traces.txt", "r");
    char accessType;
    int address;
    while (fscanf(file, "%c %x\n", &accessType, &address) != EOF) {
        accessCache(address, 'r');
    }
    printf("Random Replacement:\n");
    printf("Number of hits: %d\n", hit);
    printf("Number of accesses: %d\n", access);
    printf("Hit rate: %.2f%%\n", (float) hit / access * 100);

    hit = 0;
}
