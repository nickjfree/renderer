#ifndef __H3D_H__
#define __H3D_H__

#define H3DMAGIC   "HU3D"


namespace h3d {

#pragma pack(1)



	typedef struct h3d_mesh
	{
		DWORD VertexSize;
		DWORD IndexSize;
		DWORD VertexNum;
		DWORD OffsetVertex;
		DWORD IndexNum;
		DWORD OffsetIndex;
		char  Texture[256];
	}h3d_mesh;

	//280

	typedef struct h3d_bone
	{
		int    Frames;
		int    BoneNum;
		DWORD  OffsetBone;
	}h3d_bone;

	//12

	typedef struct h3d_header
	{
		DWORD  Magic;
		DWORD  Version;
		int    MeshNum;
	}h3d_header;

	//12


	typedef struct h3d_vertex
	{
		float x, y, z;
		float nx, ny, nz;
		float u, v;
		float parameter;
		float tx, ty, tz;
	}h3d_vertex;

	//12

	typedef struct h3d_vertex_skinning
	{
		float x, y, z;
		float nx, ny, nz;
		float u, v;
		unsigned int bone_id;
		float  w[3];
		float tx, ty, tz;
	}h3d_vertex_skinning;

	//
	typedef struct hb_header
	{
		DWORD Magic;
		DWORD Version;
		int NumBones;
	}hb_header;

	typedef struct hb_bone {
		unsigned char parent;
		float m00, m01, m02, m03;
		float m10, m11, m12, m13;
		float m20, m21, m22, m23;
		float m30, m31, m32, m33;
		char name[32];
	}hb_bone;

	typedef struct ha_header
	{
		DWORD Magic;
		DWORD Version;
		int NumFrames;
		int NumChannels;
		int NumClips;
		DWORD OffsetFrames;
	}ha_header;

	typedef struct ha_clip
	{
		int start, end;
		bool looped;
		char name[16];
	}ha_clip;

	typedef struct ha_frame {
		unsigned char bone_id;
		double time;
		float tx, ty, tz;
		float rx, ry, rz, rw;
	}ha_frame;



#pragma pack()

} //end namespace

#endif