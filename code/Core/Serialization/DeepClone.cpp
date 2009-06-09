#include "Core/Serialization/DeepClone.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Core/Io/DynamicMemoryStream.h"

namespace traktor
{
	namespace
	{

const uint32_t c_initialCapacity = 4096;	//!< Estimated initial size of clone; used to reduce number of allocation of m_copy array.

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.DeepClone", DeepClone, Object)

DeepClone::DeepClone(const Serializable* source)
{
	m_copy.reserve(c_initialCapacity);
	DynamicMemoryStream stream(m_copy, false, true);
	BinarySerializer(&stream).writeObject(source);
}

Serializable* DeepClone::create()
{
	DynamicMemoryStream stream(m_copy, true, false);
	return BinarySerializer(&stream).readObject();
}

}
