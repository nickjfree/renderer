// AssertMaker.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags


#include "struct.h"
#include <Windows.h>


#include "Misc\h3d\h3d.h"
#include "Math\LinearMath.h"


using namespace h3d;



void SaveH3d(aiMesh *Mesh, char* Name)
{
    char FileName[256] = {};
    sprintf(FileName, "%s.h3d", Name);
    HANDLE hFile = CreateFileA(FileName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, 0, NULL);
    DWORD offset = 0;
    DWORD write = 0;
    h3d_vertex * vertex = new h3d_vertex[Mesh->mNumVertices];
    WORD * index = new WORD[Mesh->mNumFaces * 3];
    // conrt to h3d vertext and index
    aiVector3D * position = Mesh->mVertices;
    aiVector3D * normal = Mesh->mNormals;
    aiVector3D * tangent = Mesh->mTangents;
    aiVector3D * texcoord = Mesh->mTextureCoords[0];
    for (int i = 0; i < Mesh->mNumVertices; i++) {
        vertex[i].x = position[i].x;
        vertex[i].y = position[i].y;
        vertex[i].z = position[i].z;
        printf("(%f %f %f) ", position[i].x, position[i].y, position[i].z);

        vertex[i].nx = normal[i].x;
        vertex[i].ny = normal[i].y;
        vertex[i].nz = normal[i].z;


        if (tangent) {
            vertex[i].tx = tangent[i].x;
            vertex[i].ty = tangent[i].y;
            vertex[i].tz = tangent[i].z;
        }
        if (texcoord) {
            vertex[i].u = texcoord[i].x;
            vertex[i].v = -texcoord[i].y;
        }
        vertex[i].parameter = 0;
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
    offset = sizeof(header) + sizeof(mesh) + sizeof(bone);
    // write mesh header
    mesh.VertexNum = Mesh->mNumVertices;
    mesh.VertexSize = sizeof(h3d_vertex);
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

void SaveH3dCharacter(aiMesh *Mesh, char* Name, BoneEntry * Bones)
{
    char FileName[256] = {};
    sprintf(FileName, "%s_character.h3d", Name);
    HANDLE hFile = CreateFileA(FileName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, 0, NULL);
    DWORD offset = 0;
    DWORD write = 0;
    h3d_vertex_skinning * vertex = new h3d_vertex_skinning[Mesh->mNumVertices];

    WORD * index = new WORD[Mesh->mNumFaces * 3];
    // conrt to h3d vertext and index
    aiVector3D * position = Mesh->mVertices;
    aiVector3D * normal = Mesh->mNormals;
    aiVector3D * tangent = Mesh->mTangents;
    aiVector3D * texcoord = Mesh->mTextureCoords[0];
    for (int i = 0; i < Mesh->mNumVertices; i++) {
        // position
        vertex[i].x = position[i].x;
        vertex[i].y = position[i].y;
        vertex[i].z = position[i].z;
        // normal
        vertex[i].nx = normal[i].x;
        vertex[i].ny = normal[i].y;
        vertex[i].nz = normal[i].z;

        vertex[i].bone_id = -1;
        vertex[i].w[0] = vertex[i].w[1] = vertex[i].w[2] = 0.0f;
        if (tangent) {
            vertex[i].tx = tangent[i].x;
            vertex[i].ty = tangent[i].y;
            vertex[i].tz = tangent[i].z;
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
    for (int i = 0; i < Mesh->mNumBones; i++) {
        aiBone * bone = Mesh->mBones[i];
        int bone_index = 0;
        while (strcmp(Bones[bone_index].name, bone->mName.data)) {
            bone_index++;
        }
        for (int j = 0; j < bone->mNumWeights; j++) {
            aiVertexWeight &weight = bone->mWeights[j];
            // handle bone info for vertex
            h3d_vertex_skinning &v = vertex[weight.mVertexId];
            // find bone in this vertex
            int matched = 0;
            for (int b = 0; b < 4; b++) {
                unsigned int bone_id = (v.bone_id >> (b * 8)) & 0x000000ff;
                if (bone_id == bone_index) {
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
                        v.bone_id = v.bone_id & (bone_index << (b * 8) | mask);
                        if (b != 3) {
                            // it is not the last bone, so set weight
                            v.w[b] = weight.mWeight;
                        }
                        else {
                        }
                        break;
                    }
                }
            }
        }
    }
    // fix vertex with less then 4 bones
    for (int i = 0; i < Mesh->mNumVertices; i++) {
        h3d_vertex_skinning &v = vertex[i];
        //printf("bones: ");
        int bone_count = 0;
        for (int b = 0; b < 4; b++) {
            unsigned int bone_id = (v.bone_id >> (b * 8)) & 0x000000ff;
            //printf("%d ", bone_id);
            if (bone_id == 255) {
                unsigned mask = ~(0x000000ff << (8 * (b)));
                v.bone_id = v.bone_id & (0 << (b * 8) | mask);
            }
            else {
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
        //printf(" weight: %f %f %f \n", v.w[0], v.w[1], v.w[2]);
    }

    // prepare h3d structure
    h3d_header header;
    h3d_mesh mesh;
    h3d_bone bone;
    header.Magic = *(DWORD*)H3DMAGIC;
    header.MeshNum = 1;
    header.Version = 0x01;
    WriteFile(hFile, &header, sizeof(header), &write, NULL);
    offset = sizeof(header) + sizeof(mesh) + sizeof(bone);
    // write mesh header
    mesh.VertexNum = Mesh->mNumVertices;
    mesh.VertexSize = sizeof(h3d_vertex_skinning);
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

aiNode * FindNode(aiNode * Node, char * name) {
    if (!strcmp(Node->mName.data, name)) {
        return Node;
    }
    else {
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
            SaveH3d(scene->mMeshes[i], (char*)name);
            // SaveH3dCharacter(scene->mMeshes[i], (char*)name);
        }
    }
}


void BFSBones(BoneEntry * entries, aiNode * node, aiMesh * mesh) {
    // store bones in bfs order into entries

    // queue
    NodeData queue[256];
    int rear = 0;
    int head = 0;
    BoneEntry * entry_ptr = entries;
    int partent = -1;
    // push root
    NodeData RootData;
    RootData.Node = node;
    RootData.parent = partent;
    // push queue
    queue[head++] = RootData;
    // BFS loop
    while (rear < head) {
        NodeData &data = queue[rear++];
        aiNode * node = data.Node;
        // check if current node is a bone
        for (int i = 0; i < mesh->mNumBones; i++) {
            aiBone * bone = mesh->mBones[i];
            if (bone->mName == node->mName) {
                // current node is a bone, save it to entries
                strcpy_s(entry_ptr->name, bone->mName.data);
                entry_ptr->offsetMatrix = bone->mOffsetMatrix;
                entry_ptr->parent = data.parent;
                partent = entry_ptr - entries;
                entry_ptr++;
                break;
            }
        }
        // handle children
        for (int i = 0; i < node->mNumChildren; i++) {
            aiNode * child = node->mChildren[i];
            NodeData ChildData;
            ChildData.Node = child;
            ChildData.parent = partent;
            // push children into queue
            queue[head++] = ChildData;
        }
    }
}



void ExtractAnimeMesh(aiScene * scene) {
    // assume there is only one mesh
    aiMesh *Mesh = scene->mMeshes[0];
    // load bones and sort bones:  parent < children
    BoneEntry * Bones = new BoneEntry[Mesh->mNumBones];
    BFSBones(Bones, scene->mRootNode, Mesh);
    // save the bone data
    hb_header bone_header;
    bone_header.Magic = *(DWORD*)"HUBO";
    bone_header.Version = 0x01;
    bone_header.NumBones = Mesh->mNumBones;
    HANDLE hBone = CreateFileA("bone.hsk", GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, 0, NULL);
    DWORD write;
    WriteFile(hBone, &bone_header, sizeof(hb_header), &write, NULL);
    for (int i = 0; i < Mesh->mNumBones; i++) {
        BoneEntry &Bone = Bones[i];
        // show bones
        printf("bone %s parent: %d\n", Bone.name, Bone.parent);
        WriteFile(hBone, &Bone, sizeof(BoneEntry), &write, NULL);
    }
    CloseHandle(hBone);
    // process animation data

    ha_header anime_header;
    anime_header.Magic = *(DWORD*)"HUAN";
    anime_header.Version = 0x01;
    anime_header.NumChannels = 0;
    anime_header.NumFrames = 0;
    anime_header.NumClips = 1;
    anime_header.OffsetFrames = sizeof(ha_header) + sizeof(ha_clip) * anime_header.NumClips;
    HANDLE hAnime = CreateFileA("..\\Engine\\keyframe\\human_run.ha", GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, 0, NULL);
    WriteFile(hAnime, &anime_header, sizeof(ha_header), &write, NULL);
    // write clips info
    ha_clip clip;
    clip.start = 0;
    clip.end = 30;
    strcpy_s(clip.name, "walk");
    clip.looped = 1;
    WriteFile(hAnime, &clip, sizeof(ha_clip), &write, NULL);
    // write frames
    aiAnimation * animation = scene->mAnimations[0];
    // root bone is special because the fucking fbx importer is a mess
    ha_frame root_frame;
    int TransChannel = 0;
    for (int i = 0; i < animation->mNumChannels; i++) {
        aiString name = animation->mChannels[i]->mNodeName;
        int BoneId = -1;
        int Trans = 1;
        for (int p = 0; p < Mesh->mNumBones; p++) {
            if (!strcmp(Bones[p].name, name.data)) {
                BoneId = p;
            }
        }
        if (BoneId == -1) {
            ha_frame frame;
            printf("node with no bone %s, with frames %d\n", name.data, animation->mChannels[i]->mNumPositionKeys);
            if (!strcmp(name.data, "mixamorig:Hips_$AssimpFbx$_Translation")) {
                // fbx special pretranslation
                BoneId = 0;
                Trans = 1;
                for (int n = 0; n < animation->mChannels[i]->mNumPositionKeys; n++) {
                    printf("%f, %f, %f\n", animation->mChannels[i]->mPositionKeys[n].mValue.x,
                        animation->mChannels[i]->mPositionKeys[n].mValue.y,
                        animation->mChannels[i]->mPositionKeys[n].mValue.z);
                }
            }
            else if (!strcmp(name.data, "mixamorig:Hips_$AssimpFbx$_Rotation")) {
                // fbx special rotation
                BoneId = 0;
                Trans = 0;
                for (int n = 0; n < animation->mChannels[i]->mNumRotationKeys; n++) {
                    printf("%f, %f, %f, %f\n", animation->mChannels[i]->mRotationKeys[n].mValue.x,
                        animation->mChannels[i]->mRotationKeys[n].mValue.y,
                        animation->mChannels[i]->mRotationKeys[n].mValue.z,
                        animation->mChannels[i]->mRotationKeys[n].mValue.w);
                }
            }
            else {
                continue;
            }
        }
        float time_scale = 1000.0f / animation->mTicksPerSecond;

        // root bone is special because the fucking fbx importer is a mess
        if (BoneId == 0) {
            if (Trans) {
                anime_header.NumFrames = animation->mChannels[i]->mNumPositionKeys;
                TransChannel = i;
            }
            else if (!Trans) {
                anime_header.NumChannels++;
                for (int index = 0; index < anime_header.NumFrames; index++) {
                    root_frame.time = animation->mChannels[TransChannel]->mPositionKeys[index].mTime * time_scale;
                    root_frame.bone_id = BoneId;
                    root_frame.tx = animation->mChannels[TransChannel]->mPositionKeys[index].mValue.x;
                    root_frame.ty = animation->mChannels[TransChannel]->mPositionKeys[index].mValue.y;
                    root_frame.tz = animation->mChannels[TransChannel]->mPositionKeys[index].mValue.z;
                    // fix rotation
                    Quaternion Fix;
                    Fix.RotationAxis(Vector3(1, 0, 0), XM_PIDIV2);
                    Quaternion Value = Quaternion(animation->mChannels[i]->mRotationKeys[index].mValue.x,
                        animation->mChannels[i]->mRotationKeys[index].mValue.y,
                        animation->mChannels[i]->mRotationKeys[index].mValue.z,
                        animation->mChannels[i]->mRotationKeys[index].mValue.w);
                    Value = Value * Fix;

                    root_frame.rx = Value.x;
                    root_frame.ry = Value.y;
                    root_frame.rz = Value.z;
                    root_frame.rw = Value.w;

                    WriteFile(hAnime, &root_frame, sizeof(ha_frame), &write, NULL);
                }
            }

            continue;
        }

        // normal keyframes assume each joint has the same amount of key frames
        anime_header.NumFrames = animation->mChannels[i]->mNumPositionKeys;
        printf("node frames %d  bone_id %d\n", anime_header.NumFrames, BoneId);
        anime_header.NumChannels++;

        for (int index = 0; index < anime_header.NumFrames; index++) {
            ha_frame frame;
            frame.time = animation->mChannels[i]->mPositionKeys[index].mTime * time_scale;
            frame.bone_id = BoneId;
            frame.tx = animation->mChannels[i]->mPositionKeys[index].mValue.x;
            frame.ty = animation->mChannels[i]->mPositionKeys[index].mValue.y;
            frame.tz = animation->mChannels[i]->mPositionKeys[index].mValue.z;

            frame.rx = animation->mChannels[i]->mRotationKeys[index].mValue.x;
            frame.ry = animation->mChannels[i]->mRotationKeys[index].mValue.y;
            frame.rz = animation->mChannels[i]->mRotationKeys[index].mValue.z;
            frame.rw = animation->mChannels[i]->mRotationKeys[index].mValue.w;
            WriteFile(hAnime, &frame, sizeof(ha_frame), &write, NULL);
        }
    }
    // rewrite the header
    SetFilePointer(hAnime, 0, 0, FILE_BEGIN);
    printf("total channels %d, total bones %d\n", anime_header.NumChannels, Mesh->mNumBones);
    WriteFile(hAnime, &anime_header, sizeof(ha_header), &write, NULL);
    CloseHandle(hAnime);
    // SaveH3dCharacter
    SaveH3dCharacter(Mesh, "", Bones);
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
    if (!scene) {
        printf("scene load failed\n");
        return false;
    }  // Now we can access the file's contents.   
    printf("scene load success\n");
    ExtractMeshToH3d(scene);
    //ExtractAnimeMesh(scene);
    // We're done. Everything will be cleaned up by the importer destructor  
    return true;
}



int _tmain(int argc, _TCHAR* argv[])
{
    //DoTheImportThing("human_2.fbx");
    DoTheImportThing("head.fbx");
    return 0;
}

