// AssertMaker.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#include "vertextype.h"
#include "h3d.h"


using namespace ModelSystem::h3d;


void SaveH3d(aiMesh *Mesh, char* Name)
{
	char FileName[256] = {};
	sprintf(FileName, "%s.h3d", Name);
	HANDLE hFile = CreateFileA(FileName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, 0, NULL);
	DWORD offset = 0;
	DWORD write = 0;
	vertex_dynamic_instancing * vertex = new vertex_dynamic_instancing[Mesh->mNumVertices];
	WORD * index = new WORD[Mesh->mNumFaces * 3];
	// conrt to h3d vertext and index
	aiVector3D * position = Mesh->mVertices;
	aiVector3D * normal = Mesh->mNormals;
	aiVector3D * tangent = Mesh->mTangents;
	aiVector3D * texcoord = Mesh->mTextureCoords[0];
	for (int i = 0; i < Mesh->mNumVertices; i++) {
		vertex[i].position = Vector3(position[i].x, position[i].y, position[i].z);
		//printf("(%f %f %f) ", position[i].x, position[i].y, position[i].z);
		vertex[i].normal = Vector3(normal[i].x, normal[i].y, normal[i].x);
		if (tangent) {
			vertex[i].tangent = Vector3(tangent[i].x, tangent[i].y, tangent[i].z);
		}
		if (texcoord) {
			vertex[i].u = texcoord[i].x;
			vertex[i].v = -texcoord[i].y;
		}
		vertex[i].instance_id = 0;
	}
	// now the index
	for (int i = 0; i < Mesh->mNumFaces; i++) {
		int pos = i * 3;
		unsigned int * indics = Mesh->mFaces[i].mIndices;
		for (int j = 0; j < Mesh->mFaces[i].mNumIndices; j++) {
			index[pos + j] = (WORD)indics[j];
			printf("%d ", indics[j]);
		}
	}
	// prepare h3d structure
	h3d_header header;
	h3d_mesh mesh;
	h3d_bone bone;
	header.Magic = (DWORD)H3DMAGIC;
	header.MeshNum = 1;
	header.Version = 0x01;
	WriteFile(hFile, &header, sizeof(header), &write, NULL);
	offset = sizeof(header)+sizeof(mesh)+sizeof(bone);
	// write mesh header
	mesh.VertexNum = Mesh->mNumVertices;
	mesh.VertexSize = sizeof(vertex_dynamic_instancing);
	mesh.IndexSize = sizeof(WORD);
	mesh.IndexNum = Mesh->mNumFaces * 3;
	mesh.OffsetVertex = offset;
	offset += mesh.VertexSize * mesh.VertexNum;
	mesh.OffsetIndex = offset;
	printf("vertex count %d, indics count %d\n", mesh.VertexNum, mesh.IndexNum);
	WriteFile(hFile, &mesh, sizeof(mesh), &write, NULL);
	WriteFile(hFile, &bone, sizeof(bone), &write, NULL);
	// save vertex and index
	WriteFile(hFile, vertex, mesh.VertexSize * mesh.VertexNum, &write, NULL);
	WriteFile(hFile, index, mesh.IndexNum * mesh.IndexSize, &write, NULL);
	//over
	CloseHandle(hFile);
}

void ExtractMeshToH3d(aiScene * scene)
{
	// extract scene mesh to h3d files, each file contains a mesh, with names
	// hash nothing to do with the node, only mesh here 
	if (scene->HasMeshes()) {
		int NumMesh = scene->mNumMeshes;
		for (int i = 0; i < NumMesh; i++) {
			const char * name = scene->mMeshes[i]->mName.C_Str();
			printf("extracting mesh %s\n", name);
			// save to files
			SaveH3d(scene->mMeshes[i], (char*)name);
		}
	}
}


bool DoTheImportThing(const std::string& pFile) {
	// Create an instance of the Importer class  
	Assimp::Importer importer;
	// And have it read the given file with some example postprocessing 
	// Usually - if speed is not the most important aspect for you - you'll   
	// propably to request more postprocessing than we do in this example.  
	aiScene* scene = (aiScene*)importer.ReadFile(pFile,
		aiProcess_CalcTangentSpace
		| aiProcess_Triangulate
		| aiProcess_JoinIdenticalVertices
		| aiProcess_SortByPType 
		| aiProcess_MakeLeftHanded
		/*| aiProcess_FlipWindingOrder*/);
	// If the import failed, report it  
	if( !scene)  {
		printf("scene load failed\n");    
		return false;  
	}  // Now we can access the file's contents.   
	printf("scene load success\n");
	ExtractMeshToH3d(scene);
	// We're done. Everything will be cleaned up by the importer destructor  
	return true;
}



int _tmain(int argc, _TCHAR* argv[])
{
	DoTheImportThing("adata.dae");
	return 0;
}

