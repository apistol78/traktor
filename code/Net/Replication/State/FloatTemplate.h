#ifndef traktor_net_FloatTemplate_H
#define traktor_net_FloatTemplate_H

#include "Net/Replication/State/IValueTemplate.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_NET_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace net
	{

class T_DLLCLASS FloatTemplate : public IValueTemplate
{
	T_RTTI_CLASS;

public:
	FloatTemplate(float errorScale);

	explicit FloatTemplate(float errorScale, float min, float max, bool lowPrecision);

	virtual const TypeInfo& getValueType() const;

	virtual void pack(BitWriter& writer, const IValue* V) const;

	virtual Ref< const IValue > unpack(BitReader& reader) const;

	virtual float error(const IValue* Vl, const IValue* Vr) const;

	virtual Ref< const IValue > extrapolate(const IValue* Vn2, float Tn2, const IValue* Vn1, float Tn1, const IValue* V0, float T0, float T) const;

private:
	float m_errorScale;
	float m_min;
	float m_max;
	bool m_lowPrecision;
};

	}
}

#endif	// traktor_net_FloatTemplate_H
