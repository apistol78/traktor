/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Const.h"
#include "Core/Serialization/AttributePrivate.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRef.h"
#include "Input/Binding/InLowPass.h"
#include "Input/Binding/InputValueSet.h"

namespace traktor::input
{
	namespace
	{

struct InLowPassInstance : public RefCountImpl< IInputNode::Instance >
{
	Ref< IInputNode::Instance > sourceInstance;
	Ref< IInputNode::Instance > coeffInstance;
	float filteredValue;
};

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.InLowPass", 0, InLowPass, IInputNode)

InLowPass::InLowPass(IInputNode* source, IInputNode* coeff)
:	m_source(source)
,	m_coeff(coeff)
{
}

Ref< IInputNode::Instance > InLowPass::createInstance() const
{
	Ref< InLowPassInstance > instance = new InLowPassInstance();
	instance->sourceInstance = m_source->createInstance();
	instance->coeffInstance = m_coeff->createInstance();
	instance->filteredValue = 0.0f;
	return instance;
}

float InLowPass::evaluate(
	Instance* instance,
	const InputValueSet& valueSet,
	float T,
	float dT
) const
{
	InLowPassInstance* lpi = static_cast< InLowPassInstance* >(instance);
	const float V = m_source->evaluate(lpi->sourceInstance, valueSet, T, dT);
	const float k = m_coeff->evaluate(lpi->coeffInstance, valueSet, T, dT);
	lpi->filteredValue = V * k + lpi->filteredValue * (1.0f - k);
	return lpi->filteredValue;
}

void InLowPass::serialize(ISerializer& s)
{
	s >> MemberRef< IInputNode >(L"source", m_source, AttributePrivate());
	s >> MemberRef< IInputNode >(L"coeff", m_coeff, AttributePrivate());
}

}
