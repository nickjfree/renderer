#include "StdAfx.h"
#include "Include\Rect2D.h"

using SceneManager::CRect2D;
using SceneManager::CPoint2D;

CRect2D::CRect2D(void)
{
}

CRect2D::CRect2D(float ux, float uz, float lx, float lz)
{
	m_UL.m_x = ux;
	m_UL.m_z = uz;
	m_LR.m_x = lx;
	m_LR.m_z = lz;
}

CRect2D::CRect2D(CPoint2D ulp,CPoint2D lrp)
{
	m_UL = ulp;
	m_LR = lrp;
}

CRect2D::~CRect2D(void)
{
}

bool SceneManager::CRect2D::IsPointInside(CPoint2D Point)
{
	if(Point.m_x - m_UL.m_x >=0 && Point.m_z - m_UL.m_z >=0 &&
		m_LR.m_x - Point.m_x >= 0 && m_LR.m_z - Point.m_z >= 0)
	{
		return true;
	}
	return false;
}

bool SceneManager::CRect2D::IsRect2DInside(CRect2D Rect)
{
	if(Rect.m_UL.m_x >= this->m_UL.m_x && Rect.m_UL.m_z >= this->m_UL.m_z
		&& Rect.m_LR.m_x <= this->m_LR.m_x && Rect.m_LR.m_z <= this->m_LR.m_z)
	{
		return true;
	}
	return false;
}

bool SceneManager::CRect2D::IsRect2DTouch(CRect2D Rect)
{
	if(IsPointInside(Rect.UpLeft()) || IsPointInside(Rect.UpRight())
		|| IsPointInside(Rect.LowLeft()) ||IsPointInside(Rect.LowRight()))
		return true;
	if(Rect.IsPointInside(this->UpLeft()) || Rect.IsPointInside(this->UpRight())
		|| Rect.IsPointInside(this->LowLeft()) || Rect.IsPointInside(this->LowRight()))
		return true;
	return false;
}

bool SceneManager::CRect2D::IsRect2DOut(CRect2D Rect)
{
	return false;
}

CPoint2D SceneManager::CRect2D::Center(void)
{
	return CPoint2D((m_UL.m_x + m_LR.m_x)/2.0f,(m_UL.m_z + m_LR.m_z)/2.0f);
}

CPoint2D SceneManager::CRect2D::UpLeft(void)
{
	return m_UL;
}

CPoint2D SceneManager::CRect2D::UpRight(void)
{
	return CPoint2D(m_LR.m_x,m_UL.m_z);
}

CPoint2D SceneManager::CRect2D::LowLeft(void)
{
	return CPoint2D(m_UL.m_x,m_LR.m_z);
}

CPoint2D SceneManager::CRect2D::LowRight(void)
{
	return m_LR;
}
