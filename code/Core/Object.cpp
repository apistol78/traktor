#include "Core/Object.h"
#if defined(T_DEBUG_OBJECT_NOT_REFERENCED)
#include "Core/Heap/Heap.h"
#endif

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS_NB(L"traktor.Object", Object)

Object::Object()
:	m_managed(false)
{
}

Object::~Object()
{
#if defined(T_DEBUG_OBJECT_NOT_REFERENCED)
	uint32_t referenceCount = Heap::getInstance().getReferenceCount(this);
	if (referenceCount != 0)
		T_FATAL_ERROR;
#endif
}

}
