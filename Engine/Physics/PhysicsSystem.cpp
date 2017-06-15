#include "PhysicsSystem.h"
#include "PhysicsObject.h"
#include "GLDebuger\GLDebugDrawer.h"
#include <gl\GL.h>
#include <gl\GLU.h>


PhysicsSystem::PhysicsSystem(Context * context) : System(context) {
}


PhysicsSystem::~PhysicsSystem() {
}


void PhysicsSystem::CreateDebugWindow() {
	WNDCLASS wc;
	BOOL quit = FALSE;
	float theta = 0.0f;
	// register window class
	wc.style = CS_OWNDC;
	wc.lpfnWndProc = DefWindowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = NULL;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = L"Physics Debuging";
	RegisterClass(&wc);
		// create main window

	hWnd = CreateWindow(
		L"Physics Debuging", L"Physics Debuging",
		WS_CAPTION | WS_VISIBLE | WS_OVERLAPPEDWINDOW,
		//		0, 0, 640, 480,
		0, 0, 1024, 768,
		NULL, NULL, NULL, NULL);
	// enable OpenGL for the window
	EnableDebug(hWnd, &hDC, &hRC);
	GLDebugDrawer * debugDraw = new GLDebugDrawer();

	if (dynamicsWorld) {
		dynamicsWorld->setDebugDrawer(debugDraw);
		debugDraw->setDebugMode(btIDebugDraw::DBG_DrawWireframe);
	}
}

void PhysicsSystem::EnableDebug(HWND hWnd, HDC * hDC, HGLRC * hRC) {
	PIXELFORMATDESCRIPTOR pfd;
	int format;
	// get the device context (DC)
	*hDC = GetDC(hWnd);
	// set the pixel format for the DC
	ZeroMemory(&pfd, sizeof(pfd));
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 16;
	pfd.cStencilBits = 1;
	pfd.iLayerType = PFD_MAIN_PLANE;
	format = ChoosePixelFormat(*hDC, &pfd);
	SetPixelFormat(*hDC, format, &pfd);
	// create and enable the render context (RC)
	*hRC = wglCreateContext(*hDC);
	wglMakeCurrent(*hDC, *hRC);
}

void PhysicsSystem::DrawDebug() {
	glClearColor(.7f, 0.7f, 0.7f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);
	// update camera
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	float aspect = 1024 / 768.0f;
	glFrustum(-aspect, aspect, -1.0, 1.0, 1.0, 10000.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	Vector3 Ref = Eye + Look;
	gluLookAt(Eye.x, Eye.y, Eye.z,
		Ref.x, Ref.y, Ref.z,
		Up.x, Up.y, Up.z);

	if (dynamicsWorld && dynamicsWorld->getDebugDrawer()) {
		dynamicsWorld->debugDrawWorld();
	}
	glFlush();
	SwapBuffers(hDC);
}

void PhysicsSystem::SetDebugView(Vector3& Look, Vector3& Up, Vector3& Right, Vector3& Eye) {
	this->Look = Look;
	this->Up = Up;
	this->Right = Right;
	this->Eye = Eye;
};

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
		btCollisionShape* groundShape = new btBoxShape(btVector3(btScalar(1000.), btScalar(50.), btScalar(1000.)));

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
	// debug window
#ifdef DEBUG_PHYSICS
	CreateDebugWindow();
#endif
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
#ifdef DEBUG_PHYSICS
	// debug
	DrawDebug();
#endif
	return 0;
}

int PhysicsSystem::Shutdown() {
	return 0;
}

void PhysicsSystem::AddPhysicsObject(PhysicsObject * object) {
	Objects.Insert(object);
}

