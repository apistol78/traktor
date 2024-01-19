/*
 * TRAKTOR
 * Copyright (c) 2023 Anders Pistol.
 *
 * This Custom Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Class/IRuntimeDispatch.h"
#include "Core/Serialization/ISerializer.h"
#include "Resource/IResourceManager.h"
#include "Resource/MemberIdProxy.h"
#include "Sound/BoxedAudioBlock.h"
#include "Sound/IAudioBuffer.h"
#include "Sound/Processor/GraphEvaluator.h"
#include "Sound/Processor/Nodes/Custom.h"

namespace traktor::sound
{
	namespace
	{

const ImmutableNode::InputPinDesc c_Custom_i[] =
{
	{ L"Input", NodePinType::Signal, false },
	{ 0 }
};

const ImmutableNode::OutputPinDesc c_Custom_o[] =
{
	{ L"Output", NodePinType::Signal },
	{ 0 }
};

class CustomCursor : public RefCountImpl< IAudioBufferCursor >
{
public:
	Ref< ITypedObject > m_object;

	virtual void setParameter(handle_t id, float parameter) override final
	{
	}

	virtual void disableRepeat() override final
	{
	}

	virtual void reset() override final
	{
	}
};

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.Custom", 0, Custom, ImmutableNode)

Custom::Custom()
:	ImmutableNode(c_Custom_i, c_Custom_o)
{
}

bool Custom::bind(resource::IResourceManager* resourceManager)
{
	return resourceManager->bind(m_class);
}

Ref< IAudioBufferCursor > Custom::createCursor() const
{
	if (!m_class)
		return nullptr;

	Ref< ITypedObject > object = createRuntimeClassInstance(m_class, nullptr, 0, nullptr);
	if (!object)
		return nullptr;

	Ref< CustomCursor > sourceCursor = new CustomCursor();
	sourceCursor->m_object = object;

	return sourceCursor;
}

bool Custom::getScalar(IAudioBufferCursor* cursor, const GraphEvaluator* evaluator, float& outScalar) const
{
	return false;
}

bool Custom::getBlock(IAudioBufferCursor* cursor, const GraphEvaluator* evaluator, const IAudioMixer* mixer, AudioBlock& outBlock) const
{
	CustomCursor* customCursor = static_cast< CustomCursor* >(cursor);

	if (!evaluator->evaluateBlock(getInputPin(0), mixer, outBlock))
		return false;

	const Any argv[] =
	{
		CastAny< AudioBlock& >::set(outBlock)
	};

	const IRuntimeDispatch* apply = findRuntimeClassMethod(m_class, "apply");
	if (apply)
		apply->invoke(customCursor->m_object, sizeof_array(argv), argv);

	return true;
}

void Custom::serialize(ISerializer& s)
{
	ImmutableNode::serialize(s);

	s >> resource::MemberIdProxy< IRuntimeClass >(L"class", m_class);
}

}
