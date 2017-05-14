#ifndef __PHYSICS_ENGINE__
#define __PHYSICS_ENGINE__


#include "Core\System.h"
#include "Container\List.h"
#include "Math\LinearMath.h"

#define BT_NO_SIMD_OPERATOR_OVERLOADS

#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"
//#include "PhysicsObject.h"

class PhysicsObject;

class PhysicsSystem : public System {

private:
	// bullet classes
	btDefaultCollisionConfiguration* collisionConfiguration;

	///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
	btCollisionDispatcher* dispatcher;

	///btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
	btBroadphaseInterface* overlappingPairCache;

	///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
	btSequentialImpulseConstraintSolver* solver;

	btDiscreteDynamicsWorld* dynamicsWorld;
	
	// a list to keep track of all the physics objects
	List<PhysicsObject> Objects;

public:
	PhysicsSystem(Context * context);

	virtual ~PhysicsSystem();
	// init
	virtual int Initialize();
	// update
	virtual int Update(int ms);
	// shutdown 
	virtual int Shutdown();
	// add rigide body
	void AddPhysicsObject(PhysicsObject * object);
	// get btworld
	inline btDiscreteDynamicsWorld * GetWorld() { return dynamicsWorld; };
};

#endif