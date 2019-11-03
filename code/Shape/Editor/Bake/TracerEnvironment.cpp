#include "Shape/Editor/Bake/TracerEnvironment.h"

namespace traktor
{
	namespace shape
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.TracerEnvironment", TracerEnvironment, Object)

TracerEnvironment::TracerEnvironment(const IProbe* environment)
	: m_environment(environment)
{
}

	}
}
