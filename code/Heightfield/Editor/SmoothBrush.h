#ifndef traktor_hf_SmoothBrush_H
#define traktor_hf_SmoothBrush_H

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

class T_DLLCLASS SmoothBrush : public IBrush
{
	T_RTTI_CLASS;

public:
	SmoothBrush(float radius);

protected:
	virtual void apply(HeightfieldCompositor* compositor, const Vector4& at, Region& outDirty) const;

private:
	float m_radius;
};

	}
}

#endif	// traktor_hf_SmoothBrush_H
