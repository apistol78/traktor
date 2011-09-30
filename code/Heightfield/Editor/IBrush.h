#ifndef traktor_hf_IBrush_H
#define traktor_hf_IBrush_H

#include "Core/Object.h"
#include "Core/Math/Vector4.h"

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

class HeightfieldCompositor;

class T_DLLCLASS IBrush : public Object
{
	T_RTTI_CLASS;

protected:
	friend class HeightfieldCompositor;

	virtual void apply(HeightfieldCompositor* compositor, const Vector4& at, float is) const = 0;
};

	}
}

#endif	// traktor_hf_IBrush_H
