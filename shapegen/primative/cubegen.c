#include <stdio.h>
#include <stdlib.h>
#include <float.h>

#define PI 3.14159265358979323846f

typedef struct vec3 {
    float x;
    float y;
    float z;
} vec3;

typedef vec3 vtex;

const int faces[6][4] = {
    {1, 2, 6, 5},
    {5, 6, 7, 8},
    {7, 8, 3, 4},
    {1, 2, 3, 4},
    {3, 5, 1, 7},
    {2, 4, 6, 8}
};

int main(int argc, char** argv) {
    if(argc != 5) {
        fprintf(stderr, "Usage: %s [output obj file] [length] [width] [height]\n", argv[0]);
        return 1;
    }

    const float length = ((float)atof(argv[2])) / 2;
    const float width = ((float)atof(argv[3])) / 2;
    const float height = ((float)atof(argv[4])) / 2;

    FILE* obj = fopen(argv[1], "w");

    vtex vtexs[8];
    int vtexamt;

    vtexs[0].x = length;
    vtexs[0].y = height;
    vtexs[0].z = width;

    vtexs[1].x = length;
    vtexs[1].y = -height;
    vtexs[1].z = width;

    vtexs[2].x = -length;
    vtexs[2].y = height;
    vtexs[2].z = width;

    vtexs[3].x = -length;
    vtexs[3].y = -height;
    vtexs[3].z = width;

    vtexs[4].x = length;
    vtexs[4].y = height;
    vtexs[4].z = -width;

    vtexs[5].x = length;
    vtexs[5].y = -height;
    vtexs[5].z = -width;

    vtexs[6].x = -length;
    vtexs[6].y = height;
    vtexs[6].z = -width;

    vtexs[7].x = -length;
    vtexs[7].y = -height;
    vtexs[7].z = -width;

    for(int i = 0; i < 8; i++) {
        fprintf(obj, "v %.*f %.*f %.*f\n", FLT_DECIMAL_DIG, vtexs[i].x, FLT_DECIMAL_DIG, vtexs[i].y, FLT_DECIMAL_DIG, vtexs[i].z);
    }

    for(int i = 0; i < 6; i++) {
        fprintf(obj, "f %d %d %d %d\n", faces[i][0], faces[i][1], faces[i][2], faces[i][3]);
    }

    fclose(obj);
}