#include "Model/Model.h"
#include "Model/Operations/Transform.h"

namespace traktor
{
	namespace model
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.Transform", Transform, IModelOperation)

Transform::Transform(const Matrix44& tf)
:	m_transform(tf)
{
}

bool Transform::apply(Model& model) const
{
	AlignedVector< Vector4 > positions = model.getPositions();
	for (AlignedVector< Vector4 >::iterator i = positions.begin(); i != positions.end(); ++i)
		*i = m_transform * (*i);
	model.setPositions(positions);

	AlignedVector< Vector4 > normals = model.getNormals();
	for (AlignedVector< Vector4 >::iterator i = normals.begin(); i != normals.end(); ++i)
		*i = m_transform * (*i);
	model.setNormals(normals);
	return true;
}

	}
}
