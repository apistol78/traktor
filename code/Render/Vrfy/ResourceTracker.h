#pragma once

#include "Core/Object.h"
#include "Core/Containers/SmallMap.h"

namespace traktor
{
	namespace render
	{

class ResourceTracker : public Object
{
public:
	void add(const Object* resource);

	void remove(const Object* resource);

	void alive();

private:
	struct Data
	{
		void* callstack[16] = { nullptr };
	};

	SmallMap< const Object*, Data > m_data;
};

	}
}