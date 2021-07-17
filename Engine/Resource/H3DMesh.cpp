#include "H3DMesh.h"


USING_ALLOCATER(H3DMesh);

H3DMesh::H3DMesh(Context * context) : Mesh(context){
	SceneManager = CRenderEngine::GetRenderEngine()->GetSceneManager();
	Entity = SceneManager->CreateEntity();
}


H3DMesh::~H3DMesh() {
}


//int H3DMesh::Load() {
//	// test load
//	Entity = SceneManager->CreateEntity();
//	Entity->Create("unit.pack", "c", 0, 0);
//	return Entity->Load();
//}
//
//int H3DMesh::UnLoad() {
//	return Entity->UnLoad();
//}

int CreateCallback(Resource * resource, int CreateId) {
	resource->OnCreateComplete((void*)CreateId);
	return 0;
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

int H3DMesh::OnSerialize(void * Data) {
	h3d_header * Header = (h3d_header*)Data;
	h3d_mesh   * H3DMesh = GetH3DMesh(Header, this->Index);
	DWORD VSize = H3DMesh->VertexSize * H3DMesh->VertexNum;
	void * Vertex = H3DMesh->OffsetVertex + (char*)Header;
	DWORD INum = H3DMesh->IndexNum;
	WORD * Index = (WORD*)(H3DMesh->OffsetIndex + (char*)Header);
	Entity->SetGeometryData(Vertex, VSize, Index, INum);
	return 0;
}

int H3DMesh::OnLoadComplete(void *Data) {
	// create geometry in GPU
	Entity->Load((h3d_callback)CreateCallback, this);
	return 0;
}

int H3DMesh::OnCreateComplete(void * Data) {
	Event * event = Event::Create();
	event->EventId = 999;
	event->EventParam["Mesh"]= this;
	context->SendEvent(Listener, event);
	event->Recycle();
	return 0;
}

