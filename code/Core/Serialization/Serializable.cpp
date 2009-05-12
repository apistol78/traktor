#include "Core/Serialization/Serializable.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.Serializable", Serializable, Object);

int Serializable::getVersion() const
{
	return 0;
}

}
