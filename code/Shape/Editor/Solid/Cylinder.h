#pragma once

#include "Shape/Editor/Solid/IShape.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SHAPE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace shape
	{
	
/*! Cylinder shape.
 * \ingroup Shape
 */
class T_DLLCLASS Cylinder : public IShape
{
	T_RTTI_CLASS;

public:
	Cylinder();

	virtual Ref< model::Model > createModel(db::Database* database) const override final;

	virtual void createAnchors(AlignedVector< Vector4 >& outAnchors) const override final;

	virtual void serialize(ISerializer& s) override final;

	const Guid* getMaterials() const { return m_materials; }

private:
	float m_length;
    float m_radius;
    int32_t m_faces;
	Guid m_materials[3];
};

	}
}