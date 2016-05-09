#include "StdAfx.h"
#include "Include\Point.h"

using SceneManager::CPoint2D;
using SceneManager::CPoint3D;

CPoint2D::CPoint2D(void):m_x(0),m_z(0)
{
}

CPoint2D::CPoint2D(float x,float z):m_x(x),m_z(z)
{
}
CPoint2D::~CPoint2D(void)
{
}

CPoint3D::CPoint3D(void)
{
}

CPoint3D::~CPoint3D(void)
{
}

CPoint2D SceneManager::CPoint2D::Middle(CPoint2D Target)
{
	return CPoint2D((m_x + Target.m_x)/2.0f,(m_z + Target.m_z)/2.0f);
}
