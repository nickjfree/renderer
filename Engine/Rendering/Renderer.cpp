#include "Renderer.h"


USING_ALLOCATER(Renderer);

Renderer::Renderer(Context* context) : Component(context) {
}


Renderer::~Renderer() {
}

int Renderer::OnTransform(GameObject* GameObj) {
	return 0;
}