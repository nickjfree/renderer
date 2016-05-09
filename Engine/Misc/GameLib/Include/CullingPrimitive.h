#ifndef __CULLING_PRIMITIVE__
#define __CULLING_PRIMITIVE__


#include "cullingproxy.h"
#include "MathLib.h"

namespace CullingSystem {


class CCullingPoint : public CCullingProxy
{
public:
	float     x,y,z;
public:
	CCullingPoint(void);
	CCullingPoint(float x,float y,float z);
	virtual ~CCullingPoint(void);
};

class CCullingAARect : public CCullingProxy
{
public:
	CCullingPoint min;
	CCullingPoint max;
public:
	CCullingAARect(void);
	CCullingAARect(CCullingPoint& min, CCullingPoint& max);
	virtual ~CCullingAARect(void);
	CCullingPoint UpLeft();
	CCullingPoint UpRight();
	CCullingPoint LowLeft();
	CCullingPoint LowRight();
	CCullingPoint Center(); 
	CCullingPoint LeftCenter();
	CCullingPoint UpCenter();
	CCullingPoint RightCenter();
	CCullingPoint LowCenter();
	float Length();
	float Width();
	CCullingPoint DirectionMin(Vector3& Direction);
	CCullingPoint DirectionMax(Vector3& Direction);
	// virtual funcs
	virtual int Collision(CCullingProxy * Proxy);
};

class CCullingPlane : public CCullingProxy
{
public:
	Vector3 normal;
	float  d;
public:
	CCullingPlane(void);
	CCullingPlane(float a,float b,float c,float d);
	virtual ~CCullingPlane(void);
};

class CCullingAABB : public CCullingProxy
{
public:
	CCullingPoint min;
	CCullingPoint max;
public:
	CCullingAABB(void);
	CCullingAABB(CCullingPoint& min, CCullingPoint& max);
	virtual ~CCullingAABB(void);
	CCullingPoint Center(); 
	float Length();
	float Width();
	float Height();
	CCullingPoint DirectionMin(Vector3& Direction);
	CCullingPoint DirectionMax(Vector3& Direction);
	// virtual funcs
	virtual int Collision(CCullingProxy * Proxy);
};

class CCullingSphere : public CCullingProxy
{
public:
	CCullingPoint center;
	float radius;
public:
	CCullingSphere(void) {};
	CCullingSphere(CCullingPoint& center, float radius);
	virtual ~CCullingSphere(void){};
	CCullingPoint Center(); 
	float Radius() {return radius;};
	// virtual funcs
	virtual int Collision(CCullingProxy * Proxy);
};




} // end namespace

#endif
