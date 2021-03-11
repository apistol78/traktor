#pragma once

#include <Core/Object.h>

class T_DLLEXPORT Example : public traktor::Object
{
	T_RTTI_CLASS;

public:
	void sayHelloWorld();
};
