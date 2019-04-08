#ifndef __LEVEL_DESC__
#define __LEVEL_DESC__

#include "Math\LinearMath.h"

#pragma pack(16)
/*
    headers

*/
typedef struct LevelHeader {
    int NumEntries;
    int Version;
    int pad;
    int pad2;
}LevelHeader;

/*
    Mesh Entry
*/
typedef struct MeshEntry {
    char Url[256];
}MeshEntry;

/*
    Material Entry
*/
typedef struct MatrialEntry {
    char Url[256];
}MatrialEntry;

/*
    Model Entry
*/
typedef struct ModelEntry {
    char Name[128];
    int MeshGroup[8];  // 8 lods in a model
}ModelEntry;

/*
    Component Entry
*/
typedef struct ComponentEntry {
    char TypeName[64];
}ComponentEntry;

/*
    Render Component
*/
typedef struct RenderEntry {
    ComponentEntry Info;
    int ModelIndex;              // model index
    int MaterialIndex;           // material index
    int pad1, pad2;
}RenderEntry;

/*
    Light Entry
*/
typedef struct LightEntry {
    ComponentEntry Info;
    int ModelIndex;
    int MaterialIndex;
    int Type;
    float Intensity;
    float Radius;
    Vector3 Color;
    Vector3 Direction;
}LightEntry;


/*
    Gameobjects, and it's component
*/
typedef struct ObjectEntry {
    char Name[128];
    Vector3 Position;
    Quaternion Rotation;
    Vector3 Scale;
    int NumComponents;
}ObjectEntry;
#pragma pack()

#endif