#ifndef __BONEINFO__
#define __BONEINFO__

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags
#include "vertextype.h"

typedef struct BoneInfo {
	char * name;
	int parent;
	aiMatrix4x4 offsetMatrix;
}BoneInfo;

typedef struct TestFrame {
	int BoneId;
	float Translation[3];
	float Rotation[4];
}TestFrame;

#endif 