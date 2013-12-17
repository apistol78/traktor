#ifndef traktor_net_FloatTemplate_H
#define traktor_net_FloatTemplate_H

#include <climits>
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

enum FloatTemplatePrecision
{
	Ftp32,
	Ftp16,
	Ftp8,
	Ftp4
};

class T_DLLCLASS FloatTemplate : public IValueTemplate
{
	T_RTTI_CLASS;

public:
	FloatTemplate(float threshold = std::numeric_limits< float >::max());

	explicit FloatTemplate(float threshold, float min, float max, FloatTemplatePrecision precision);

	virtual const TypeInfo& getValueType() const;

	virtual void pack(BitWriter& writer, const IValue* V) const;

	virtual Ref< const IValue > unpack(BitReader& reader) const;

	virtual Ref< const IValue > extrapolate(const IValue* Vn2, float Tn2, const IValue* Vn1, float Tn1, const IValue* V0, float T0, float T) const;

	virtual bool threshold(const IValue* Vn1, const IValue* V) const;

private:
	float m_threshold;
	float m_min;
	float m_max;
	FloatTemplatePrecision m_precision;
};

	}
}

#endif	// traktor_net_FloatTemplate_H
