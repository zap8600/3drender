#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define CNFG_IMPLEMENTATION
#include "rawdraw_sf.h"

void HandleKey(int keycode, int bDown) {}
void HandleButton(int x, int y, int button, int bDown) {}
// Not sure how I use this to get mouse motion but I'll try
void HandleMotion( int x, int y, int mask ) {
    printf("%d %d %x\n", x, y, mask);
}
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

typedef vec3 vtex;

typedef struct face {
    int* vtexs;
    int vtexamt;
} face;

// Functions for manipulating 3D points

// Add one vecter to another
vec3 add(vec3 v1, vec3 v2) {
    vec3 r = {v1.x + v2.x, v1.y + v2.y, v1.z + v2.z};
    return r;
}

// Subtract one vector by another
vec3 sub(vec3 v1, vec3 v2) {
    vec3 r = {v1.x - v2.x, v1.y - v2.y, v1.z - v2.z};
    return r;
}

// Rotate a vector around the x axis by specified radians
vec3 rotx(vec3 in, float rad) {
    vec3 r;
    r.x = in.x;
    r.y = (in.y * cosf(rad)) + (in.z * (-(sinf(rad))));
    r.z = (in.y * sinf(rad)) + (in.z * cosf(rad));
    return r;
}

// Rotate a vector around the y axis by specified radians
vec3 roty(vec3 in, float rad) {;
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
vec3 norm(vec3 in) {
    vec3 r = in;
    float l = sqrtf((in.x * in.x) + (in.y * in.y) + (in.z * in.z));
    if(l != 0) {
        float il = 1/l;
        r.x *= il;
        r.y *= il;
        r.z *= il;
    }
    return r;
}

// Get the cross product of 2 vectors
vec3 cross(vec3 v1, vec3 v2) {
    vec3 r = {(v1.y * v2.z) - (v1.z * v2.y), (v1.z * v2.x) - (v1.x * v2.z), (v1.x * v2.y) - (v1.y * v2.x)};
    return r;
}

// Get the dot product of 2 vectors
float dot(vec3 v1, vec3 v2) {
    float r = ((v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z));
    return r;
}


// Functions for reading models

// Read an OBJ model and load it into memory
// User has to free vertex and face arrays themselves
int load_obj(const char* filename, vec3** vtexs, int* ovtexamt, face** faces, int* ofaceamt) {
    FILE* obj = fopen(filename, "rb");
    if(obj == NULL) {
        fprintf(stderr, "Failed to open file %s!\n", filename);
        return 0;
    }

    int type;
    int vtexamt = 0;
    int faceamt = 0;

    while((type = fgetc(obj)) != EOF) {
        switch(type) {
            case 'v':
            {
                type = fgetc(obj);
                switch(type) {
                    case ' ': {
                        vtexamt++;
                        (*vtexs) = (vec3*)realloc((*vtexs), vtexamt * sizeof(vec3));

                        char buf[100];
                        for(int i = 0; i < 3; i++) {
                            int counter = 0;
                            while(1) {
                                int c = fgetc(obj);
                                buf[counter++] = (char)c;
                                if((c == ' ') || (c == '\n')) {
                                    buf[counter] = '\0';
                                    break;
                                }
                            }
                            float v = (float)atof(buf);
                            switch(i) {
                                case 0: (*vtexs)[vtexamt - 1].x = v; break;
                                case 1: (*vtexs)[vtexamt - 1].y = v; break;
                                case 2: (*vtexs)[vtexamt - 1].z = v; break;
                            }
                        }
                        break;
                    }
                    case 'n':
                    case 't': {
                        int c;
                        while((c = fgetc(obj)) != '\n') {}
                        break;
                    }
                    default:
                    {
                        fprintf(stderr, "Unknown type: v%c\n", (char)type);
                        fclose(obj);
                        *ovtexamt = 0;
                        *ofaceamt = 0;
                        return 0;
                    }
                }
                break;
            }
            case 'f':
            {
                fseek(obj, 1, SEEK_CUR);

                faceamt++;
                (*faces) = (face*)realloc((*faces), faceamt * sizeof(face));

                char buf[100];
                int fvtexamt = 0;
                bool stillvtex = true;
                while(stillvtex) {
                    int counter = 0;
                    while(1) {
                        int c = fgetc(obj);
                        buf[counter++] = (char)c;
                        if(c == ' ') {
                            buf[counter] = '\0';
                            break;
                        } else if((c == '\n') || (c == EOF)) {
                            stillvtex = false;
                            buf[counter] = '\0';
                            break;
                        }
                    }
                    fvtexamt++;
                    (*faces)[faceamt - 1].vtexs = (int*)realloc((*faces)[faceamt - 1].vtexs, fvtexamt * sizeof(int));
                    int v = atoi(buf) - 1; // We need to subtract one because the obj vertex arra starts at 1 instead of 0
                    if(v >= vtexamt) {
                        fprintf(stderr, "vtexamt is %d, but index is %d\n", vtexamt, v);
                    }
                    (*faces)[faceamt - 1].vtexs[fvtexamt - 1] = v;
                }
                (*faces)[faceamt - 1].vtexamt = fvtexamt;
                break;
            }
            case '\n': continue;
            case '#':
            {
                int c;
                while((c = fgetc(obj)) != '\n') {}
                break;
            }
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

    fclose(obj);

    printf("%d faces, %d vtexs\n", vtexamt, faceamt);

    *ovtexamt = vtexamt;
    *ofaceamt = faceamt;

    return 1;
}


const vec3 up = {0, 1, 0};
const vec3 realmodelpos = {0, 0, 3};
vec3 camerapos = {0, 4, 0};

int main(int argc, char **argv) {
    if(argc != 2) {
        fprintf(stderr, "Usage: %s [obj model file path]\n", argv[0]);
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
    
    vec3 modelrotpos = {0, 0, 0};

    load_obj(argv[1], &vtexs, &vtexamt, &faces, &faceamt); // TODO: return an error if this fails
    printf("%d faces\n", faceamt);

    CNFGSetup("3D Renderer", (int)width, (int)height);

    clock_t lasttime = clock();

    while(CNFGHandleInput()) {
        CNFGClearFrame();
        CNFGColor(0xffffffff);

        clock_t now = clock();
        float delta = ((float)(now - lasttime)) / CLOCKS_PER_SEC;
        lasttime = now;

        // Rotate the object around the y axis in a circle -90 degrees per second
        // modelrotpos = roty(modelrotpos, delta*(-0.5*PI));

        // Rotate the model around the y axis 90 degrees per second
        // Time is tracked by measuring how long the last frame was
        for(int i = 0; i < vtexamt; i++) {
            vtexs[i] = roty(vtexs[i], delta*(0.5*PI));
        }

        // Compute pixel coordinates of the points and draw lines
        for(int i = 0; i < faceamt; i++) {
            //printf("Face %d has %d vtexs\n", i + 1, faces[i].vtexamt);
            int* xps = (int*)malloc(faces[i].vtexamt * sizeof(int));
            int* yps = (int*)malloc(faces[i].vtexamt * sizeof(int));

            for(int j = 0; j < faces[i].vtexamt; j++) {
                vec3 cvtex = add(add(vtexs[faces[i].vtexs[j]], modelrotpos), realmodelpos);
                float vtexx = cvtex.x;
                float vtexy = cvtex.y;
                float vtexz = cvtex.z;
                float w = 1;

                vec3 za = norm(sub(camerapos, add(modelrotpos, realmodelpos)));
                vec3 xa = norm(cross(up, za));
                vec3 ya = cross(za, xa);

                // Convert from world space to camera space
                vtexx = (vtexx * xa.x) + (vtexy * xa.y) + (vtexz * xa.z) + (w * (-(dot(xa, camerapos))));
                vtexy = (vtexx * ya.x) + (vtexy * ya.y) + (vtexz * ya.z) + (w * (-(dot(ya, camerapos))));
                vtexz = (vtexx * za.x) + (vtexy * za.y) + (vtexz * za.z) + (w * (-(dot(za, camerapos))));
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

            const int fvtexamt = faces[i].vtexamt;
            for(int j = 0; j < fvtexamt; j++) {
                //CNFGTackPixel(xps[j], yps[j]);
                CNFGTackSegment(xps[j], yps[j], xps[(j + 1) % fvtexamt], yps[(j + 1) % fvtexamt]);
            }

            free(xps);
            free(yps);
        }

        CNFGSwapBuffers();
    }

    free(vtexs);
    for(int i = 0; i < faceamt; i++) {
        free(faces[i].vtexs);
    }
    free(faces);

    return 0;
}
