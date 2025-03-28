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

typedef struct vtex {
    vec3 pos;
} vtex;

typedef struct vtex nrmi;

typedef struct tcrd {
    float u;
    float v;
} tcrd;

typedef struct face {
    int vtexs[3];
    int nrmis[3];
    int hasnrmis;
    int tcrds[3];
    int hastcrds
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
// Texture coordinates and normal indices are read but not loaded and ignored in faces for now
// Does not support quad faces yet
// User has to free vertex and face arrays themselves
int load_obj(const char* filename, vtex** vtexs, int* ovtexamt, face** faces, int* ofaceamt, tcrd** tcrds, int* otcrdamt, nrmi** nrmis, int* onrmiamt) {
    FILE* obj = fopen(filename, "r");
    if(obj == NULL) {
        fprintf(stderr, "Failed to open file %s!\n", filename);
        return 0;
    }

    int type;
    int vtexamt = 0;
    int faceamt = 0;
    int tcrdamt = 0;
    int nrmiamt = 0;

    while(!feof(obj)) {
        char line[500];
        memset(line, 0, 500);
        fgets(line, 500, obj);
        if(line[strlen(line)] == '\n') line[strlen(line)] = ' ';
        
        switch(line[0]) {
            case 'v':
            {
                switch(line[1]) {
                    case ' ':
                    {
                        vtexamt++;
                        (*vtexs) = (vtex*)realloc((*vtexs), vtexamt * sizeof(vtex));
                        char* curpar = line + 2;
                        for(int i = 0; i < 3; i++) {
                            char* eo = strchr(curpar, ' ');
                            *eo = '\0';
                            float v = (float)atof(curpar);
                            switch(i) {
                                case 0: (*vtexs)[vtexamt - 1].pos.x = v; break;
                                case 1: (*vtexs)[vtexamt - 1].pos.y = v; break;
                                case 2: (*vtexs)[vtexamt - 1].pos.z = v; break;
                            }
                            curpar = eo + 1;
                        }
                        break;
                    }
                    case 't':
                    {
                        tcrdamt++;
                        (*tcrds) = (tcrd*)realloc((*tcrds), tcrdamt * sizeof(tcrd));
                        char* curpar = line + 3;
                        for(int i = 0; i < 2; i++) { // TODO: Handle optional W value
                            char* eo = strchr(curpar, ' ');
                            *eo = '\0';
                            float v = (float)atof(curpar);
                            switch(i) {
                                case 0: (*tcrds)[tcrdamt - 1].u = v; break;
                                case 1: (*tcrds)[tcrdamt - 1].v = v; break;
                            }
                            curpar = eo + 1;
                        }
                        break;
                    }
                    case 'n':
                    {
                        nrmiamt++;
                        (*nrmis) = (nrmi*)realloc((*nrmis), nrmiamt * sizeof(nrmi));
                        char* curpar = line + 3;
                        for(int i = 0; i < 3; i++) {
                            char* eo = strchr(curpar, ' ');
                            *eo = '\0';
                            float v = (float)atof(curpar);
                            switch(i) {
                                case 0: (*nrmis)[nrmiamt - 1].pos.x = v; break;
                                case 1: (*nrmis)[nrmiamt - 1].pos.y = v; break;
                                case 2: (*nrmis)[nrmiamt - 1].pos.z = v; break;
                            }
                            curpar = eo + 1;
                        }
                        break;
                    }
                    default: continue; // TODO: Handle loading errors
                }
                break;
            }
            case 'f':
            {
                faceamt++;
                (*faces) = (face*)realloc((*faces), faceamt * sizeof(face));
                char* curpar = line + 2;
                char* tfv = strchr(curpar, '/');
                int hasnrmis = 0;
                int hastcrds = 0;
                if(tfv != NULL) { // Check for optional values
                    if(strchr(tfv + 1, '/') < strchr(tfv + 1, ' ')) {
                        if(tfv[1] == '/') {
                            hasnrmis = 1;
                            hastcrds = 0;
                        } else {
                            hasnrmis = 1;
                            hastcrds = 1;
                        }
                    } else {
                        hasnrmis = 0;
                        hastcrds = 1;
                    }
                } else {
                    hasnrmis = 0;
                    hastcrds = 0;
                }
                for(int i = 0; i < 3; i++) {
                    char* eo = strchr(curpar, (hasnrmis | hastcrds)?'/':' ');
                    *eo = '\0';
                    int v = atoi(curpar) - 1;
                    (*faces)[faceamt - 1].vtexs[i] = v;
                    curpar = eo + 1;
                    if(hastcrds) {
                        v = atoi(*curpar);
                    }
                }
            }
            default: continue; // TODO: Handle loading errors
        }
    }

    return 1;
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

    int* faces = NULL;
    int faceamt;
    
    vec3 modelrotpos = {0, 0, 2};

    load_obj(argv[1], &vtexs, &vtexamt, &faces, &faceamt); // TODO: return an error if this fails

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
