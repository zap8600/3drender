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
        fprintf(stderr, "Usage: %s [output obj file] [slices] [rings]\n", argv[0]);
    }

    const float slices = (float)atoi(argv[2]); // The amount of parts around the y axis
    const float rings = (float)atoi(argv[3]); // The amout of layer in between 1 and -1

    FILE* obj = fopen(argv[1], "w");

    // First I need to set up an array for the vertexs
    vtex* vtexs = (vtex*)malloc(sizeof(vtex));
    // I also need to keep track of how many I have so far
    int vtexamt = 1;

    // An optimization so that we don't need to calculate the top and bottom vertexs
    vtexs[0].x = 0;
    vtexs[0].y = 1;
    vtexs[0].z = 0;

    // Create a circle of different sizes at different Y levels, 0.5 being the biggest circle and ~1 and ~-1 being the smallest
    for(int i = 0; i < rings; i++) {
        float p = PI * (((float)i + 1) / rings); // Goes from 1 to -1, used for the Y level and the size of the circle
        for(int j = 0; j < slices; j++) {
            float t = (2 * PI) * (((float)j) / slices); // Making the circle
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

    for(int i = 1; i < slices + 1; i++) {
        int v1 = i + 1;
        int v2 = (i + 1) % (slices + 1); // Next vertex over
    }

    fclose(obj);
    free(vtex);
}