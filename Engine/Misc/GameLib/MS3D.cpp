#include "stdafx.h"
#include "MS3D.h"


CMS3D::CMS3D(void)
{
}

CMS3D::~CMS3D(void)
{
}

int CMS3D::LoadMod(char * FileName)
{
	HANDLE hFile = CreateFileA(FileName,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);
	if(hFile == (HANDLE)-1)
	{
		return -1;
	}
	DWORD FileSize = GetFileSize(hFile,0);
	m_pFileData = new char[FileSize];
	DWORD Read;
	ReadFile(hFile,m_pFileData,FileSize,&Read,NULL);
	CloseHandle(hFile);
	char * pData = m_pFileData,* pCurrent = m_pFileData;
	pCurrent += 14;
	m_VertexNum = *(WORD*)pCurrent;
	pCurrent += 2;
	m_Vertex = (MS3D_VERTEX*)pCurrent;
	pCurrent += sizeof(MS3D_VERTEX) * m_VertexNum;
	m_FacesNum = *(WORD*)pCurrent;
	pCurrent += 2;
	m_Triangles = (MS3D_TRIANGLE*)pCurrent;
	pCurrent += sizeof(MS3D_TRIANGLE) * m_FacesNum;
	m_MeshNum = *(WORD*)pCurrent;
	pCurrent+=2;
	m_MS3D_Mesh = new MS3D_MESH[m_MeshNum];
	for(int i = 0;i < m_MeshNum;i++)
	{
		memcpy(&m_MS3D_Mesh[i],pCurrent,35);
		pCurrent += 35;
		m_MS3D_Mesh[i].Index = (WORD*)pCurrent;
		pCurrent += m_MS3D_Mesh[i].NumTriangles * sizeof(WORD);
		m_MS3D_Mesh[i].MaterialIndex = *(char*)pCurrent;
		pCurrent++;
	}
	m_MaterialNum = *(WORD*)pCurrent;
	pCurrent += 2;
	m_MS3D_Material = (MS3D_MATERIAL*)pCurrent;
	pCurrent += m_MaterialNum * sizeof(MS3D_MATERIAL);
	for(int i=0;i<m_MaterialNum;i++)
	{
	}
	//Load animation
	m_FPS = *(float*)pCurrent;
	pCurrent += sizeof(float);
	m_CurrentTime = *(float*)pCurrent;
	pCurrent += sizeof(float);
	m_TotalFrames = *(DWORD*)pCurrent;
	pCurrent += sizeof(DWORD);
	m_TotalTime = ((float)m_TotalFrames/m_FPS) * 1000;
	// load joint
	m_JointNum = *(WORD*)pCurrent;
	pCurrent += 2;
	if(m_JointNum)
	{
		m_MS3D_JOINT = new MS3D_JOINT[m_JointNum];
		for(int i = 0;i < m_JointNum;i++)
		{
			memcpy(&m_MS3D_JOINT[i],pCurrent,93);
			pCurrent += 93;
			m_MS3D_JOINT[i].f_Rot = (MS3D_KEY_FRAME*)pCurrent;
			pCurrent += m_MS3D_JOINT[i].num_Rotation * sizeof(MS3D_KEY_FRAME);
			m_MS3D_JOINT[i].f_Trans = (MS3D_KEY_FRAME*)pCurrent;
			pCurrent += m_MS3D_JOINT[i].num_Translation * sizeof(MS3D_KEY_FRAME);
		}
	}
	return 0;
}

int CMS3D::Draw(DWORD TimeDelt, float Scal)
{
	for(int i = 0;i < m_MeshNum;i++)
	{
		m_Render->SetSkin(m_Mesh[i].SkinID);
		m_Render->Render(m_Mesh[i].vertex,m_Mesh[i].VertexNum * sizeof(vertex_dynamic_instancing),m_Mesh[i].index,m_Mesh[i].IndexNum);
	}
	return 0;
}

CMS3D::CMS3D(IMRender * theRender):m_TimePassed(0),m_ag(0),m_Render(theRender)
{
}

