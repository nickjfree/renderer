#ifndef __RECT2D__
#define __RECT2D__


#include "point.h"

namespace SceneManager{

class CRect2D
{
public:
	CPoint2D m_UL;
	CPoint2D m_LR;
public:
	CRect2D(void);
	CRect2D(float ux,float uz,float lx,float lz);
	CRect2D(CPoint2D ulp,CPoint2D lrp);
	~CRect2D(void);
	bool IsPointInside(CPoint2D Point);
	bool IsRect2DInside(CRect2D Rect);
	bool IsRect2DTouch(CRect2D Rect);
	bool IsRect2DOut(CRect2D Rect);
	CPoint2D Center(void);
	CPoint2D UpLeft(void);
	CPoint2D UpRight(void);
	CPoint2D LowLeft(void);
	CPoint2D LowRight(void);
	float    Width(void) {return m_LR.m_x - m_UL.m_x;}
	float    Height(void){return m_LR.m_z - m_UL.m_z;}
};

}

#endif