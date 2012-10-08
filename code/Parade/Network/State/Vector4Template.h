#ifndef traktor_parade_Vector4Template_H
#define traktor_parade_Vector4Template_H

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

class T_DLLCLASS Vector4Template : public IValueTemplate
{
	T_RTTI_CLASS;

public:
	Vector4Template();

	explicit Vector4Template(float w);

	virtual void pack(BitWriter& writer, const IValue* V) const;

	virtual Ref< const IValue > unpack(BitReader& reader) const;

	virtual Ref< const IValue > extrapolate(const IValue* Vn1, float Tn1, const IValue* V0, float T0, float T) const;

	virtual Ref< const IValue > extrapolate(const IValue* Vn2, float Tn2, const IValue* Vn1, float Tn1, const IValue* V0, float T0, float T) const;

private:
	float m_w;
	bool m_immutableW;
};

	}
}

#endif	// traktor_parade_Vector4Template_H
