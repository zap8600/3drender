#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>

// https://danielsieger.com/blog/2021/03/27/generating-spheres.html#:~:text=The%20basic%20approach%20is%20to,and%20quadrilateral%20elements%20everywhere%20else.

#define PI 3.14159265358979323846f

typedef struct vec3 {
    float x;
    float y;
    float z;
} vec3;

typedef vec3 vtex;

int main(int argc, char** argv) {
    if(argc != 4) {
        fprintf(stderr, "Usage: %s [output obj file] [slices] [rings]\n", argv[0]);
    }

    FILE* obj = fopen(argv[1], "w");

    vtex* vtexs = malloc(sizeof(vtexs));
    int vtexamt = 1;

    vtexs[0].x = 0;
    vtexs[0].y = 1;
    vtexs[0].z = 0;

    const int slices = atoi(argv[2]);
    const int rings = atoi(argv[3]);

    for(int i = 0; i < rings; i++) {
        float p = PI * ((float)(i + 1)) / ((float)rings);
        for(int j = 0; j < slices; j++) {
            float t = 2 * PI * ((float)j) / ((float)slices);
            float x = sinf(p) * cosf(t);
            float y = cosf(p);
            float z = sinf(p) * sinf(t);
            vtexamt++;
            vtexs = (vtex*)realloc(vtexs, vtexamt * sizeof(vtex));
            vtexs[vtexamt - 1].x = x;
            vtexs[vtexamt - 1].y = y;
            vtexs[vtexamt - 1].z = z;
        }
    }

    vtexamt++;
    vtexs = (vtex*)realloc(vtexs, vtexamt * sizeof(vtex));
    vtexs[vtexamt - 1].x = 0;
    vtexs[vtexamt - 1].y = -1;
    vtexs[vtexamt - 1].z = 0;

    for(int i = 0; i < vtexamt; i++) {
        fprintf(obj, "v %.*f %.*f %.*f\n", FLT_DECIMAL_DIG, vtexs[i].x, FLT_DECIMAL_DIG, vtexs[i].y, FLT_DECIMAL_DIG, vtexs[i].z);
    }

    for(int i = 0; i < slices; i++) {
        int i0 = i + 1;
        int i1 = (i + 1) % slices + 1;
        fprintf(obj, "f ");
    }

    fclose(obj);
    free(vtexs);
}