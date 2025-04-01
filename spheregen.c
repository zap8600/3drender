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

    for(int i = 0; i < rings; i++) {
        float p;
    }

    fclose(obj);
    free(vtex);
}