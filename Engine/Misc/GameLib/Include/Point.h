#ifndef __POINT_H__
#define __POINT_H__

namespace SceneManager{

class CPoint2D
{
public:
	float m_x,m_z;
public:
	CPoint2D(void);
	CPoint2D(float x,float z);
	~CPoint2D(void);
	CPoint2D Middle(CPoint2D Target);
};

class CPoint3D
{
public:
	CPoint3D(void);
	~CPoint3D(void);
};

}// end namespace

#endif
