#include "Render/Ps3/LocalMemoryObject.h"

namespace traktor
{
	namespace render
	{

LocalMemoryObject::LocalMemoryObject()
:	m_immutable(false)
,	m_alignment(0)
,	m_size(0)
,	m_pointer(0)
,	m_offset(0)
{
}

	}
}
