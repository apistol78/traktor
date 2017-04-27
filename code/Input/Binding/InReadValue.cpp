/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Input/Binding/InReadValue.h"
#include "Input/Binding/InputValueSet.h"

namespace traktor
{
	namespace input
	{
		namespace
		{

struct InReadValueInstance : public RefCountImpl< IInputNode::Instance >
{
	handle_t valueId;
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.InReadValue", 0, InReadValue, IInputNode)

InReadValue::InReadValue()
{
}

InReadValue::InReadValue(const std::wstring& valueId)
:	m_valueId(valueId)
{
}

Ref< IInputNode::Instance > InReadValue::createInstance() const
{
	Ref< InReadValueInstance > instance = new InReadValueInstance();
	instance->valueId = getParameterHandle(m_valueId);
	return instance;
}

float InReadValue::evaluate(
	Instance* instance,
	const InputValueSet& valueSet,
	float T,
	float dT
) const
{
	const InReadValueInstance* rvi = static_cast< const InReadValueInstance* >(instance);
	return valueSet.get(rvi->valueId);
}

void InReadValue::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"valueId", m_valueId);
}
	
	}
}
