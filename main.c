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

// 45 degrees
const float fov = 60*DEG2RAD;

// Points for a test cube
const float vtexs[24] = {1, 1, 1,  1, -1, 1,  -1, 1, 1,  -1, -1, 1,  1, 1, -1,  1, -1, -1,  -1, 1, -1,  -1, -1,-1};
const int vtexscon[24] = {1, 2, 4,  0, 3, 5,  0, 3, 6,  2, 1, 7,  0, 5, 6,  4, 1, 7,  2, 7, 4,  3, 6, 5};
int vtexsp[16];

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
    const float fnnf2 = ((2*zf)*zn)/(zn-zf);

    CNFGSetup("3D renderer", 1024, 768);

    while(CNFGHandleInput()) {
        CNFGClearFrame();
        CNFGColor(0xffffffff);

        // Compute pixel coordinates of the points
        for(int i = 0; i < 24; i += 3) {
            float vtexx = vtexs[i];
            float vtexy = vtexs[i + 1];
            float vtexz = vtexs[i + 2];
            float w = 1;

            vtexx *= atf;
            vtexy *= tf;
            vtexz = (vtexz * fnnf) + (w * -1);
            w = vtexs[i + 2] * fnnf2;

            float xndc = vtexx/w;
            float yndc = vtexy/w;

            int xp = (int)(((xndc + 1) * width) / 2);
            int yp = (int)(((1 - yndc) * height) / 2);

            printf("Vertex %d\nxndc: %f\nyndc: %f\nxp: %d\nyp: %d\n\n", (i / 3) + 1, xndc, yndc, xp, yp);

            vtexsp[i / 3] = xp;
            vtexsp[(i / 3) + 1] = yp;
        }

        for(int i = 0; i < 8; i++) {
            CNFGTackSegment(vtexsp[i * 2], vtexsp[(i * 2) + 1], vtexsp[(vtexscon[i * 3]) * 2], vtexsp[((vtexscon[i * 3]) * 2) + 1]);
            CNFGTackSegment(vtexsp[i * 2], vtexsp[(i * 2) + 1], vtexsp[(vtexscon[i * 3] + 1) * 2], vtexsp[((vtexscon[i * 3] + 1) * 2) + 1]);
            CNFGTackSegment(vtexsp[i * 2], vtexsp[(i * 2) + 1], vtexsp[(vtexscon[i * 3] + 2) * 2], vtexsp[((vtexscon[i * 3] + 2) * 2) + 1]);
        }

        CNFGSwapBuffers();
    }

    return 0;
}