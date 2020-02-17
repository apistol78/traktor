#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Render/Editor/InputPin.h"
#include "Render/Editor/OutputPin.h"
#include "Render/Editor/Image2/IImgStep.h"
#include "Render/Editor/Image2/ImgPass.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImgPass", 0, ImgPass, Node)

ImgPass::ImgPass()
{
	const Guid c_null;
	m_outputPins.push_back(new OutputPin(this, c_null, L"Output"));
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
	s >> MemberRefArray< IImgStep >(L"steps", m_steps);
	if (s.getDirection() == ISerializer::SdRead)
		refresh();
}

void ImgPass::refresh()
{
	const Guid c_null;

	for (auto& inputPin : m_inputPins)
		delete inputPin;

	m_inputPins.clear();

	std::set< std::wstring > inputs;
	for (auto step : m_steps)
		step->getInputs(inputs);
	for (const auto& input : inputs)
		m_inputPins.push_back(new InputPin(this, c_null, input, false));
}

	}
}
