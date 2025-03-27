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


// Functions for reading models

// Read an OBJ model and load it into memory
// Currently lacks support for anything but vertexes and faces
// Faces do not support texture coordinates and normal indices
// Does not support quad faces yet
// User has to free vertex and face arrays themselves
int load_obj(const char* filename, float* vtexs, int* ovtexamt, int* faces, int* ofaceamt) {
    FILE* obj = fopen(filename, "rb");
    if(obj == NULL) {
        fprintf(stderr, "Failed to open file %s!\n", filename);
        return 0;
    }

    *vtexamt = 0;
    *faceamt = 0;

    int type;
    int vtexamt = 0;
    int faceamt = 0;

    while((type = fgetc(obj)) != EOF) {
        switch(type) {
            case 'v':
            {
                vtexamt++;
                vtexs = (float*)realloc(vtexs, (vtexamt * 3) * sizeof(float));

                char buf[100]; // TODO: make it dynamic
                char *bufptr = buf;
                int pr = 0;
                while(1) {
                    int c = fgetc(obj);
                    *bufptr++ = (char)c;
                    if((c == ' ') || (c == '\n') || (c == EOF)) { // TODO: do something in case we hit an EOF and we're not done reading the vertex
                        *bufptr == '\0';
                        vtexs[(vtexamt * 3) + (pr++)] = (float)atof(buf);
                        if(pr == 3) {
                            break;
                        } else {
                            continue;
                        }
                    } else {
                        continue;
                    }
                }
                break;
            }
            case 'f':
            {
                faceamt++;
                faces = (int*)realloc(faces, (faceamt * 3) * sizeof(int));

                char buf[100]; // TODO: make it dynamic
                char *bufptr = buf;
                int pr = 0;
                while(1) {
                    int c = fgetc(obj);
                    *bufptr++ = (char)c;
                    if((c == ' ') || (c == '\n') || (c == EOF)) { // TODO: do something in case we hit an EOF and we're not done reading the face
                        *bufptr == '\0';
                        faces[(faceamt * 3) + (pr++)] = atoi(buf);
                        if(pr == 3) {
                            break;
                        } else {
                            continue;
                        }
                    } else {
                        continue;
                    }
                }
                break;
            }
            case '\n': continue;
            default:
            {
                fprintf(stderr, "Unknown type: %c\n", (char)type);
                fclose(obj);
                return 0;
            }
        }
    }
    *ovtexamt = vtexamt;
    *ofaceamt = faceamt;

    return 1;
}


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

    const float atf = 1/(aspect-tanf(fov/2));
    const float tf = 1/tanf(fov/2);
    const float fnnf = (zf+zn)/(zn-zf);
    const float fnnf2 = ((2*zf)*zn)/(zn-zf);

    // Model loading
    float* vtexs = NULL;
    int vtexamt;

    int* faces = NULL;
    int faceamt;
    
    const float modelpos[3] = {0, 0, 3}; // Making it a constant for now

    load_obj(argv[1], vtexs, &vtexamt, faces, &faceamt); // TODO: return an error if this fails

    CNFGSetup("3D Renderer", (int)width, (int)height);

    clock_t lasttime = clock();

    while(CNFGHandleInput()) {
        CNFGClearFrame();
        CNFGColor(0xffffffff);

        clock_t now = clock();
        float delta = ((float)(now - lasttime)) / CLOCKS_PER_SEC;
        lasttime = now;

        // Rotate the cube around the y axis 90 degrees per second
        // Time is tracked by measuring how long the last frame was
        // Will not rotate when the window is being moved
        for(int i = 0; i < vtexamt * 3; i += 3) {
            float nvtex[3];
            roty(&(vtexs[i]), nvtex, delta*(90*DEG2RAD));
            vtexs[i] = nvtex[0];
            vtexs[i+1] = nvtex[1];
            vtexs[i+2] = nvtex[2];
        }

        // Compute pixel coordinates of the points and draw lines
        for(int i = 0; i < faceamt * 3; i += 3) {
            int xps[3];
            int yps[3];

            for(int j = 0; j < 3; j++) {
                float vtexx = vtexs[((faces[i + j]) * 3)] + modelpos[0];
                float vtexy = vtexs[((faces[i + j]) * 3) + 1] + modelpos[1];
                float vtexz = vtexs[((faces[i + j]) * 3) + 2] + modelpos[2];
                float w = 1;

                vtexx *= atf;
                vtexy *= tf;
                vtexz = (vtexz * fnnf) + (w * -1);
                w = (vtexs[((faces[i + j]) * 3) + 2] + modelpos[2]) * fnnf2;

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
    }

    free(vtexs);
    free(faces);

    return 0;
}
