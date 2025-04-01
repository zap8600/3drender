#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>

// Took some time to figure out how sin and cos make circles and spheres... let's hope my math is right

#define PI 3.14159265358979323846f

typedef struct vec3 {
    float x;
    float y;
    float z;
} vec3;

typedef vec3 vtex;

int main(int argc, char** argv) {
    if(argc != 4) {
        fprintf(stderr, "Usage: %s [output obj file] [radius] [slices]\n", argv[0]);
        return 1;
    }

    const float radius = (float)atof(argv[2]); // The radius of the sphere
    const int slices = atoi(argv[3]); // The amount of parts around the y axis
    const int rings = atoi(argv[3]); // The amout of layer in between 1 and -1

    FILE* obj = fopen(argv[1], "w");

    // First I need to set up an array for the vertexs
    vtex* vtexs = (vtex*)malloc(sizeof(vtex));
    // I also need to keep track of how many I have so far
    int vtexamt = 1;

    // An optimization so that we don't need to calculate the top and bottom vertexs
    vtexs[0].x = 0;
    vtexs[0].y = radius;
    vtexs[0].z = 0;

    // Create a circle of different sizes at different Y levels, 0.5 being the biggest circle and ~1 and ~-1 being the smallest
    for(int i = 0; i < rings; i++) {
        float p = PI * (((float)i + 1) / ((float)rings)); // Goes from 1 to -1, used for the Y level and the size of the circle
        for(int j = 0; j < slices; j++) {
            float t = (2 * PI) * (((float)j) / ((float)slices)); // Making the circle
            float x = sinf(p) * (radius * cosf(t));
            float y = radius * cosf(p);
            float z = sinf(p) * (radius * sinf(t));
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
    vtexs[vtexamt - 1].y = -radius;
    vtexs[vtexamt - 1].z = 0;

    for(int i = 0; i < vtexamt; i++) {
        fprintf(obj, "v %.*f %.*f %.*f\n", FLT_DECIMAL_DIG, vtexs[i].x, FLT_DECIMAL_DIG, vtexs[i].y, FLT_DECIMAL_DIG, vtexs[i].z);
    }

    free(vtexs);

    // Add the top and bottom triangles
    for(int i = 0; i < slices; i++) {
        int v1 = i + 1;
        int v2 = ((i + 1) % slices) + 1; // Next vertex over
        fprintf(obj, "f 1 %d %d\n", v1 + 1, v2 + 1); // Adding one since the vertex array in an obj starts at 1 instead of 0
        v1 = (i + (slices * (rings - 2))) + 1; // Subtract 2 from rings to exclude the top and bottom
        v2 = (((i + 1) % slices) + (slices * (rings - 2))) + 1;
        fprintf(obj, "f %d %d %d\n", vtexamt, v1 + 1, v2 + 1);
    }

    // Add the squares per ring / slice
    for(int i = 0; i < rings - 2; i++) {
        int i0 = (i * rings) + 1;
        int i1 = ((i + 1) * rings) + 1;
        for(int j = 0; j < slices; j++) {
            int v1 = i0 + j;
            int v2 = i0 + ((j + 1) % slices);
            int v3 = i1 + ((j + 1) % slices);
            int v4 = i1 + j;
            fprintf(obj, "f %d %d %d\n", v1 + 1, v2 + 1, v4 + 1);
            fprintf(obj, "f %d %d %d\n", v4 + 1, v3 + 1, v2 + 1);
        }
    }

    fclose(obj);
    return 0;
}