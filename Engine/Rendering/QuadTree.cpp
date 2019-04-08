#include "QuadTree.h"


QuadTree::QuadTree(Context * context) : PartitionMethod(context)
{
    Init();
}


QuadTree::~QuadTree()
{
}

void QuadTree::Init() {
    /*RenderEngine = CRenderEngine::GetRenderEngine();
    SceneManager = RenderEngine->GetSceneManager();
    Tree = SceneManager->CreateQuadTree();*/
}

//int QuadTree::Construct(CRect2D& Rect, float accuracy) {
//	Tree->Construct(Rect, accuracy);
//	Rect = Rect;
//	return 0;
//}

int QuadTree::OnAttach(GameObject * GameObj) {
    // add quad tree to scene
    //SceneManager->ActiveQuadTree(Tree);
    return 0;
}

int QuadTree::HandleEvent(Event * Evt) {
    // test code
    if (Evt->EventId == 300) {
        //CSceneNode * Node = Evt->EventParam["SceneNode"].as<CSceneNode*>();
        //Tree->AddSenceNode(Node, 0);
    }
    return 0;
}


