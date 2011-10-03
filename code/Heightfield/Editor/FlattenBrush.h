#ifndef traktor_hf_FlattenBrush_H
#define traktor_hf_FlattenBrush_H

#include "Heightfield/Editor/IBrush.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_HEIGHTFIELD_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace hf
	{

class T_DLLCLASS FlattenBrush : public IBrush
{
	T_RTTI_CLASS;

public:
	FlattenBrush(float radius);

protected:
	virtual void apply(HeightfieldCompositor* compositor, const Vector4& at, Region& outDirty) const;

private:
	float m_radius;
	mutable height_t m_height;
	mutable bool m_first;
};

	}
}

#endif	// traktor_hf_FlattenBrush_H
