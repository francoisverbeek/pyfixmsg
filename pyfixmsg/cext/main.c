//
//  main.c
//  pyfixlib
//
//  Created by Francois Verbeek on 02/02/2017.
//  Copyright © 2017 Francois Verbeek. All rights reserved.
//

#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    uint32_t start;
    uint16_t len;
    uint16_t tag;
} tagentry_t;

int main(int argc, const char *argv[]) {
    // insert code here...
    printf("Hello, World!\n");
    uint64_t blo = 0;
    char *stuff = "ABCD";
    memcpy(&blo, stuff, 4);
    printf("Blo %llu\n", blo);
    char stuff2[5] = {0, 0, 0, 0, 0};
    memcpy(&stuff2, &blo, 4);
    printf(stuff2);
    printf("\n");
    printf("%d\n", 2 ^ 16);
    printf("%zu", sizeof(tagentry_t));
    printf("size of int %zu", sizeof(int));
}
