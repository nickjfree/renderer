#include "CullingTask.h"
#include "RenderingSystem.h"

USING_ALLOCATER(CullingTask);
USING_RECYCLE(CullingTask);

CullingTask::CullingTask()
{
}


CullingTask::~CullingTask()
{
}


int CullingTask::Work() {
	Frustum frustum = renderview->Camera->GetFrustum();
	BatchCompiler * Compiler = renderview->Compiler;
	renderview->VisibleObjects.Empty();
  
    
   
	spatial->Query(frustum, renderview->VisibleObjects, ObjectType);

//	printf("visible objects in %d %d\n", ObjectType, renderview->VisibleObjects.Size());
	int Size = renderview->VisibleObjects.Size();
	char * Buffer = (char*)renderview->CommandBuffer;
	Compiler->SetBuffer(Buffer);
	int Compiled = 0;
	// target and view prepair
	renderview->Compile(Context);
	for (int i = 0; i < Size; i++) {
		RenderObject * obj = (RenderObject*)renderview->VisibleObjects[i];
		Compiled += obj->Compile(Compiler, renderview->Type, 0, renderview->Parameters, renderview->Camera, Context);
        // clear frame params
        renderview->Parameters.Clear();
		if (Compiled >= COMMANDBUFFER_SIZE) {
			printf("warning: commandbuffer overflow\n");
			break;
		}
	}
	// add the buffer to renderqueue
	renderview->QueueCommand();
	// signal main thread
	renderview->Event->Set();   
	return 0;
}


int CullingTask::Complete() {
	// report task complete
	return 0;
}

