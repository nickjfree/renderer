// AssertMaker.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags


#include "struct.h"

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
		printf("(%f %f %f) ", position[i].x, position[i].y, position[i].z);
		vertex[i].normal = Vector3(normal[i].x, normal[i].y, normal[i].z);
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
			//printf("%d ", indics[j]);
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

void SaveH3dCharacter(aiMesh *Mesh, char* Name)
{
	char FileName[256] = {};
	sprintf(FileName, "%s_character.h3d", Name);
	HANDLE hFile = CreateFileA(FileName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, 0, NULL);
	DWORD offset = 0;
	DWORD write = 0;
	vertex_skinning * vertex = new vertex_skinning[Mesh->mNumVertices];

	WORD * index = new WORD[Mesh->mNumFaces * 3];
	// conrt to h3d vertext and index
	aiVector3D * position = Mesh->mVertices;
	aiVector3D * normal = Mesh->mNormals;
	aiVector3D * tangent = Mesh->mTangents;
	aiVector3D * texcoord = Mesh->mTextureCoords[0];
	for (int i = 0; i < Mesh->mNumVertices; i++) {
		vertex[i].position = Vector3(position[i].x, position[i].y, position[i].z);
		//printf("(%f %f %f) ", position[i].x, position[i].y, position[i].z);
		vertex[i].normal = Vector3(normal[i].x, normal[i].y, normal[i].z);
		vertex[i].bone_id = -1;
		vertex[i].w[0] = vertex[i].w[1] = vertex[i].w[2] = 0.0f;
		if (tangent) {
			vertex[i].tangent = Vector3(tangent[i].x, tangent[i].y, tangent[i].z);
		}
		if (texcoord) {
			vertex[i].u = texcoord[i].x;
			vertex[i].v = -texcoord[i].y;
		}
	}
	// now the index
	for (int i = 0; i < Mesh->mNumFaces; i++) {
		int pos = i * 3;
		unsigned int * indics = Mesh->mFaces[i].mIndices;
		for (int j = 0; j < Mesh->mFaces[i].mNumIndices; j++) {
			index[pos + j] = (WORD)indics[j];
			//printf("%d ", indics[j]);
		}
	}
	// handle bone and weight
	for (int i = 0; i < Mesh->mNumBones; i++){
		aiBone * bone = Mesh->mBones[i];
		for (int j = 0; j < bone->mNumWeights; j++) {
			aiVertexWeight &weight = bone->mWeights[j];
			// handle bone info for vertex
			vertex_skinning &v = vertex[weight.mVertexId];
			// find bone in this vertex
			int matched = 0;
			for (int b = 0; b < 4; b++) {
				unsigned int bone_id = (v.bone_id >> (b * 8)) & 0x000000ff;
				if (bone_id == j) {
					matched = 1;
					break;
				}
			}
			if (!matched) {
				// a new bone, so add id to it
				for (int b = 0; b < 4; b++) {
					unsigned int bone_id = (v.bone_id >> (b * 8)) & 0x000000ff;
					if (bone_id == 255) {
						unsigned mask = ~(0x000000ff << (8 * (b)));
						v.bone_id = v.bone_id & (i << (b * 8) | mask);
						if (b != 3) {
							// it is not the last bone, so set weight
							v.w[b] = weight.mWeight;
						} else {
						}
						break;
					}
				}
			}
		}
	}
	// fix vertex with less then 4 bones
	for (int i = 0; i < Mesh->mNumVertices;i++) {
		vertex_skinning &v = vertex[i];
		printf("bones: ");
		int bone_count = 0;
		for (int b = 0; b < 4; b++) {
			unsigned int bone_id = (v.bone_id >> (b * 8)) & 0x000000ff;
			printf("%d ", bone_id);
			if (bone_id == 255) {
				unsigned mask = ~(0x000000ff << (8 * (b)));
				v.bone_id = v.bone_id & (0 << (b * 8) | mask);
			} else {
				bone_count++;
			}
		}
		if (bone_count < 4) {
			// normalize	
			float scale = 1.0f / (v.w[0] + v.w[1] + v.w[2]);
			v.w[0] *= scale;
			v.w[1] *= scale;
			v.w[2] *= scale;
		}
		printf(" weight: %f %f %f \n", v.w[0], v.w[1], v.w[2]);
	}

	// prepare h3d structure
	h3d_header header;
	h3d_mesh mesh;
	h3d_bone bone;
	header.Magic = (DWORD)H3DMAGIC;
	header.MeshNum = 1;
	header.Version = 0x01;
	WriteFile(hFile, &header, sizeof(header), &write, NULL);
	offset = sizeof(header) + sizeof(mesh) + sizeof(bone);
	// write mesh header
	mesh.VertexNum = Mesh->mNumVertices;
	mesh.VertexSize = sizeof(vertex_skinning);
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

aiNode *  FindNode(aiNode * Node, char * name) {
		if (!strcmp(Node->mName.data, name)) {
			return Node;
		} else {
			// find in children
			int i = 0;
			while (i < Node->mNumChildren) {
				aiNode * retNode = FindNode(Node->mChildren[i], name);
				if (retNode) {
					return retNode;
				}
				i++;
			}
			return NULL;
		}
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
			//SaveH3d(scene->mMeshes[i], (char*)name);
			SaveH3dCharacter(scene->mMeshes[i], (char*)name);
		}
	}
	// save bone data
	DWORD write;
	char pad = 0;
	aiMesh *Mesh = scene->mMeshes[0];
	BoneInfo * Bones = new BoneInfo[Mesh->mNumBones];
	HANDLE BoneFile = CreateFileA("bone", GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, 0, NULL);
	for (int i = 0; i < Mesh->mNumBones; i++) {
		aiBone * bone = Mesh->mBones[i];
		Bones[i].name = bone->mName.data;
		Bones[i].offsetMatrix = bone->mOffsetMatrix;
		Bones[i].parent = -1;
		// find parent id
		// write parent id
	}
	// save parent infos
	for (int i = 0; i < Mesh->mNumBones; i++) {
		aiBone * bone = Mesh->mBones[i];
		Bones[i].name = bone->mName.data;
		Bones[i].parent = -1;
		// find node first
		aiNode * self = FindNode(scene->mRootNode, bone->mName.data);
		aiNode * ParentNode = self->mParent;
		// find parent id
		int p = 0;
		for (int p = 0; p < Mesh->mNumBones; p++) {
			if (Mesh->mBones[p]->mName == ParentNode->mName) {
				Bones[i].parent = p;
			}
		}
		WriteFile(BoneFile, &Bones[i].parent, sizeof(int), &write, NULL);
		WriteFile(BoneFile, &pad, sizeof(char) * 12, &write, NULL);
		WriteFile(BoneFile, &Bones[i].offsetMatrix, sizeof(float) * 16, &write, NULL);
		
		printf("%d bone: %s %d\n", i, Mesh->mBones[i]->mName.data, Bones[i].parent);
	}


	//WriteFile(BoneFile, &bone->mOffsetMatrix, sizeof(float) * 16, &write, NULL);
	CloseHandle(BoneFile);

	// save test animation file
	HANDLE AnimeFile = CreateFileA("anime", GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, 0, NULL);
	aiAnimation * animation = scene->mAnimations[0];
	for (int i = 0; i < animation->mNumChannels; i++) {
		aiString name = animation->mChannels[i]->mNodeName;
		int BoneId = -1;
		for (int p = 0; p < Mesh->mNumBones; p++) {
			if (Mesh->mBones[p]->mName == name) {
				BoneId = p;
			}
		}
		// same frame 0 as test frame
		TestFrame Frame = {};
		if (BoneId == -1) {
			printf("node with no bone %s\n", name.data);
			continue;
		}
		int index = 0;
		if (animation->mChannels[i]->mNumPositionKeys == 39) {
			index = 1;
		} else {
			index = 0;
		}
		Frame.BoneId = BoneId;
		Frame.Translation[0] = animation->mChannels[i]->mPositionKeys[index].mValue.x;
		Frame.Translation[1] = animation->mChannels[i]->mPositionKeys[index].mValue.y;
		Frame.Translation[2] = animation->mChannels[i]->mPositionKeys[index].mValue.z;
		index = 0;
		if (animation->mChannels[i]->mNumRotationKeys == 39) {
			index = 1;
		} else {
			index = 0;
		}
		Frame.Rotation[0] = animation->mChannels[i]->mRotationKeys[index].mValue.x;
		Frame.Rotation[1] = animation->mChannels[i]->mRotationKeys[index].mValue.y;
		Frame.Rotation[2] = animation->mChannels[i]->mRotationKeys[index].mValue.z;
		Frame.Rotation[3] = animation->mChannels[i]->mRotationKeys[index].mValue.w;

		WriteFile(AnimeFile, &Frame, sizeof(Frame), &write, NULL);
	}
	CloseHandle(AnimeFile);
}


bool DoTheImportThing(const std::string& pFile) {
	// Create an instance of the Importer class  
	Assimp::Importer importer;
	// And have it read the given file with some example postprocessing 
	// Usually - if speed is not the most important aspect for you - you'll   
	// propably to request more postprocessing than we do in this example.  
	importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);

	aiScene* scene = (aiScene*)importer.ReadFile(pFile,
		aiProcess_CalcTangentSpace
		| aiProcess_Triangulate
		| aiProcess_JoinIdenticalVertices
		| aiProcess_SortByPType 
		| aiProcess_MakeLeftHanded
		| aiProcess_ImproveCacheLocality
		/*| aiProcess_LimitBoneWeights*/
		/*| aiProcess_PreTransformVertices*/
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
	DoTheImportThing("human_test.fbx");
	return 0;
}

