/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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

	bool match(const State* S) const;

	bool critical(const State* Sn1, const State* S0) const;

	Ref< const State > extrapolate(const State* Sn2, float Tn2, const State* Sn1, float Tn1, const State* S0, float T0, float T) const;

	uint32_t pack(const State* S, void* buffer, uint32_t bufferSize) const;

	Ref< const State > unpack(const void* buffer, uint32_t bufferSize) const;

private:
	RefArray< const IValueTemplate > m_valueTemplates;
};

	}
}

#endif	// traktor_net_StateTemplate_H
