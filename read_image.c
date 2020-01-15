#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "lc3.h"

/* change endianness */
uint16_t swap16(uint16_t x)
{
    return (x << 8) | (x >> 8);
}

uint16_t *read_image(const char *path)
{
    FILE *file = fopen(path, "rb");
    if (!file)
    {
        printf("[-] Read error!");
        return NULL;
    }
    uint16_t *memory = malloc(sizeof(INT16_MAX * sizeof(uint16_t)));

    /* the origin tells us where in memory to place the image */
    uint16_t origin;
    fread(&origin, sizeof(origin), 1, file);
    origin = swap16(origin);

    /* we know the maximum file size so we only need one fread */
    uint16_t max_read = UINT16_MAX - origin;
    uint16_t *p = memory + origin;
    size_t read = fread(p, sizeof(uint16_t), max_read, file);

    /* swap file content to little endian */
    while (read-- > 0)
    {
        *p = swap16(*p);
        ++p;
    }
    fclose(file);
    return memory;
}