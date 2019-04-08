#include "Node.h"


USING_ALLOCATER(Node);

Node::Node() :Type(Node::NODE)
{
    SubNodes.Owner = this;
    Sibling.Owner = this;
}


Node::~Node()
{
}

void Node::AddChild(Node * Child) {
    Child->Sibling.InsertAfter(&SubNodes);
}

void Node::Remove(Node * Child) {
    Child->Sibling.Remove();
}


int Node::Query(Frustum& Fr, Vector<Node*>& Result, int Types, bool inside) {
    // test if we are the one
    if (Type & Types) {
        Node * node = this;
        Result.PushBack(node);
    }
    LinkList<Node>::Iterator Iter;
    for (Iter = SubNodes.Begin(); Iter != SubNodes.End(); Iter++) {
        Node * sub = *Iter;
        CullingObject& cull_obj = sub->GetCullingObject();
        int test = Fr.Contains(cull_obj);
        switch (test) {
        case ContainType::CONTAINS:
            inside = true;
            break;
        case ContainType::INTERSECTS:
            inside = false;
            break;
        case ContainType::DISJOINT:
            continue;
        default:
            continue;
        }
        sub->Query(Fr, Result, Types, inside);
    }
    return 0;
}


int Node::Compile(BatchCompiler * Compiler, int Stage, int Lod) {
    return 0;
}

void Node::SyncCullingObject() {
    Vector3 Center = CullingObj.LocalCenter;
    //Center = Center * Rotation;
    //Center = Center + Position;
    Center = Center * GetWorldMatrix();
    CullingObj.Translate(Center);

}


void Node::SetPosition(Vector3& Position_) {
    Position = Position_;
    SyncCullingObject();
}

void Node::SetRotation(Quaternion& Rot) {
    Rotation = Rot;
    SyncCullingObject();
}

Matrix4x4& Node::GetWorldMatrix() {
    Transform.Tranform(Position, Rotation);
    return Transform;
}
