#ifndef traktor_net_BooleanTemplate_H
#define traktor_net_BooleanTemplate_H

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

class T_DLLCLASS BooleanTemplate : public IValueTemplate
{
	T_RTTI_CLASS;

public:
	BooleanTemplate(float threshold = 0.0f);

	virtual const TypeInfo& getValueType() const;

	virtual void pack(BitWriter& writer, const IValue* V) const;

	virtual Ref< const IValue > unpack(BitReader& reader) const;

	virtual Ref< const IValue > extrapolate(const IValue* Vn2, float Tn2, const IValue* Vn1, float Tn1, const IValue* V0, float T0, float T) const;

private:
	float m_threshold;
};

	}
}

#endif	// traktor_net_BooleanTemplate_H
