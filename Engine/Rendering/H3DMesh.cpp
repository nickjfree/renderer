#include "H3DMesh.h"


USING_ALLOCATER(H3DMesh);

H3DMesh::H3DMesh(Context * context) : Mesh(context){
	VTSize = 48;
}


H3DMesh::~H3DMesh() {
}




h3d_mesh * H3DMesh::GetH3DMesh(h3d_header * Header, int MeshIndex)
{
	h3d_mesh * Mesh = 0;
	if (Header->MeshNum < (MeshIndex + 1))
	{
		return NULL;
	}
	return (h3d_mesh*)((char*)Header + sizeof(h3d_header)+MeshIndex * sizeof(h3d_mesh));
}


int H3DMesh::OnSerialize(Deserializer& deserializer) {
	printf("mesh serialize %s\n", (char*)this->GetUrl());
	void * Data = deserializer.Raw();
	h3d_header * Header = (h3d_header*)Data;
	h3d_mesh   * H3DMesh = GetH3DMesh(Header, this->Index);
	DWORD VSize = H3DMesh->VertexSize * H3DMesh->VertexNum;
	void * Vertex = H3DMesh->OffsetVertex + (char*)Header;
	DWORD INum = H3DMesh->IndexNum;
	WORD * Index = (WORD*)(H3DMesh->OffsetIndex + (char*)Header);
	VBuffer = Vertex;
	VBSize = VSize;
	this->INum = INum;
	IBuffer = Index;
	return 0;
}



int H3DMesh::OnLoadComplete(Variant& Data) {
	// create geometry in GPU
	id = renderinterface->CreateGeometry(VBuffer, VBSize, VTSize, IBuffer, INum, FORMAT_R16_UINT);
	// calc AABB 
	float minx = 0,miny = 0,minz = 0;
	float maxx = 0,maxy = 0,maxz = 0;
	h3d_vertex * vertex = (h3d_vertex*)VBuffer;
	int Count = VBSize / sizeof(h3d_vertex);
	for (int i = 0; i < Count; i++) {
		h3d_vertex v = vertex[i];
		if (v.x > maxx) {
			maxx = v.x;
		}
		if (v.y > maxy) {
			maxy = v.y;
		}
		if (v.z > maxz) {
			maxz = v.z;
		}
		if (v.x < minx) {
			minx = v.x;
		}
		if (v.y < miny) {
			miny = v.y;
		}
		if (v.z < minz) {
			minz = v.z;
		}
	}
	Vector3 Center = (Vector3(maxx, maxy, maxz) + Vector3(minx, miny, minz)) * 0.5;
	Vector3 Extend = (Vector3(maxx, maxy, maxz) - Vector3(minx, miny, minz)) * 0.5;
	Box = AABB(Center, Extend);
	return 0;
}



int H3DMesh::OnCreateComplete(Variant& Data) {
	//Event * event = Event::Create();
	//event->EventId = 999;
	//event->EventParam["Mesh"].as<Mesh*>() = this;
	//context->SendEvent(Listener, event);
	//event->Recycle();
	NotifyOwner(RM_LOAD, Data);
	return 0;
}

