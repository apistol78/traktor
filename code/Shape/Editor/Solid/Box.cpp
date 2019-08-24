#include "Core/Math/Aabb3.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Shape/Editor/Solid/Box.h"

namespace traktor
{
	namespace shape
	{
	
T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.shape.Box", 0, Box, IShape)

Box::Box()
:	m_extent(1.0f, 1.0f, 1.0f)
{
}

bool Box::createWindings(AlignedVector< Winding3 >& outWindings) const
{
	const Scalar two(2.0f);

	Vector4 vertices[8];
	Aabb3(-m_extent / two, m_extent / two).getExtents(vertices);

	for (uint32_t i = 0; i < 6; ++i)
	{
		const int* face = Aabb3::getFaces() + i * 4;
		auto& w = outWindings.push_back();
		w.resize(4);
		w[0] = vertices[face[3]];
		w[1] = vertices[face[2]];
		w[2] = vertices[face[1]];
		w[3] = vertices[face[0]];
	}

	return true;
}

void Box::serialize(ISerializer& s)
{
	s >> Member< Vector4 >(L"extent", m_extent);
}

	}
}