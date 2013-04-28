#ifndef traktor_net_StateTemplate_H
#define traktor_net_StateTemplate_H

#include "Core/Object.h"
#include "Core/Ref.h"

namespace traktor
{
	namespace net
	{

class IValueTemplate;
class State;

class T_DLLCLASS StateTemplate : public Object
{
	T_RTTI_CLASS;

public:
	void declare(const IValueTemplate* value);

	Ref< const State > extrapolate(const State* Sn2, float Tn2, const State* Sn1, float Tn1, const State* S0, float T0, const State* S, float T) const;

	uint32_t pack(const State* S, void* buffer, uint32_t bufferSize) const;

	uint32_t pack(const State* Sn1, const State* S, void* buffer, uint32_t bufferSize) const;

	Ref< const State > unpack(const void* buffer, uint32_t bufferSize) const;

	Ref< const State > unpack(const State* Sn1, const void* buffer, uint32_t bufferSize) const;

	float error(const State* Sl, const State* Sr) const;

private:
	RefArray< const IValueTemplate > m_valueTemplates;
};

	}
}

#endif	// traktor_net_StateTemplate_H
