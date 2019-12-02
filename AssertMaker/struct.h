#ifndef __BONEINFO__
#define __BONEINFO__

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags



typedef struct BoneInfo {
    char * name;
    int parent;
    aiMatrix4x4 offsetMatrix;
}BoneInfo;

#pragma pack(1)
typedef struct BoneEntry {
    unsigned char parent;
    aiMatrix4x4 offsetMatrix;
    char name[32];
}BoneEntry;
#pragma pack()

typedef struct NodeData {
    aiNode * Node;
    int parent;
}NodeData;


typedef struct TestFrame {
    int BoneId;
    float Translation[3];
    float Rotation[4];
}TestFrame;

#endif 