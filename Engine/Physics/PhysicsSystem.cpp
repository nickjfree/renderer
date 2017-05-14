#include "PhysicsSystem.h"
#include "PhysicsObject.h"


PhysicsSystem::PhysicsSystem(Context * context) : System(context) {
}


PhysicsSystem::~PhysicsSystem() {
}


int PhysicsSystem::Initialize() {
	// setup bullet physics configurations
	///collision configuration contains default setup for memory, collision setup. Advanced users can create their own configuration.
	collisionConfiguration = new btDefaultCollisionConfiguration();

	///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
	dispatcher = new	btCollisionDispatcher(collisionConfiguration);

	///btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
	overlappingPairCache = new btDbvtBroadphase();

	///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
	solver = new btSequentialImpulseConstraintSolver;

	dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);

	dynamicsWorld->setGravity(btVector3(0, -10, 0));

	// build a test plane
	//the ground is a cube of side 100 at position y = -56.
	//the sphere will hit it at y = -6, with center at -5
	{
		btCollisionShape* groundShape = new btBoxShape(btVector3(btScalar(1000.), btScalar(50.), btScalar(50.)));

		btTransform groundTransform;
		groundTransform.setIdentity();
		groundTransform.setOrigin(btVector3(0, -56, 0));

		btScalar mass(0.);

		//rigidbody is dynamic if and only if mass is non zero, otherwise static
		bool isDynamic = (mass != 0.f);

		btVector3 localInertia(0, 0, 0);
		if (isDynamic)
			groundShape->calculateLocalInertia(mass, localInertia);

		//using motionstate is optional, it provides interpolation capabilities, and only synchronizes 'active' objects
		btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, groundShape, localInertia);
		btRigidBody* body = new btRigidBody(rbInfo);

		//add the body to the dynamics world
		dynamicsWorld->addRigidBody(body);
	}
	return 0;
}

int PhysicsSystem::Update(int ms) {
	// do simulation
	dynamicsWorld->stepSimulation(ms/60.0f, 2);
	// update every component
	List<PhysicsObject>::Iterator Iter;
	for (Iter = Objects.Begin(); Iter != Objects.End(); Iter++) {
		PhysicsObject * obj = *Iter;
		obj->Update(ms);
	}
	return 0;
}

int PhysicsSystem::Shutdown() {
	return 0;
}

void PhysicsSystem::AddPhysicsObject(PhysicsObject * object) {
	Objects.Insert(object);
}

