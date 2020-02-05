#include "Core/Serialization/AttributePrivate.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberStaticArray.h"
#include "Core/Serialization/MemberStl.h"
#include "Render/Editor/Image2/IgaPass.h"
#include "Render/Editor/Image2/IgaTarget.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.IgaPass", 0, IgaPass, ISerializable)

IgaPass::IgaPass()
{
	m_position[0] =
	m_position[1] = 0;
}

IgaPass::IgaPass(const std::wstring& name)
:   m_name(name)
{
	m_position[0] =
	m_position[1] = 0;
}

void IgaPass::setName(const std::wstring& name)
{
	m_name = name;
}

const std::wstring& IgaPass::getName() const
{
	return m_name;
}

void IgaPass::setPosition(int32_t x, int32_t y)
{
	m_position[0] = x;
	m_position[1] = y;
}

const int32_t* IgaPass::getPosition() const
{
	return m_position;
}

const std::list< IgaPass::Input >& IgaPass::getInputs() const
{
	return m_inputs;
}

bool IgaPass::attachInput(const std::wstring& name, const IgaTarget* source)
{
	// Replace existing input.
	for (auto& input : m_inputs)
	{
		if (input.name == name)
		{
			input.source = source;
			return true;
		}
	}

	// Add new input.
	Input input;
	input.name = name;
	input.source = source;
	m_inputs.push_back(input);
	return true;
}

const IgaPass::Output& IgaPass::getOutput() const
{
	return m_output;
}

bool IgaPass::attachOutput(const IgaTarget* target)
{
	m_output.target = target;
	return true;
}

void IgaPass::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", m_name);
	s >> MemberStaticArray< int32_t, 2 >(L"position", m_position, AttributePrivate());
	s >> MemberStlList< Input, MemberComposite< Input > >(L"inputs", m_inputs, AttributePrivate());
	s >> MemberComposite< Output >(L"output", m_output, AttributePrivate());
}

void IgaPass::Input::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", name);
	s >> MemberRef< const IgaTarget >(L"source", source);
}

void IgaPass::Output::serialize(ISerializer& s)
{
	s >> MemberRef< const IgaTarget >(L"target", target);
}

	}
}