#include "Include\CullingPrimitive.h"
#include "CullingManager.h"


using namespace CullingSystem;

CCullingPoint::CCullingPoint(void)
{
	m_Type = CCullingManager::POINT;
}

CCullingPoint::CCullingPoint(float ix,float iy,float iz):x(ix),y(iy),z(iz)
{
	m_Type = CCullingManager::POINT;
}


CCullingPoint::~CCullingPoint(void)
{
}


CCullingAARect::CCullingAARect(void)
{
	m_Type = CCullingManager::AARECT;
}

CCullingAARect::CCullingAARect(CCullingPoint& min, CCullingPoint& max)
{
	m_Type = CCullingManager::AARECT;
	this->min = min;
	this->max = max;
}

CCullingAARect::~CCullingAARect(void)
{
}

CCullingPoint CCullingAARect::UpLeft()
{
	return min;
}

CCullingPoint CCullingAARect::UpRight()
{
	return CCullingPoint(max.x,0 ,min.z);
}

CCullingPoint CCullingAARect::LowLeft()
{
	return CCullingPoint(min.x,0 ,max.z);
}

CCullingPoint CCullingAARect::LowRight()
{
	return max;
}

CCullingPoint CCullingAARect::Center()
{
	return CCullingPoint((min.x + max.x) * 0.5,0 ,(min.x + max.x) * 0.5);
}

CCullingPoint  CCullingAARect::LeftCenter()
{
	return CCullingPoint(min.x, 0, (min.z + max.z) * 0.5);
}

CCullingPoint  CCullingAARect::UpCenter()
{
	return CCullingPoint((min.x + max.z) * 0.5, 0, min.z);
}

CCullingPoint  CCullingAARect::RightCenter()
{
	return CCullingPoint(max.x, 0, (min.z + max.z) * 0.5);
}

CCullingPoint  CCullingAARect::LowCenter()
{
	return CCullingPoint((min.x + max.z) * 0.5, 0, max.z);
}

float CCullingAARect::Length()
{
	return max.x-min.x;
}
float CCullingAARect::Width()
{
	return max.z-min.z;
}

CCullingPoint CCullingAARect::DirectionMin(Vector3& Direction)
{
	float length = Length();
	float width = Width();
	float unitx = Direction._x>0?-length:length;
	float unitz = Direction._z>0?-width:width;
	CCullingPoint center = Center();
	return CCullingPoint(center.x + unitx, 0, center.z + unitz);
}

CCullingPoint CCullingAARect::DirectionMax(Vector3& Direction)
{
	float length = Length();
	float width = Width();
	float unitx = Direction._x>0?length:-length;
	float unitz = Direction._z>0?width:-width;
	CCullingPoint center = Center();
	return CCullingPoint(center.x + unitx, 0, center.z + unitz);
}

int CCullingAARect::Collision(CCullingProxy * Proxy)
{

	return 0;
}

CCullingPlane::CCullingPlane(void)
{
	m_Type = CCullingManager::PLANE;
}

CCullingPlane::CCullingPlane(float ia,float ib,float ic,float id):normal(ia,ib,ic),d(id)
{
	m_Type = CCullingManager::PLANE;
}

CCullingPlane::~CCullingPlane(void)
{
}

CCullingAABB::CCullingAABB(void)
{
	m_Type = CCullingManager::AABB;
}

CCullingAABB::CCullingAABB(CCullingPoint& min, CCullingPoint& max)
{
	m_Type = CCullingManager::AABB;
	this->min = min;
	this->max = max;
}


CCullingAABB::~CCullingAABB(void)
{
}

float CCullingAABB::Length()
{
	return max.x-min.x;
}

float CCullingAABB::Width()
{
	return max.z-min.z;
}

float CCullingAABB::Height()
{
	return max.y-min.y;
}

CCullingPoint CCullingAABB::Center()
{
	return CCullingPoint((min.x + max.x) * 0.5, (min.y + max.y) * 0.5 ,(min.z + max.z) * 0.5);
}

CCullingPoint CCullingAABB::DirectionMin(Vector3& Direction)
{
	float length = Length() * 0.5f;
	float width = Width() * 0.5f;
	float height = Height() * 0.5f;
	float unitx = Direction._x>0?-length:length;
	float unity = Direction._y>0?-height:height;
	float unitz = Direction._z>0?-width:width;
	CCullingPoint center = Center();
	return CCullingPoint(center.x + unitx, center.y + unity, center.z + unitz);
}

CCullingPoint CCullingAABB::DirectionMax(Vector3& Direction)
{
	float length = Length() * 0.5f;
	float width = Width() * 0.5f;
	float height = Height() * 0.5f;
	float unitx = Direction._x>0?length:-length;
	float unity = Direction._y>0?height:-height;
	float unitz = Direction._z>0?width:-width;
	CCullingPoint center = Center();
	return CCullingPoint(center.x + unitx, center.y + unity, center.z + unitz);
}

int CCullingAABB::Collision(CCullingProxy * Proxy)
{
	return 0;
}

CCullingSphere::CCullingSphere(CCullingPoint& center, float radius):center(center),radius(radius)
{
}


int CCullingSphere::Collision(CCullingProxy * Proxy)
{
	return CCullingManager::GetCullingManager()->SphereCollision(this, Proxy);
}

CCullingPoint CCullingSphere::Center()
{
	return center;
}