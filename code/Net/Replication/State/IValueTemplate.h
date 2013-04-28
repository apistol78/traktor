#ifndef traktor_net_IValueTemplate_H
#define traktor_net_IValueTemplate_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_NET_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class BitReader;
class BitWriter;

	namespace net
	{

class IValue;

class T_DLLCLASS IValueTemplate : public Object
{
	T_RTTI_CLASS;

public:
	virtual void pack(BitWriter& writer, const IValue* V) const = 0;

	virtual Ref< const IValue > unpack(BitReader& reader) const = 0;

	virtual float error(const IValue* Vl, const IValue* Vr) const = 0;

	virtual Ref< const IValue > extrapolate(const IValue* Vn2, float Tn2, const IValue* Vn1, float Tn1, const IValue* V0, float T0, const IValue* V, float T) const = 0;
};

	}
}

#endif	// traktor_net_IValueTemplate_H
