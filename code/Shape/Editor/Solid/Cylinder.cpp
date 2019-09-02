#include "Core/Math/Const.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Shape/Editor/Solid/Cylinder.h"

namespace traktor
{
	namespace shape
	{
	
T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.shape.Cylinder", 0, Cylinder, IShape)

Cylinder::Cylinder()
:	m_length(1.0f)
,   m_radius(0.5f)
,   m_faces(16)
{
}

Ref< model::Model > Cylinder::createModel() const
{
 //   auto& top = outWindings.push_back();
 //   auto& bottom = outWindings.push_back();

 //   for (int32_t i = 0; i < m_faces; ++i)
 //   {
 //       float phi1 = TWO_PI * float(i) / m_faces;
 //       float phi2 = TWO_PI * float(i + 1) / m_faces;

 //       top.push(Vector4(
 //           sin(phi1) * m_radius,
 //           m_length / 2.0f,
 //           cos(phi1) * m_radius,
 //           1.0f
 //       ));
 //       bottom.push(Vector4(
 //           sin(phi1) * m_radius,
 //           -m_length / 2.0f,
 //           cos(phi1) * m_radius,
 //           1.0f
 //       ));

 //       auto& w = outWindings.push_back();
 //       w.resize(4);
 //       w[0] = Vector4(
 //           sin(phi1) * m_radius,
 //           m_length / 2.0f,
 //           cos(phi1) * m_radius,
 //           1.0f
 //       );
 //       w[1] = Vector4(
 //           sin(phi2) * m_radius,
 //           m_length / 2.0f,
 //           cos(phi2) * m_radius,
 //           1.0f
 //       );
 //       w[2] = Vector4(
 //           sin(phi2) * m_radius,
 //           -m_length / 2.0f,
 //           cos(phi2) * m_radius,
 //           1.0f
 //       );
 //       w[3] = Vector4(
 //           sin(phi1) * m_radius,
 //           -m_length / 2.0f,
 //           cos(phi1) * m_radius,
 //           1.0f
 //       );
 //   }

 //   top.flip();
	//return true;

	return nullptr;
}

void Cylinder::createAnchors(AlignedVector< Vector4 >& outAnchors) const
{
}

void Cylinder::serialize(ISerializer& s)
{
	s >> Member< float >(L"length", m_length, AttributeRange(0.0f));
    s >> Member< float >(L"radius", m_radius, AttributeRange(0.0f));
    s >> Member< int32_t >(L"faces", m_faces, AttributeRange(3));
}

	}
}