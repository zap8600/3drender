#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>

#define PI 3.14159265358979323846f

typedef struct vec3 {
    float x;
    float y;
    float z;
} vec3;

typedef vec3 vtex;

int main(int argc, char** argv) {
    if(argc != 5) {
        fprintf(stderr, "Usage: %s [output obj file] [radius] [height] [slices]\n", argv[0]);
    }

    const float radius = (float)atof(argv[2]);
    const float height = (float)atof(argv[3]);
    const int slices = atoi(argv[4]);

    FILE* obj = fopen(argv[1], "w");

    vtex* vtexs = (vtex*)malloc(2 * sizeof(vtex));
    int vtexamt = 2;

    vtexs[0].x = 0;
    vtexs[0].y = 0;
    vtexs[0].z = 0;

    vtexs[1].x = 0;
    vtexs[1].y = height;
    vtexs[1].z = 0;

    for(int i = 0; i < slices; i++) {
        float p = (2 * PI) * (((float)i) / ((float)slices));
        float x = radius * cosf(p);
        float y = -1;
        float z = radius * sinf(p);
        vtexamt++;
        vtexs = (vtex*)realloc(vtexs, vtexamt * sizeof(vtex));
        vtexs[vtexamt - 1].x = x;
        vtexs[vtexamt - 1].y = y;
        vtexs[vtexamt - 1].z = z;
    }

    for(int i = 0; i < slices; i++) {
        float p = (2 * PI) * (((float)i) / ((float)slices));
        float x = radius * cosf(p);
        float y = -1 + height;
        float z = radius * sinf(p);
        vtexamt++;
        vtexs = (vtex*)realloc(vtexs, vtexamt * sizeof(vtex));
        vtexs[vtexamt - 1].x = x;
        vtexs[vtexamt - 1].y = y;
        vtexs[vtexamt - 1].z = z;
    }

    for(int i = 0; i < vtexamt; i++) {
        fprintf(obj, "v %.*f %.*f %.*f\n", FLT_DECIMAL_DIG, vtexs[i].x, FLT_DECIMAL_DIG, vtexs[i].y, FLT_DECIMAL_DIG, vtexs[i].z);
    }

    free(vtexs);

    for(int i = 0; i < slices; i++) {
        int v1 = i + 2;
        int v2 = ((i + 1) % slices) + 2;
        fprintf(obj, "f 1 %d %d\n", v1 + 1, v2 + 1);
    }

    for(int i = 0; i < slices; i++) {
        int v1 = (i + slices) + 2;
        int v2 = (((i + 1) % slices) + slices) + 2;
        fprintf(obj, "f 2 %d %d\n", v1 + 1, v2 + 1);
    }

    for(int i = 0; i < slices; i++) {
        int v1 = i + 2;
        int v2 = ((i + 1) % slices) + 2;
        int v3 = (((i + 1) % slices) + slices) + 2;
        int v4 = (i + slices) + 2;
        fprintf(obj, "f %d %d %d %d\n", v1 + 1, v2 + 1, v3 + 1, v4 + 1);
    }

    fclose(obj);
    return 0;
}