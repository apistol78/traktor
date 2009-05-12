#include "Core/Serialization/DeepClone.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Core/Io/DynamicMemoryStream.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.DeepClone", DeepClone, Object)

DeepClone::DeepClone(const Serializable* source)
{
	DynamicMemoryStream stream(m_copy, false, true);
	BinarySerializer(&stream).writeObject(source);
}

Serializable* DeepClone::create()
{
	DynamicMemoryStream stream(m_copy, true, false);
	return BinarySerializer(&stream).readObject();
}

}