int CMS3D::InitDXMesh(void)
{
	if(m_MeshNum)
	{
		m_Mesh = new Mesh[m_MeshNum];
	}
	for(int i = 0;i < m_MeshNum;i++)
	{
		m_Mesh[i].vertex = new vertex_dynamic_instancing[m_VertexNum];
		m_Mesh[i].VertexNum = m_VertexNum;
		for(int k = 0;k < m_VertexNum;k++)
		{
			m_Mesh[i].vertex[k].position = *(D3DXVECTOR3*)&m_Vertex[k].vertex;
		}
		m_Mesh[i].index = new WORD[m_MS3D_Mesh[i].NumTriangles * 3];
		m_Mesh[i].IndexNum = m_MS3D_Mesh[i].NumTriangles * 3;
		char * FileList[8];
		COLOR color(1.0,1.0,1.0,1.0);
		FileList[0] = NULL;
		FileList[1] = NULL;
		FileList[2] = NULL;
		FileList[3] = NULL;
		FileList[4] = NULL;
		FileList[5] = NULL;
		FileList[6] = NULL;
		FileList[7] = NULL;
		if(m_MS3D_Material[m_MS3D_Mesh[i].MaterialIndex].TextrueFile[0])
		{
			FileList[0] = m_MS3D_Material[m_MS3D_Mesh[i].MaterialIndex].TextrueFile;
		}
		m_Mesh[i].SkinID = m_Render->CreateSkin(&color,&color,&color,&color,5.0f,FileList);
		for(int j = 0;j < m_MS3D_Mesh[i].NumTriangles;j++)
		{
			m_Mesh[i].index[j * 3] = m_Triangles[m_MS3D_Mesh[i].Index[j]].index[0];
			m_Mesh[i].index[j * 3 + 2] = m_Triangles[m_MS3D_Mesh[i].Index[j]].index[1];
			m_Mesh[i].index[j * 3 + 1] = m_Triangles[m_MS3D_Mesh[i].Index[j]].index[2];
			m_Mesh[i].vertex[m_Mesh[i].index[j * 3]].u = m_Triangles[m_MS3D_Mesh[i].Index[j]].TextureCoorU[0];
			m_Mesh[i].vertex[m_Mesh[i].index[j * 3 + 1]].u = m_Triangles[m_MS3D_Mesh[i].Index[j]].TextureCoorU[1];
			m_Mesh[i].vertex[m_Mesh[i].index[j * 3 + 2]].u = m_Triangles[m_MS3D_Mesh[i].Index[j]].TextureCoorU[2];
			m_Mesh[i].vertex[m_Mesh[i].index[j * 3]].v = m_Triangles[m_MS3D_Mesh[i].Index[j]].TextureCoorV[0];
			m_Mesh[i].vertex[m_Mesh[i].index[j * 3 + 1]].v = m_Triangles[m_MS3D_Mesh[i].Index[j]].TextureCoorV[1];
			m_Mesh[i].vertex[m_Mesh[i].index[j * 3 + 2]].v = m_Triangles[m_MS3D_Mesh[i].Index[j]].TextureCoorV[2];
		}
	}
	return 0;
}

/*int CMS3D::InitJoint(void)
{
	for(int i = 0;i < m_JointNum;i++)
	{
		//set Parent_ID
		m_MS3D_JOINT[i].Parent_ID = -1;
		if(m_MS3D_JOINT[i].Parent[0])
		{
			for(int j = 0;j < m_JointNum;j++)
			{
				if(!strcmp(m_MS3D_JOINT[i].Parent,m_MS3D_JOINT[j].Name))
				{
					m_MS3D_JOINT[i].Parent_ID = j;
				}
			}
		}
	}
	//calc InitMatrix
	   // Calc Local Matrix
	for(int i = 0; i < m_JointNum;i++)
	{
		MS3D_JOINT * Joint;
		Joint = &m_MS3D_JOINT[i];
		D3DXMATRIX Rx,Ry,Rz,R;
		D3DXMATRIX T;
		D3DXMATRIX Tmp;
		// local
		D3DXMatrixRotationX(&Rx,Joint->init_Rotation[0]);
		D3DXMatrixRotationY(&Ry,Joint->init_Rotation[1]);
		D3DXMatrixRotationZ(&Rz,Joint->init_Rotation[2]);
		R = Rx * Ry * Rz;
		D3DXMatrixTranslation(&T,Joint->init_Translation[0],Joint->init_Translation[1],Joint->init_Translation[2]);
		Tmp = R * T;
		Joint->Matr_InitLocal = Joint->Matr_Local = Tmp;
	}
	 // calc Final Matrix
	for(int i = 0; i < m_JointNum;i++)
	{
		MS3D_JOINT * Joint = &m_MS3D_JOINT[i];
		m_MS3D_JOINT[i].Matr_Abs =  m_MS3D_JOINT[i].Matr_Local;
		while(Joint->Parent_ID != (DWORD)65535)
		{
			m_MS3D_JOINT[i].Matr_Abs = m_MS3D_JOINT[i].Matr_Abs * m_MS3D_JOINT[Joint->Parent_ID].Matr_Local;
			Joint = &m_MS3D_JOINT[Joint->Parent_ID];
		}
		m_MS3D_JOINT[i].Matr_Fanal = m_MS3D_JOINT[i].Matr_Abs;
	}
	return 0;
}
*/


