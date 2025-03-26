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
const float fov = 45*DEG2RAD;


// Points for a test cube
const float vtexs[24] = {1, 1, 1,  1, -1, 1,  -1, 1, 1,  -1, -1, 1,  1, 1, -1,  1, -1, -1,  -1, 1, -1,  -1, -1,-1};
const int faces[36] = {0, 1, 5,  0, 5, 4,  4, 5, 7,  4, 6, 5,  6, 7, 3,  6, 2, 3,  2, 3, 1,  2, 0, 1,  2, 4, 6,  2, 0, 6,  1, 3, 5,  3, 5, 7};


int main(int argc, char **argv) {
    float zoff = 3;
    if(argc == 2) {
        zoff = (float)atof(argv[1]); 
    }
    /*
    const float width = (float)atoi(argv[1]); 
    const float height = (float)atoi(argv[2]);
    */
    const float width = 512;
    const float height = 512;

    const float aspect = width/height; // 1

    const float zoff = 3;

    const float zn = 1;
    const float zf = 10;

    const float atf = 1/(aspect-tanf(fov/2));
    const float tf = 1/tanf(fov/2);
    const float fnnf = (zf+zn)/(zn-zf);
    const float fnnf2 = ((2*zf)*zn)/(zn-zf);

    CNFGSetup("3D renderer", (int)width, (int)height);

    while(CNFGHandleInput()) {
        CNFGClearFrame();
        CNFGColor(0xffffffff);

        // Compute pixel coordinates of the points and draw lines
        for(int i = 0; i < 36; i += 3) {
            int xps[3];
            int yps[3];

            for(int j = 0; j < 3; j++) {
                float vtexx = vtexs[((faces[i + j]) * 3)];
                float vtexy = vtexs[((faces[i + j]) * 3) + 1];
                float vtexz = vtexs[((faces[i + j]) * 3) + 2] + zoff;
                float w = 1;

                vtexx *= atf;
                vtexy *= tf;
                vtexz = (vtexz * fnnf) + (w * -1);
                w = (vtexs[((faces[i + j]) * 3) + 2] + zoff) * fnnf2;

                float xndc = vtexx/w;
                float yndc = vtexy/w;

                int xp = (int)(((xndc + 1) * width) / 2);
                int yp = (int)(((1 - yndc) * height) / 2);

                printf("Face %d Vertex %d\nxp: %d\nyp: %d\n\n", (i / 3) + 1, j + 1, xp, yp);

                xps[j] = xp;
                yps[j] = yp;
            }

            CNFGTackSegment(xps[0], yps[0], xps[1], yps[1]);
            CNFGTackSegment(xps[1], yps[1], xps[2], yps[2]);
            CNFGTackSegment(xps[2], yps[2], xps[0], yps[0]);
        }
        /*
        for(int i = 0; i < 24; i += 3) {
            float vtexx = vtexs[i];
            float vtexy = vtexs[i + 1];
            float vtexz = vtexs[i + 2] + 10;
            float w = 1;

            vtexx *= atf;
            vtexy *= tf;
            vtexz = (vtexz * fnnf) + (w * -1);
            w = (vtexs[i + 2] + 10) * fnnf2;

            float xndc = vtexx/w;
            float yndc = vtexy/w;

            int xp = (int)(((xndc + 1) * width) / 2);
            int yp = (int)(((1 - yndc) * height) / 2);

            //printf("Vertex %d\nxndc: %f\nyndc: %f\nxp: %d\nyp: %d\n\n", (i / 3) + 1, xndc, yndc, xp, yp);

            //vtexsp[i / 3] = xp;
            //vtexsp[(i / 3) + 1] = yp;
            CNFGTackPixel(xp, yp);
        }
        */

        /*
        for(int i = 0; i < 8; i++) {
            CNFGTackSegment(vtexsp[i * 2], vtexsp[(i * 2) + 1], vtexsp[(vtexscon[i * 3]) * 2], vtexsp[((vtexscon[i * 3]) * 2) + 1]);
            CNFGTackSegment(vtexsp[i * 2], vtexsp[(i * 2) + 1], vtexsp[(vtexscon[(i * 3) + 1]) * 2], vtexsp[((vtexscon[(i * 3) + 1]) * 2) + 1]);
            CNFGTackSegment(vtexsp[i * 2], vtexsp[(i * 2) + 1], vtexsp[(vtexscon[(i * 3) + 2]) * 2], vtexsp[((vtexscon[(i * 3) + 2]) * 2) + 1]);
            
            CNFGTackPixel(vtexsp[i * 2], vtexsp[(i * 2) + 1]);
        }
        */

        CNFGSwapBuffers();
    }

    return 0;
}