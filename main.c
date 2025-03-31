#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define CNFG_IMPLEMENTATION
#include "rawdraw_sf.h"

void HandleKey( int keycode, int bDown ) { }
void HandleButton( int x, int y, int button, int bDown ) { }
void HandleMotion( int x, int y, int mask ) { }
int HandleDestroy() { return 0; }


#define PI 3.14159265358979323846f
#define DEG2RAD (PI/180.0f)

// 45 degrees
const float fov = 45*DEG2RAD;

typedef float vec3[3];
typedef float vec4[4];

typedef vec4 vtex;
typedef vec3 tcrd;
typedef vec3 nrmi;

typedef struct face {
    int* vtexs;
    int vtexamt;
    int* tcrds;
    int hastcrds
} face;

// Functions for manipulating 3D points

// Add one vecter to another
vec3 add(vec3 v1, vec3 v2) {
    vec3 r = {v1[0] + v2[0], v1[1] + v2[1], v1[2] + v2[2]};
    return r;
}

// Subtract one vector by another
vec3 sub(vec3 v1, vec3 v2) {
    vec3 r = {v1[0] - v2[0], v1[1] - v2[1], v1[2] - v2[2]};
    return r;
}

// Rotate a vector around the x axis by specified radians
vec3 rotx(vec3 in, float rad) {
    vec3 r;
    r[0] = in[0];
    r[1] = (in[1] * cosf(rad)) + (in[2] * (-(sinf(rad))));
    r[2] = (in[1] * sinf(rad)) + (in[2] * cosf(rad));
    return r;
}

// Rotate a vector around the y axis by specified radians
vec3 roty(vec3 in, float rad) {;
    vec3 r;
    r[0] = (in[0] * cosf(rad)) + (in[2] * (-(sinf(rad))));
    r[1] = in[1];
    r[2] = (in[0] * sinf(rad)) + (in[2] * cosf(rad));
    return r;
}

// Rotate a vector around the z axis by specified radians
vec3 rotz(vec3 in, float rad) {
    vec3 r;
    r[0] = (in[0] * cosf(rad)) + (in[1] * (-(sinf(rad))));
    r[1] = (in[0] * sinf(rad)) + (in[1] * cosf(rad));
    r[2] = in[2];
    return r;
}

// Normalize a vector
vec3 norm(vec3 in) {
    vec3 r = in;
    float l = sqrtf((in[0] * in[0]) + (in[1] * in[1]) + (in[2] * in[2]));
    if(l != 0) {
        float il = 1/l;
        r[0] *= il;
        r[1] *= il;
        r[2] *= il;
    }
    return r;
}

// Get the cross product of 2 vectors
vec3 cross(vec3 v1, vec3 v2) {
    vec3 r = {(v1[1] * v2[2]) - (v1[2] * v2[1]), (v1[2] * v2[0]) - (v1[0] * v2[2]), (v1[0] * v2[1]) - (v1[1] * v2[0])};
    return r;
}

// Get the dot product of 2 vectors
float dot(vec3 v1, vec3 v2) {
    float r = ((v1[0] * v2[0]) + (v1[1] * v2[1]) + (v1[2] * v2[2]));
    return r;
}


// Functions for reading models

int load_obj(const char* filename) {
    FILE* obj = fopen(filename, "r");
    if(obj == NULL) {
        fprintf(stderr, "Unable to open file %s!\n", filename);
        return 0;
    }
}

const vec3 up = {0, 1, 0};
const vec3 realmodelpos = {0, 0, 5};
const vec3 camerapos = {0, 0, 0};

int main(int argc, char **argv) {
    if(argc != 2) {
        fprintf(stderr, "Usage: %s [obj model file pathe]\n", argv[0]);
        return 1;
    }

    // Precomputing math required for rendering
    const float width = 512;
    const float height = 512;

    const float aspect = width/height; // 1

    const float zn = 1;
    const float zf = 10;

    const float atf = 1/(aspect*tanf(fov/2));
    const float tf = 1/tanf(fov/2);
    const float fnnf = (zf+zn)/(zn-zf);
    const float fnnf2 = ((2*zf)*zn)/(zn-zf);

    // Model loading
    vec3* vtexs = NULL;
    int vtexamt;

    face* faces = NULL;
    int faceamt;
    
    vec3 modelrotpos = {0, 0, 2};

    CNFGSetup("3D Renderer", (int)width, (int)height);

    clock_t lasttime = clock();

    while(CNFGHandleInput()) {
        CNFGClearFrame();
        CNFGColor(0xffffffff);

        clock_t now = clock();
        float delta = ((float)(now - lasttime)) / CLOCKS_PER_SEC;
        lasttime = now;

        // Rotate the object around the y axis in a circle -90 degrees per second
        modelrotpos = roty(modelrotpos, delta*(-0.5*PI));

        // Rotate the model around the y axis 90 degrees per second
        // Time is tracked by measuring how long the last frame was
        // Will not rotate when the window is being moved, but will jump to the new rotation when movement stops
        for(int i = 0; i < vtexamt; i++) {
            vtexs[i] = roty(vtexs[i], delta*(0.5*PI));
        }

        // Compute pixel coordinates of the points and draw lines
        for(int i = 0; i < faceamt * 3; i += 3) {
            int xps[3];
            int yps[3];

            for(int j = 0; j < 3; j++) {
                vec3 cvtex = add(add(vtexs[faces[i + j]], modelrotpos), realmodelpos);
                float vtexx = cvtex[0];
                float vtexy = cvtex[1];
                float vtexz = cvtex[2];
                float w = 1;

                vec3 za = norm(sub(camerapos, add(modelrotpos, realmodelpos)));
                vec3 xa = norm(cross(up, za));
                vec3 ya = cross(za, xa);

                // Convert from world space to camera space
                vtexx = (vtexx * xa[0]) + (vtexy * xa[1]) + (vtexz * xa[2]) + (w * (-(dot(xa, camerapos))));
                vtexy = (vtexx * ya[0]) + (vtexy * ya[1]) + (vtexz * ya[2]) + (w * (-(dot(ya, camerapos))));
                vtexz = (vtexx * za[0]) + (vtexy * za[1]) + (vtexz * za[2]) + (w * (-(dot(za, camerapos))));
                const float vz = vtexz;

                // Convert from camera space to NDC
                vtexx *= atf;
                vtexy *= tf;
                vtexz = (vtexz * fnnf) + (w * -1);
                w = vz * fnnf2;

                float xndc = vtexx/w;
                float yndc = vtexy/w;

                // Convert from NDC to screen space
                int xp = (int)(((xndc + 1) * width) / 2);
                int yp = (int)(((1 - yndc) * height) / 2);

                xps[j] = xp;
                yps[j] = yp;
            }

            CNFGTackSegment(xps[0], yps[0], xps[1], yps[1]);
            CNFGTackSegment(xps[1], yps[1], xps[2], yps[2]);
            CNFGTackSegment(xps[2], yps[2], xps[0], yps[0]);
        }

        CNFGSwapBuffers();
    }

    free(vtexs);
    free(faces);

    return 0;
}
