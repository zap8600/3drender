#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

// printf("\x1b[2J");

#define CNFG_IMPLEMENTATION
#include "rawdraw_sf.h"

void HandleKey( int keycode, int bDown ) { }
void HandleButton( int x, int y, int button, int bDown ) { }
void HandleMotion( int x, int y, int mask ) { }
int HandleDestroy() { return 0; }

#define PI 3.14159265358979323846f
#define DEG2RAD (PI/180.0f)

// 60 degrees
const float fov = 60*DEG2RAD;

// Points for a test cube
const float vtexs[] = {1, 1, 6,  1, -1, 6,  -1, 1, 6,  -1, -1, 6,  1, 1, 4,  1, -1, 4,  -1, 1, 4,  -1, -1, 4};
int vtexsp[((sizeof(vtexs) / sizeof(float)) / 3) * 2];

// char output[1760];

int main() {
    /*
    const float width = (float)atoi(argv[1]); 
    const float height = (float)atoi(argv[2]);
    */
    const float width = 1024;
    const float height = 768;

    const float aspect = width/height;

    const float zn = 0.1;
    const float zf = 100;

    const float atf = 1/(atf-tanf(fov/2));
    const float tf = 1/tanf(fov/2);
    const float fnnf = (zf+zn)/(zn-zf);
    const float fnnf2 = (2*zf*zn)/(zn-zf);

    CNFGSetup("3D renderer", 1024, 768);

    while(CNFGHandleInput()) {
        CNFGClearFrame();
        CNFGColor(0xffffffff);

        // Compute pixel coordinates of the points
        for(int i = 0; i < (sizeof(vtexs) / sizeof(float)); i += 3) {
            float vtexx = vtexs[i];
            float vtexy = vtexs[i + 1];
            float vtexz = vtexs[i + 2];
            float w = 1;

            vtexx *= atf;
            vtexy *= tf;
            vtexy = (vtexz * fnnf) + (w * -1);
            w = vtexs[i + 2] * fnnf2;

            float xndc = vtexx/w;
            float yndc = vtexy/w;

            int xp = (int)(((xndc + 1)/2)*width);
            int yp = (int)(height-(((yndc + 1)/2)*height));
            printf("Vertex %d\nxp: %d\nyp: %d\n\n", i + 1, xp, yp);

            CNFGTackPixel(xp, yp);
        }

        CNFGSwapBuffers();
    }

    return 0;
}