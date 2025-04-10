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

typedef vtex nrmi;

typedef struct tcrd {
    float u;
    float v;
    float w;
} tcrd;

typedef struct face {
    int vtexs[3];
    int tcrds[3];
    int nrmis[3];
    bool hastcrds;
    bool hasnrmis;
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

// Scale a vector
vec3 scale(vec3 v, float s) {
    vec3 r = {v.x * s, v.y * s, v.z * s};
    return r;
}

// Rotate a vector around an axis
vec3 rotb(vec3 v, vec3 axis, float rad) {
    vec3 r = v;
    axis = norm(axis);

    rad /= 2;
    float a = sinf(rad);
    float b = axis.x * a;
    float c = axis.y * a;
    float d = axis.z * a;
    a = cosf(rad);
    vec3 w = {b, c, d};

    vec3 wv = cross(w, v);

    vec3 wwv = cross(w, wv);

    wv = scale(wv, 2 * a);

    wwv = scale(wwv, 2);

    r = add(r, wv);
    r = add(r, wwv);

    return r;
}


// Functions for reading models

// Read an OBJ model and load it into memory
// User has to free vertex and face arrays themselves
int load_obj(const char* filename, vtex** vtexs, int* ovtexamt, face** faces, int* ofaceamt, tcrd** tcrds, int* otcrdamt, nrmi** nrmis, int* onrmiamt) {
    FILE* obj = fopen(filename, "rb");
    if(obj == NULL) {
        fprintf(stderr, "Failed to open file %s!\n", filename);
        return 0;
    }

    int type;
    int vtexamt = 0;
    int faceamt = 0;
    int tcrdamt = 0;
    int nrmiamt = 0;

    while((type = fgetc(obj)) != EOF) {
        switch(type) {
            case 'v':
            {
                type = fgetc(obj);
                switch(type) {
                    case ' ':
                    {
                        vtexamt++;
                        (*vtexs) = (vtex*)realloc((*vtexs), vtexamt * sizeof(vtex));

                        char buf[100];
                        char* bufptr = buf;
                        for(int i = 0; i < 3; i++) {
                            while(1) {
                                int c = fgetc(obj);
                                *bufptr++ = (char)c;
                                if((c == ' ') || (c == '\n')) {
                                    *bufptr = '\0';
                                    break;
                                }
                            }
                            float v = (float)atof(buf);
                            switch(i) {
                                case 0: (*vtexs)[vtexamt - 1].x = v; break;
                                case 1: (*vtexs)[vtexamt - 1].y = v; break;
                                case 2: (*vtexs)[vtexamt - 1].z = v; break;
                            }
                            bufptr = buf;
                        }
                        break;
                    }
                    case 't':
                    {
                        fseek(obj, 1, SEEK_CUR);

                        tcrdamt++;
                        (*tcrds) = (tcrd*)realloc((*tcrds), tcrdamt * sizeof(tcrd));

                        char buf[100];
                        char* bufptr = buf;
                        int vamts = 0;
                        bool stillvs = true;
                        while(stillvs) {
                            while(1) {
                                int c = fgetc(obj);
                                *bufptr++ = (char)c;
                                if(c == ' ') {
                                    *bufptr = '\0';
                                    break;
                                } else if(c == '\n') {
                                    stillvs = false;
                                    *bufptr = '\0';
                                    break;
                                }
                            }
                            vamts++;
                            float v = (float)atof(buf);
                            switch(vamts) { // TODO: Throw error if it exceeds 3
                                case 1: (*tcrds)[tcrdamt - 1].u = v; break;
                                case 2: (*tcrds)[tcrdamt - 1].v = v; break;
                                case 3: (*tcrds)[tcrdamt - 1].w = v; break;
                            }
                            bufptr = buf;
                        }
                        break;
                    }
                    case 'n':
                    {
                        fseek(obj, 1, SEEK_CUR);

                        nrmiamt++;
                        (*nrmis) = (nrmi*)realloc((*nrmis), nrmiamt * sizeof(nrmi));

                        char buf[100];
                        char* bufptr = buf;
                        for(int i = 0; i < 3; i++) {
                            while(1) {
                                int c = fgetc(obj);
                                *bufptr++ = (char)c;
                                if((c == ' ') || (c == '\n')) {
                                    *bufptr = '\0';
                                    break;
                                }
                            }
                            float v = (float)atof(buf);
                            switch(i) {
                                case 0: (*nrmis)[nrmiamt - 1].x = v; break;
                                case 1: (*nrmis)[nrmiamt - 1].y = v; break;
                                case 2: (*nrmis)[nrmiamt - 1].z = v; break;
                            }
                            bufptr = buf;
                        }
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

                int fvtexamt = 0; // TODO: ensure these values are the same
                int ftcrdamt = 0;
                int fnrmiamt = 0;

                int* fvtexs = NULL;
                int* ftcrds = NULL;
                int* fnrmis = NULL;

                char buf[100];
                char* bufptr = buf;
                bool stillvtex = true;
                bool ftcrd = false;
                bool fnrmi = false;
                bool notcrd = false;
                unsigned long ftcrdpos = 0;
                while(stillvtex) {
                    while(1) {
                        int c = fgetc(obj);
                        *bufptr++ = (char)c;
                        if(c == ' ') {
                            *bufptr = '\0';
                            break;
                        } else if(c == '/') {
                            if(!ftcrd) {
                                ftcrd = true;
                            }
                            tcrdpos = ftell(obj); 
                            *bufptr = '\0';
                            break;
                        } else if((c == '\n') || (c == EOF)) {
                            if(!ftcrd) {
                                stillvtex = false;
                            }
                            *bufptr = '\0';
                            break;
                        }
                    }
                    fvtexamt++;
                    fvtexs = (int*)realloc(fvtexs, fvtexamt * sizeof(int));
                    int v = atoi(buf) - 1; // We need to subtract one because the obj vertex arra starts at 1 instead of 0
                    if(v >= vtexamt) {
                        fprintf(stderr, "vtexamt is %d, but index is %d\n", vtexamt, v);
                    }
                    bufptr = buf;
                    if(ftcrd) {
                        while(1) {
                            int c = fgetc(obj);
                            *bufptr++ = (char)c;
                            if(c == ' ') {
                                *bufptr = '\0';
                                break;
                            } else if(c == '/') {
                                if(!fnrmi) {
                                    if((ftell(obj) - tcrdpos) == 1) {
                                        notcrd = true;
                                    }
                                    fnrmi = true;
                                }
                                *bufptr = '\0';
                                break;
                            } else if((c == '\n') || (c == EOF)) {
                                stillvtex = false;
                                *bufptr = '\0';
                                break;
                            }
                        }
                    }
                }
                (*faces)[faceamt - 1].vtexamt = fvtexamt;
                (*faces)[faceamt - 1].tcrdamt = ftcrdamt;
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

    printf("%d faces, %d vtexs, %d tcrds\n", vtexamt, faceamt, tcrdamt);

    *ovtexamt = vtexamt;
    *ofaceamt = faceamt;
    *otcrdamt = tcrdamt;

    return 1;
}


const vec3 realmodelpos = {0, 0, 3};

vec3 cameraup = {0, 1, 0};
vec3 camerapos = {0, 0, 0};

float width = 512;
float height = 512;

int lastx = 0;
int lasty = 0;
bool holding = false;

void HandleMotion(int x, int y, int mask) {
    if(mask == 1) {
        if(!holding) {
            holding = true;
        } else {
            float dx = ((float)(x - lastx)) / width;
            float dy = ((float)(y - lasty)) / height;

            vec3 right = norm(cross(norm(sub(realmodelpos,camerapos)),norm(cameraup)));

            vec3 tpos = sub(realmodelpos, camerapos);
            tpos = rotb(tpos, norm(cameraup), PI*dx);
            tpos = rotb(tpos, right, PI*dy);
            camerapos = sub(realmodelpos, tpos);
        }

        lastx = x;
        lasty = y;
    } else if(!mask) {
        if(holding) {
            holding = false;
        }
    }
}


int main(int argc, char **argv) {
    if(argc != 2) {
        fprintf(stderr, "Usage: %s [obj model file path]\n", argv[0]);
        return 1;
    }

    // Precomputing math required for rendering

    float aspect = width/height; // 1

    const float zn = 1;
    const float zf = 100;

    const float tof = tanf(fov/2);
    float atf = 1/(aspect*tof);
    const float tf = 1/tof;
    const float fnnf = (zf+zn)/(zn-zf);
    const float fnnf2 = ((2*zf)*zn)/(zn-zf);

    // Model loading
    vtex* vtexs = NULL;
    int vtexamt;

    face* faces = NULL;
    int faceamt;

    tcrd* tcrds = NULL;
    int tcrdamt;

    nrmi* nrmis = NULL;
    int nrmiamt;
    
    vec3 modelrotpos = {0, 0, 0};

    load_obj(argv[1], &vtexs, &vtexamt, &faces, &faceamt, &tcrds, &tcrdamt, &nrmis, &nrmiamt); // TODO: return an error if this fails

    CNFGSetup("3D Renderer", (int)width, (int)height);

    clock_t lasttime = clock();

    while(CNFGHandleInput()) {
        CNFGClearFrame();
        CNFGColor(0xffffffff);
        short w;
        short h;
        CNFGGetDimensions(&w, &h);
        if((width != (float)w) || (height != (float)h)) { // Only recalculate when needed
            width = (float)w;
            height = (float)h;
            aspect = width/height;
            atf = 1/(aspect*tof);
        }

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
                vec3 cvtex = add(vtexs[faces[i].vtexs[j]], realmodelpos);
                float vtexx = cvtex.x;
                float vtexy = cvtex.y;
                float vtexz = cvtex.z;
                float w = 1;

                vec3 za = norm(sub(camerapos, realmodelpos));
                vec3 xa = norm(cross(cameraup, za));
                vec3 ya = cross(za, xa);

                // Convert from world space to camera space
                vtexx = (cvtex.x * xa.x) + (cvtex.y * xa.y) + (cvtex.z * xa.z) + (w * (-(dot(xa, camerapos))));
                vtexy = (cvtex.x * ya.x) + (cvtex.y * ya.y) + (cvtex.z * ya.z) + (w * (-(dot(ya, camerapos))));
                vtexz = (cvtex.x * za.x) + (cvtex.y * za.y) + (cvtex.z * za.z) + (w * (-(dot(za, camerapos))));
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
