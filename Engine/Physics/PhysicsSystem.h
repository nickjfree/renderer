#ifndef __PHYSICS_ENGINE__
#define __PHYSICS_ENGINE__


#include "Core\System.h"
#include "Container\List.h"
#include "Math\LinearMath.h"

#define BT_NO_SIMD_OPERATOR_OVERLOADS

#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"
#include "BulletDynamics/Character/btKinematicCharacterController.h"
#include "BulletCollision/CollisionDispatch/btGhostObject.h"
//#include "PhysicsObject.h"

class PhysicsObject;

class PhysicsSystem : public System {

	OBJECT(PhysicsSystem);
	DECLARE_ALLOCATER(PhysicsSystem);
	BASEOBJECT(PhysicsSystem);
private:
	// bullet classes
	btDefaultCollisionConfiguration* collisionConfiguration;

	///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
	btCollisionDispatcher* dispatcher;

	///btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
	btBroadphaseInterface* overlappingPairCache;

	// the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
	btSequentialImpulseConstraintSolver* solver;
	
	// the world
	btDiscreteDynamicsWorld* dynamicsWorld;

	// ghostobect callback
	btGhostPairCallback* ghostPairCallback;

	// a list to keep track of all the physics objects
	List<PhysicsObject> Objects;
	// destroyed objects
	Vector<PhysicsObject*> Destroyed;
	// for debug window
	Vector3 Look, Up, Right, Eye;
	// debug window handles
	HWND hWnd;
	HDC hDC;
	HGLRC hRC;
private:
	// create debug window
	void CreateDebugWindow();
	// enable the debug window
	void EnableDebug(HWND hWnd, HDC* hDC, HGLRC* hRC);
	// draw debug
	void DrawDebug();
	// detect collision
	void DetectCollision();
public:
	PhysicsSystem(Context* context);
	// desctructor
	virtual ~PhysicsSystem();
	// init
	virtual int Initialize();
	// update
	virtual int Update(int ms);
	// shutdown 
	virtual int Shutdown();
	// add physics body
	void AddPhysicsObject(PhysicsObject* object);
	// remove physics objects
	void RemovePhysicsObject(PhysicsObject* object);
	// get btworld
	inline btDiscreteDynamicsWorld* GetWorld() { return dynamicsWorld; };
	// set debug view
	void SetDebugView(Vector3& Look, Vector3& Up, Vector3& Right, Vector3& Eye);
};

#endif