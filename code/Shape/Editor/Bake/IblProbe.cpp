#include "Render/Editor/Texture/CubeMap.h"
#include "Shape/Editor/Bake/IblProbe.h"

namespace traktor
{
	namespace shape
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.IblProbe", IblProbe, IProbe)

IblProbe::IblProbe(const render::CubeMap* cubeMap)
:	m_cubeMap(cubeMap)
{
}

Color4f IblProbe::sample(const Vector4* directions, uint32_t count) const
{
	Color4f ibl(0.0f, 0.0f, 0.0f, 0.0f);
	for (uint32_t i = 0; i < count; ++i)
		ibl += m_cubeMap->get(directions[i].xyz0()) * directions[i].w();
	return ibl;
}

	}
}
