#ifndef traktor_net_BodyStateTemplate_H
#define traktor_net_BodyStateTemplate_H

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

class T_DLLCLASS BodyStateTemplate : public IValueTemplate
{
	T_RTTI_CLASS;

public:
	BodyStateTemplate(float linearError = 8.0f, float angularError = 24.0f);

	virtual const TypeInfo& getValueType() const T_OVERRIDE T_FINAL;

	virtual void pack(BitWriter& writer, const IValue* V) const T_OVERRIDE T_FINAL;

	virtual Ref< const IValue > unpack(BitReader& reader) const T_OVERRIDE T_FINAL;

	virtual Ref< const IValue > extrapolate(const IValue* Vn2, float Tn2, const IValue* Vn1, float Tn1, const IValue* V0, float T0, float T) const T_OVERRIDE T_FINAL;

	virtual bool threshold(const IValue* Vn1, const IValue* V) const T_OVERRIDE T_FINAL;

private:
	float m_linearError;
	float m_angularError;
};

	}
}

#endif	// traktor_net_BodyStateTemplate_H
