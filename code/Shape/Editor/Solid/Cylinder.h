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
	
class T_DLLCLASS Cylinder : public IShape
{
	T_RTTI_CLASS;

public:
	Cylinder();

	virtual bool createWindings(AlignedVector< Winding3 >& outWindings) const override final;

	virtual void createAnchors(AlignedVector< Vector4 >& outAnchors) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	float m_length;
    float m_radius;
    int32_t m_faces;
};

	}
}