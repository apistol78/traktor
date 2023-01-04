/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberBitMask.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Core/Serialization/MemberStaticArray.h"
#include "Render/Editor/InputPin.h"
#include "Render/Editor/OutputPin.h"
#include "Render/Editor/Image2/IImgStep.h"
#include "Render/Editor/Image2/ImgPass.h"

namespace traktor::render
{
	namespace
	{

class MemberClear : public MemberComplex
{
public:
	MemberClear(const wchar_t* const name, Clear& ref)
	:	MemberComplex(name, true)
	,	m_ref(ref)
	{
	}

	virtual void serialize(ISerializer& s) const override final
	{
		const MemberBitMask::Bit c_ClearFlag_bits[] =
		{
			{ L"Color", CfColor },
			{ L"Depth", CfDepth },
			{ L"Stencil", CfStencil },
			{ 0 }
		};

		s >> MemberBitMask(L"mask", m_ref.mask, c_ClearFlag_bits);
		s >> MemberStaticArray< Color4f, sizeof_array(m_ref.colors) >(L"colors", m_ref.colors);
		s >> Member< float >(L"depth", m_ref.depth);
		s >> Member< int32_t >(L"stencil", m_ref.stencil);
	}

private:
	Clear& m_ref;
};

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImgPass", 1, ImgPass, Node)

ImgPass::ImgPass()
{
	// Setup default clear parameters.
	m_clear.mask = 0;
	for (int32_t i = 0; i < sizeof_array(m_clear.colors); ++i)
		m_clear.colors[i] = Color4f(0.0f, 0.0f, 0.0f, 1.0f);
	m_clear.depth = 1.0f;
	m_clear.stencil = 0;

	// Create the output pin.
	const Guid outputId(L"{745cf81d-d35f-437a-96ca-dc1fcdb24ce6}");
	m_outputPins.push_back(new OutputPin(this, outputId, L"Output"));
}

ImgPass::~ImgPass()
{
	for (auto& inputPin : m_inputPins)
		delete inputPin;
	for (auto& outputPin : m_outputPins)
		delete outputPin;
}

const std::wstring& ImgPass::getName() const
{
	return m_name;
}

const Clear& ImgPass::getClear() const
{
	return m_clear;
}

const RefArray< IImgStep >& ImgPass::getSteps() const
{
	return m_steps;
}

int ImgPass::getInputPinCount() const
{
	return (int)m_inputPins.size();
}

const InputPin* ImgPass::getInputPin(int index) const
{
	T_ASSERT(index >= 0 && index < (int)m_inputPins.size());
	return m_inputPins[index];
}

int ImgPass::getOutputPinCount() const
{
	return (int)m_outputPins.size();
}

const OutputPin* ImgPass::getOutputPin(int index) const
{
	T_ASSERT(index >= 0 && index < (int)m_outputPins.size());
	return m_outputPins[index];
}

void ImgPass::serialize(ISerializer& s)
{
	Node::serialize(s);

	s >> Member< std::wstring >(L"name", m_name);

	if (s.getVersion< ImgPass >() >= 1)
		s >> MemberClear(L"clear", m_clear);

	s >> MemberRefArray< IImgStep >(L"steps", m_steps);

	if (s.getDirection() == ISerializer::Direction::Read)
		refresh();
}

void ImgPass::refresh()
{
	Guid inputId(L"{fdff30de-f467-419b-bb66-68ae2fe12fa1}");

	for (auto& inputPin : m_inputPins)
		delete inputPin;

	m_inputPins.clear();

	std::set< std::wstring > inputs;
	for (auto step : m_steps)
		step->getInputs(inputs);
	for (const auto& input : inputs)
	{
		m_inputPins.push_back(new InputPin(this, inputId, input, false));
		inputId.permutate();
	}
}

}
