#include "TestComponent.h"
#include <stdio.h>

USING_ALLOCATER(TestComponent)

TestComponent::TestComponent(Context * context_) :Component(context_)
{
}


TestComponent::~TestComponent()
{
}


int TestComponent::HandleEvent(Event *Evt) {

	printf("reduce health by %d, the attacker is %x\n", (*Evt)[String("param1")].as<int>(), (unsigned int)(*Evt)[String("attacker")].as<GameObject*>());
	return 0;
}
