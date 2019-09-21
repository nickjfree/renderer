#include "H3DMesh.h"
#include "hacdCircularList.h"
#include "hacdVector.h"
#include "hacdICHull.h"
#include "hacdGraph.h"
#include "hacdHACD.h"


USING_ALLOCATER(H3DMesh);

H3DMesh::H3DMesh(Context * context) : Mesh(context) {
    VTSize = 48;
}


H3DMesh::~H3DMesh() {
}




h3d_mesh * H3DMesh::GetH3DMesh(h3d_header * Header, int MeshIndex) {
    h3d_mesh * Mesh = 0;
    if (Header->MeshNum < (MeshIndex + 1))
    {
        return NULL;
    }
    return (h3d_mesh*)((char*)Header + sizeof(h3d_header) + MeshIndex * sizeof(h3d_mesh));
}


int H3DMesh::OnSerialize(Deserializer& deserializer) {
    printf("mesh serialize %s\n", this->GetUrl().ToStr());
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
    VTSize = H3DMesh->VertexSize;
    IBuffer = Index;
    // check for out off bound indices
    for (DWORD i = 0; i < INum; i++) {
        unsigned int pos = IBuffer[i];
        if (pos > H3DMesh->VertexNum) {
            IBuffer[i] = static_cast<WORD>(H3DMesh->VertexNum - 1);
        }
    }
    return 0;
}



int H3DMesh::OnLoadComplete(Variant& Data) {
    // create geometry in GPU
    id = renderinterface->CreateGeometry(VBuffer, VBSize, VTSize, IBuffer, INum, FORMAT_R16_UINT, R_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    // calc AABB 
    float minx = 0, miny = 0, minz = 0;
    float maxx = 0, maxy = 0, maxz = 0;
    char * vertex = (char*)VBuffer;
    int Count = VBSize / VTSize;
    for (int i = 0; i < Count; i++) {
        h3d_vertex v = *(h3d_vertex *)(vertex + VTSize * i);
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
    float d = max(max(Extend.x, Extend.y), max(Extend.x, Extend.z));
    Box = AABB(Center, Vector3(d, d, d));
    // calc convex hull
#ifndef _DEBUG
    //ComputeConvexHull();
#endif 
    return 0;
}



int H3DMesh::OnCreateComplete(Variant& Data) {
    NotifyOwner(RM_LOAD, Data);
    return 0;
}

void H3DMesh::ComputeConvexHull() {
    std::vector< HACD::Vec3<HACD::Real> > points;
    std::vector< HACD::Vec3<long> > triangles;
    int Count = VBSize / VTSize;

    h3d_vertex * vertices = (h3d_vertex *)VBuffer;
    for (int i = 0; i < Count; i++) {
        h3d_vertex * vt = (h3d_vertex*)((char*)vertices + VTSize * i);
        HACD::Vec3<HACD::Real> vertex(vt->x, vt->y, vt->z);
        points.push_back(vertex);
    }


    int Triangles = INum / 3;
    for (int i = 0; i < Triangles; i++) {
        HACD::Vec3<long> triangle(IBuffer[i * 3], IBuffer[i * 3 + 1], IBuffer[i * 3 + 2]);
        triangles.push_back(triangle);
    }

    HACD::HACD myHACD;
    myHACD.SetPoints(&points[0]);
    myHACD.SetNPoints(points.size());
    myHACD.SetTriangles(&triangles[0]);
    myHACD.SetNTriangles(triangles.size());
    myHACD.SetCompacityWeight(0.1);
    myHACD.SetVolumeWeight(0.0);

    // HACD parameters

    // Recommended parameters: 2 100 0 0 0 0

    size_t nClusters = 2;
    double concavity = 200000;
    bool invert = false;
    bool addExtraDistPoints = true;
    bool addNeighboursDistPoints = false;
    bool addFacesPoints = false;
    float smallClusterThreshold = 5;

    myHACD.SetNClusters(nClusters);                     // minimum number of clusters
    myHACD.SetNVerticesPerCH(100);                      // max of 100 vertices per convex-hull
    myHACD.SetConcavity(concavity);                     // maximum concavity
    myHACD.SetAddExtraDistPoints(addExtraDistPoints);
    myHACD.SetAddNeighboursDistPoints(addNeighboursDistPoints);

    myHACD.SetAddFacesPoints(addFacesPoints);
    myHACD.Compute();
    nClusters = myHACD.GetNClusters();

    if (nClusters) {
        ConvexHulls = new MeshConvex[nClusters];
    }

    printf("%s has %zd clusters\n", URL.ToStr(), nClusters);
    for (int c = 0; c < nClusters; c++) {
        //generate convex result
        Vector3 Center(0, 0, 0);
        size_t nPoints = myHACD.GetNPointsCH(c);
        size_t nTriangles = myHACD.GetNTrianglesCH(c);

        float* vertices = new float[nPoints * 4];
        unsigned int* triangles = new unsigned int[nTriangles * 3];

        HACD::Vec3<HACD::Real> * pointsCH = new HACD::Vec3<HACD::Real>[nPoints];
        HACD::Vec3<long> * trianglesCH = new HACD::Vec3<long>[nTriangles];
        myHACD.GetCH(c, pointsCH, trianglesCH);

        // points
        for (size_t v = 0; v < nPoints; v++) {
            vertices[4 * v] = (float)pointsCH[v].X();
            vertices[4 * v + 1] = (float)pointsCH[v].Y();
            vertices[4 * v + 2] = (float)pointsCH[v].Z();
            vertices[4 * v + 3] = 0.0f;
            Center = Center + Vector3((float)pointsCH[v].X(), (float)pointsCH[v].Y(), (float)pointsCH[v].Z());
        }
        Center = Center * (1.0f / nPoints);
        // shift points with center at (0,0,0)
        for (size_t v = 0; v < nPoints; v++) {
            Vector3 vert = Vector3(vertices[4 * v], vertices[4 * v + 1], vertices[4 * v + 2]);
            vert = vert - Center;
            vertices[4 * v] = vert.x;
            vertices[4 * v + 1] = vert.y;
            vertices[4 * v + 2] = vert.z;
            vertices[4 * v + 3] = 0;
        }
        // triangles
        for (size_t f = 0; f < nTriangles; f++) {
            triangles[3 * f] = trianglesCH[f].X();
            triangles[3 * f + 1] = trianglesCH[f].Y();
            triangles[3 * f + 2] = trianglesCH[f].Z();
        }
        ConvexHulls[c].Center = Center;
        ConvexHulls[c].VNum = static_cast<int>(nPoints);
        ConvexHulls[c].VBuffer = vertices;
        ConvexHulls[c].INum = static_cast<int>(nTriangles);
        ConvexHulls[c].IBuffer = triangles;

        delete[] pointsCH;
        delete[] trianglesCH;
    }
    NumConvex = static_cast<int>(nClusters);
}


int H3DMesh::AsyncUnLoad() {
	// free data in read from file
	DeSerial.Release();
	// delete geometry id from renderer
	renderinterface->DestroyGeometry(id);
	return 0;
}

int H3DMesh::OnDestroy(Variant& Data) {
	return 0;
}