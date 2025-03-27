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

typedef struct vec3 {
    float x;
    float y;
    float z;
} vec3;

// Functions for manipulating 3D points

// Rotate a vector around the x axis by specified radians
vec3 rotx(vec3 in, float rad) {
    vec3 r;
    r.x = in.x;
    r.y = (in.y * cosf(rad)) + (in.z * (-(sinf(rad))));
    r.z = (in.y * sinf(rad)) + (in.z * cosf(rad));
    return r;
}

// Rotate a vector around the y axis by specified radians
vec3 roty(vec3 in, float rad) {
    vec3 r;
    r.x = (in.x * cosf(rad)) + (in.z * (-(sinf(rad))));
    r.y = in.y;
    r.z = (in.x * sinf(rad)) + (in.z * cosf(rad));
    return r;
}

// Rotate a vector around the z axis by specified radians
vec3 rotz(vec3 in, float rad) {
    vec3 r;
    r.x = (in.x * cosf(rad)) + (in.y * (-(sinf(rad))));
    r.y = (in.x * sinf(rad)) + (in.y * cosf(rad));
    r.z = in.z;
    return r;
}

// Normalize a vector
vec3 nor(vec3 in) {
    vec3 r = in;
    float l = sqrtf((in.x * in.x) + (in.y * in.y) + (in.z + in.z));
    if(l != 0) {
        float il = 1/l;
        r.x *= il;
        r.y *= il;
        r.z *= il;
    }
    return r;
}


// Functions for reading models

// Read an OBJ model and load it into memory
// Currently lacks support for anything but vertexes and faces
// Faces do not support texture coordinates and normal indices
// Does not support quad faces yet
// User has to free vertex and face arrays themselves
int load_obj(const char* filename, vec3* vtexs, int* ovtexamt, int* faces, int* ofaceamt) {
    FILE* obj = fopen(filename, "rb");
    if(obj == NULL) {
        fprintf(stderr, "Failed to open file %s!\n", filename);
        return 0;
    }

    fseek(obj, 0, SEEK_END);
    long objsize = ftell(obj);
    fseek(obj, 0, SEEK_SET);

    int type;
    int vtexamt = 0;
    int faceamt = 0;

    while(ftell(obj) < objsize) {
        type = fgetc(obj);
        switch(type) {
            case 'v':
            {
                fseek(obj, 1, SEEK_CUR);

                vtexamt++;
                vtexs = (vec3*)realloc(vtexs, vtexamt * sizeof(vec3));

                char buf[100];
                char* bufptr = buf;
                for(int i = 0; i < 3; i++) {
                    while(1) {
                        int c = fgetc(obj);
                        printf("v%d: %c\n", vtexamt, (char)c);
                        *bufptr++ = (char)c;
                        if((c == ' ') || (c == '\n')) {
                            *bufptr = '\0';
                            break;
                        }
                    }
                    float v = (float)atof(buf);
                    switch(i) {
                        case 0: vtexs[vtexamt - 1].x = v; break;
                        case 1: vtexs[vtexamt - 1].y = v; break;
                        case 2: vtexs[vtexamt - 1].z = v; break;
                    }
                    bufptr = buf;
                }
                break;
            }
            case 'f':
            {
                fseek(obj, 1, SEEK_CUR);

                faceamt++;
                faces = (int*)realloc(faces, (faceamt * 3) * sizeof(int));

                char buf[100];
                char* bufptr = buf;
                for(int i = 0; i < 3; i++) {
                    while(1) {
                        int c = fgetc(obj);
                        printf("f%d: %c\n", faceamt, (char)c);
                        *bufptr++ = (char)c;
                        if((c == ' ') || (c == '\n') || (c == EOF)) {
                            *bufptr = '\0';
                            break;
                        }
                    }
                    int v = atoi(buf);
                    faces[((faceamt - 1) * 3) + i] = v;
                    bufptr = buf;
                }
                break;
            }
            case '\n': continue;
            default:
            {
                fprintf(stderr, "Unknown type: %c\n", (char)type);
                fclose(obj);
                *ovtexamt = 0;
                *ofaceamt = 0;
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
    vec3* vtexs = NULL;
    int vtexamt;

    int* faces = NULL;
    int faceamt;
    
    const vec3 modelpos = {0, 0, 3}; // Making it a constant for now

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
        for(int i = 0; i < vtexamt; i++) {
            vtexs[i] = roty(vtexs[i], delta*(90*DEG2RAD));
        }

        // Compute pixel coordinates of the points and draw lines
        for(int i = 0; i < faceamt * 3; i += 3) {
            int xps[3];
            int yps[3];

            for(int j = 0; j < 3; j++) {
                float vtexx = vtexs[faces[i + j]].x + modelpos.x;
                float vtexy = vtexs[faces[i + j]].y + modelpos.y;
                float vtexz = vtexs[faces[i + j]].z + modelpos.z;
                float w = 1;

                vtexx *= atf;
                vtexy *= tf;
                vtexz = (vtexz * fnnf) + (w * -1);
                w = (vtexs[faces[i + j]].z + modelpos.z) * fnnf2;

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