/*int CMS3D::UpDataVertex(DWORD TimeDelt)
{
	Vertex * pV,*pJ,*pCV;
	D3DXMATRIX M,tM;
	D3DXVECTOR3 Vector,VectorNormal;
	D3DXMATRIX Rx,Ry,Rz,R,Rn;
	D3DXMATRIX T,Tn;
	D3DXMATRIX Tmp;
	float Param[3];
	m_TimePassed += TimeDelt;
	m_TimePassed = m_TimePassed % (DWORD)m_TotalTime;
	float MidTime = 0;
	//Calc LocalMatrix
	for(int i = 0; i < m_JointNum;i++)
	{
		MS3D_JOINT * Joint;
		Joint = &m_MS3D_JOINT[i];
		CalcRotParam(Joint,Param);
		//find Rotation Frame
		// local
     	D3DXMatrixRotationX(&Rx,Param[0]);
		D3DXMatrixRotationY(&Ry,Param[1]);
		D3DXMatrixRotationZ(&Rz,Param[2]);
//		D3DXMatrixRotationX(&Rx,Joint->init_Rotation[0]);
//		D3DXMatrixRotationY(&Ry,Joint->init_Rotation[1]);
//		D3DXMatrixRotationZ(&Rz,Joint->init_Rotation[2]);
		R = Rx * Ry * Rz;
		//translation
		CalcTransParam(Joint,Param);
		D3DXMatrixTranslation(&T,Param[0],Param[1],Param[2]);
//		D3DXMatrixTranslation(&T,Joint->init_Translation[0],Joint->init_Translation[1],Joint->init_Translation[2]);
		Tmp = R * T;
		Joint->Matr_Local = Tmp * Joint->Matr_InitLocal;
	}
	// calc Final Matrix
	for(int i = 0; i < m_JointNum;i++)
	{
		MS3D_JOINT * Joint = &m_MS3D_JOINT[i];
		m_MS3D_JOINT[i].Matr_Abs = m_MS3D_JOINT[i].Matr_Local;
		while(Joint->Parent_ID != (DWORD)65535)
		{
			m_MS3D_JOINT[i].Matr_Abs = m_MS3D_JOINT[i].Matr_Abs * m_MS3D_JOINT[Joint->Parent_ID].Matr_Local;
			Joint = &m_MS3D_JOINT[Joint->Parent_ID];
		}
		m_MS3D_JOINT[i].Matr_Fanal = m_MS3D_JOINT[i].Matr_Abs;
	}

	//Trnaform Vertex
	m_InitMesh->LockVertexBuffer(0,(LPVOID*)&pV);
	m_Mesh->LockVertexBuffer(0,(LPVOID*)&pCV);
	for(int i = 0;i < m_VertexNum;i++)
	{
		Vector = pV[i].vetx;
		VectorNormal = pV[i].norma;
		if(m_Vertex[i].Bone_ID != -1)
		{
			tM = m_MS3D_JOINT[m_Vertex[i].Bone_ID].Matr_Fanal;
			D3DXVec3TransformNormal(&pCV[i].norma,&VectorNormal,&tM);
			D3DXVec3TransformCoord(&pCV[i].vetx,&Vector,&tM);
		}
	}
	m_Mesh->UnlockVertexBuffer();
	m_InitMesh->UnlockVertexBuffer();

	//Transform Skeleton
	if(m_JointPoint)
	{
		m_JointPoint->Lock(0,sizeof(Vertex)*m_JointNum,(void**)&pJ,0);
		for(int i = 0;i < m_JointNum;i++)
		{
			pJ[i].vetx = D3DXVECTOR3(0,0,0);
			Vector = pJ[i].vetx;
			M = m_MS3D_JOINT[i].Matr_Fanal;
			D3DXVec3TransformCoord(&pJ[i].vetx,&Vector,&M);
		}
		m_JointPoint->Unlock();
	}
	return 0;
}
*/
/*int CMS3D::CalcRotParam(MS3D_JOINT * Joint,float * Param)
{
	int iFrame,iFrameNext;
	float MidTime;
	for(iFrame = 0;iFrame < Joint->num_Rotation;iFrame++)
	{
		if(m_TimePassed < Joint->f_Rot[iFrame].time * 1000)
		{
			break;
		}
	}
	iFrame--;
	if(iFrame >= Joint->num_Rotation)
	{
		iFrame = Joint->num_Rotation-1;
	}
	iFrameNext = iFrame + 1;
	if(iFrameNext >= Joint->num_Rotation)
	{
		iFrameNext = iFrame;
	}
	MidTime = m_TimePassed - Joint->f_Rot[iFrame].time * 1000;
	MidTime = MidTime/((Joint->f_Rot[iFrameNext].time - Joint->f_Rot[iFrame].time)*1000);
	if(iFrameNext == iFrame)
	{
		MidTime = 0;
	}
	if(MidTime)
	{
		Param[0] = Joint->f_Rot[iFrame].Param[0] + (Joint->f_Rot[iFrameNext].Param[0] - Joint->f_Rot[iFrame].Param[0]) * MidTime;
		Param[1] = Joint->f_Rot[iFrame].Param[1] + (Joint->f_Rot[iFrameNext].Param[1] - Joint->f_Rot[iFrame].Param[1]) * MidTime;
		Param[2] = Joint->f_Rot[iFrame].Param[2] + (Joint->f_Rot[iFrameNext].Param[2] - Joint->f_Rot[iFrame].Param[2]) * MidTime;
	}
	else
	{
		Param[0] = Joint->f_Rot[iFrame].Param[0];
		Param[1] = Joint->f_Rot[iFrame].Param[1];
		Param[2] = Joint->f_Rot[iFrame].Param[2];
	}
	return 0;
}

int CMS3D::CalcTransParam(MS3D_JOINT * Joint, float * Param)
{
	int iFrame,iFrameNext;
	float MidTime;
	for(iFrame = 0;iFrame < Joint->num_Translation;iFrame++)
	{
		if(m_TimePassed < Joint->f_Trans[iFrame].time * 1000)
		{
			break;
		}
	}
	iFrame--;
	if(iFrame >= Joint->num_Translation)
	{
		iFrame = Joint->num_Translation-1;
	}
	iFrameNext = iFrame + 1;
	if(iFrameNext >= Joint->num_Translation)
	{
		iFrameNext = iFrame;
	}
	MidTime = m_TimePassed - Joint->f_Trans[iFrame].time * 1000;
	MidTime = MidTime/((Joint->f_Trans[iFrameNext].time - Joint->f_Trans[iFrame].time)*1000);
	if(iFrameNext == iFrame)
	{
		MidTime = 0;
	}
	if(MidTime)
	{
		Param[0] = Joint->f_Trans[iFrame].Param[0] + (Joint->f_Trans[iFrameNext].Param[0] - Joint->f_Trans[iFrame].Param[0]) * MidTime;
		Param[1] = Joint->f_Trans[iFrame].Param[1] + (Joint->f_Trans[iFrameNext].Param[1] - Joint->f_Trans[iFrame].Param[1]) * MidTime;
		Param[2] = Joint->f_Trans[iFrame].Param[2] + (Joint->f_Trans[iFrameNext].Param[2] - Joint->f_Trans[iFrame].Param[2]) * MidTime;
	}
	else
	{
		Param[0] = Joint->f_Trans[iFrame].Param[0];
		Param[1] = Joint->f_Trans[iFrame].Param[1];
		Param[2] = Joint->f_Trans[iFrame].Param[2];
	}
	return 0;
}

int CMS3D::InitInstanceData(void)
{
	m_d3d9Dev->CreateVertexBuffer(5 * sizeof(InstanceData),0,0,D3DPOOL_MANAGED,&m_InstanceData,0);
	InstanceData * Data;
	m_InstanceData->Lock(0,5 * sizeof(InstanceData),(void**)&Data,0);
	for(int i = 0;i < 5;i ++)
	{
		Data[i].y = Data[i].z = Data[i].r = Data[i].x =  i * 50;
	}
	m_InstanceData->Unlock();
	return 0;
}
*/
