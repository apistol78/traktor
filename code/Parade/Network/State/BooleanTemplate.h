#ifndef traktor_parade_BooleanTemplate_H
#define traktor_parade_BooleanTemplate_H

#include "Parade/Network/State/IValueTemplate.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PARADE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace parade
	{

class T_DLLCLASS BooleanTemplate : public IValueTemplate
{
	T_RTTI_CLASS;

public:
	BooleanTemplate(float threshold = 0.5f);

	virtual void pack(BitWriter& writer, const IValue* V) const;

	virtual Ref< const IValue > unpack(BitReader& reader) const;

	virtual Ref< const IValue > extrapolate(const IValue* Vn2, float Tn2, const IValue* Vn1, float Tn1, const IValue* V0, float T0, const IValue* V, float T) const;

private:
	float m_threshold;
};

	}
}

#endif	// traktor_parade_BooleanTemplate_H
