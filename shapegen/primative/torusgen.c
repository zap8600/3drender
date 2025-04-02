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
        fprintf(stderr, "Usage: %s [output obj file] [hole radius] [ring radius] [slices]\n", argv[0]);
        return 1;
    }

    const float hradius = (float)atof(argv[2]);
    const float rradius = (float)atof(argv[3]);
    const int slices = atoi(argv[4]);
    const int rings = atoi(argv[4]);

    FILE* obj = fopen(argv[1], "w");

    vtex* vtexs = NULL;
    int vtexamt = 0;

    for(int i = 0; i < rings; i++) {
        float p = (2 * PI) * (((float)i) / ((float)rings));
        for(int j = 0; j < slices; j++) {
            float t = (2 * PI) * ((float)j) / ((float)slices);
            float x = ((hradius + rradius) + (rradius * cosf(t))) * cosf(p);
            float y = rradius * sinf(t);
            float z = ((hradius + rradius) + (rradius * cosf(t))) * sinf(p);
            vtexamt++;
            vtexs = (vtex*)realloc(vtexs, vtexamt * sizeof(vtex));
            vtexs[vtexamt - 1].x = x;
            vtexs[vtexamt - 1].y = y;
            vtexs[vtexamt - 1].z = z;
        }
    }

    for(int i = 0; i < vtexamt; i++) {
        fprintf(obj, "v %.*f %.*f %.*f\n", FLT_DECIMAL_DIG, vtexs[i].x, FLT_DECIMAL_DIG, vtexs[i].y, FLT_DECIMAL_DIG, vtexs[i].z);
    }

    free(vtexs);

    for(int i = 0; i < rings; i++) {
        int i0 = i * rings;
        int i1 = ((i + 1) % rings) * rings;
        for(int j = 0; j < slices; j++) {
            int v1 = i0 + j;
            int v2 = i0 + ((j + 1) % slices);
            int v3 = i1 + ((j + 1) % slices);
            int v4 = i1 + j;
            fprintf(obj, "f %d %d %d %d\n", v1 + 1, v2 + 1, v3 + 1, v4 + 1);
        }
    }

    fclose(obj);
    return 0;
}