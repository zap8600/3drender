#include <stdio.h>
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


// Points for a test cube
float vtexs[24] = {1, 1, 4,  1, -1, 4,  -1, 1, 4,  -1, -1, 4,  1, 1, 2,  1, -1, 2,  -1, 1, 2,  -1, -1, 2};
const int faces[36] = {0, 1, 5,  0, 5, 4,  4, 5, 7,  4, 6, 7,  6, 7, 3,  6, 2, 3,  2, 3, 1,  2, 0, 1,  2, 4, 6,  2, 0, 4,  1, 3, 5,  3, 5, 7};


// Functions for manipulating 3D objects

// Rotate a vertex around the x axis by specified radians
void rotx(float* in, float* out, float rad) {
    out[0] = in[0];
    out[1] = (in[1] * cosf(rad)) + (in[2] * (-(sinf(rad))));
    out[2] = (in[1] * sinf(rad)) + (in[2] * cosf(rad));
}

// Rotate a vertex around the y axis by specified radians
void roty(float* in, float* out, float rad) {
    out[0] = (in[0] * cosf(rad)) + (in[2] * (-(sinf(rad))));
    out[1] = in[1];
    out[2] = (in[0] * sinf(rad)) + (in[2] * cosf(rad));
}

// Rotate a vertex around the z axis by specified radians
void rotz(float* in, float* out, float rad) {
    out[0] = (in[0] * cosf(rad)) + (in[1] * (-(sinf(rad))));
    out[1] = (in[0] * sinf(rad)) + (in[1] * cosf(rad));
    out[2] = in[2];
}


int main(int argc, char **argv) {
    /*
    const float width = (float)atoi(argv[1]); 
    const float height = (float)atoi(argv[2]);
    */
    const float width = 512;
    const float height = 512;

    const float aspect = width/height; // 1

    const float zn = 1;
    const float zf = 10;

    const float atf = 1/(aspect-tanf(fov/2));
    const float tf = 1/tanf(fov/2);
    const float fnnf = (zf+zn)/(zn-zf);
    const float fnnf2 = ((2*zf)*zn)/(zn-zf);

    CNFGSetup("3D Renderer", (int)width, (int)height);

    float lastframetime = 0;

    while(CNFGHandleInput()) {
        clock_t startofframe = clock();

        CNFGClearFrame();
        CNFGColor(0xffffffff);

        // Rotate the cube around the y axis 90 degrees per second
        // Time is tracked by measuring how long the last frame was
        // Will not rotate when the window is being moved
        for(int i = 0; i < 24; i += 3) {
            float nvtex[3];
            roty(&(vtexs[i]), &(nvtex[0]), lastframetime*(90*DEG2RAD));
            vtexs[i] = nvtex[0];
            vtexs[i+1] = nvtex[1];
            vtexs[i+2] = nvtex[2];
        }

        // Compute pixel coordinates of the points and draw lines
        for(int i = 0; i < 36; i += 3) {
            int xps[3];
            int yps[3];

            for(int j = 0; j < 3; j++) {
                float vtexx = vtexs[((faces[i + j]) * 3)];
                float vtexy = vtexs[((faces[i + j]) * 3) + 1];
                float vtexz = vtexs[((faces[i + j]) * 3) + 2];
                float w = 1;

                vtexx *= atf;
                vtexy *= tf;
                vtexz = (vtexz * fnnf) + (w * -1);
                w = (vtexs[((faces[i + j]) * 3) + 2]) * fnnf2;

                float xndc = vtexx/w;
                float yndc = vtexy/w;

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

        clock_t endofframe = clock();
        lastframetime = ((float)(endofframe - startofframe)) / ((float)CLOCKS_PER_SEC);
    }

    return 0;
}
