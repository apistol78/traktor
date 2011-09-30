#ifndef traktor_hf_RoundBrush_H
#define traktor_hf_RoundBrush_H

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

class T_DLLCLASS RoundBrush : public IBrush
{
	T_RTTI_CLASS;

public:
	RoundBrush(float radius, float pressure);

protected:
	virtual void apply(HeightfieldCompositor* compositor, const Vector4& at, float is) const;

private:
	float m_radius;
	float m_pressure;
};

	}
}

#endif	// traktor_hf_RoundBrush_H
