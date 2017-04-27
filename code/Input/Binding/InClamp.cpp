/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberStaticArray.h"
#include "Input/Binding/InClamp.h"
#include "Input/Binding/InputValueSet.h"
#include "Input/Binding/ValueDigital.h"

namespace traktor
{
	namespace input
	{
		namespace
		{

struct InClampInstance : public RefCountImpl< IInputNode::Instance >
{
	Ref< IInputNode::Instance > sourceInstance;
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.InClamp", 0, InClamp, IInputNode)

InClamp::InClamp()
{
	m_limit[0] = -1.0f;
	m_limit[1] = 1.0f;
}

InClamp::InClamp(IInputNode* source, float limitMin, float limitMax)
:	m_source(source)
{
	m_limit[0] = limitMin;
	m_limit[1] = limitMax;
}

Ref< IInputNode::Instance > InClamp::createInstance() const
{
	Ref< InClampInstance > instance = new InClampInstance();
	instance->sourceInstance = m_source->createInstance();
	return instance;
}

float InClamp::evaluate(
	Instance* instance,
	const InputValueSet& valueSet,
	float T,
	float dT
) const
{
	InClampInstance* ihi = static_cast< InClampInstance* >(instance);

	float V = m_source->evaluate(ihi->sourceInstance, valueSet, T, dT);

	if (V > m_limit[1])
		return m_limit[1];
	if (V < m_limit[0])
		return m_limit[0];

	return V;
}

void InClamp::serialize(ISerializer& s)
{
	s >> MemberRef< IInputNode >(L"source", m_source);
	s >> MemberStaticArray< float, 2 >(L"limit", m_limit);
}
	
	}
}
