/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include "Core/Meta/Traits.h"
#include "Core/Serialization/AttributeMultiLine.h"
#include "Core/Serialization/AttributePrivate.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/AttributeReadOnly.h"
#include "Core/Serialization/AttributeType.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberEnum.h"
#include "Core/Serialization/MemberStaticArray.h"
#include "Core/Serialization/MemberStl.h"
#include "Render/Editor/InputPin.h"
#include "Render/Editor/Shader/Script.h"
#include "Render/Editor/Shader/ShaderModule.h"

namespace traktor::render
{
	namespace
	{

class MemberInputPin : public MemberComplex
{
public:
	typedef InputPin value_type;

	MemberInputPin(const wchar_t* const name, Node* node, value_type& pin)
	:	MemberComplex(name, true)
	,	m_node(node)
	,	m_pin(pin)
	{
	}

	virtual void serialize(ISerializer& s) const override final
	{
		if (s.getDirection() == ISerializer::Direction::Write)
		{
			Guid id = m_pin.getId();
			std::wstring name = m_pin.getName();

			if (s.getVersion() >= 1)
				s >> Member< Guid >(L"id", id, AttributePrivate());

			s >> Member< std::wstring >(L"name", name);
		}
		else	// Direction::Read
		{
			Guid id;
			std::wstring name = L"";

			if (s.getVersion() >= 1)
				s >> Member< Guid >(L"id", id, AttributePrivate());

			s >> Member< std::wstring >(L"name", name);

			if (s.getVersion() < 2)
			{
				ParameterType type;
				std::wstring samplerId = L"";

				const MemberEnum< ParameterType >::Key c_ParameterType_Keys[] =
				{
					{ L"PtScalar", ParameterType::Scalar },
					{ L"PtVector", ParameterType::Vector },
					{ L"PtMatrix", ParameterType::Matrix },
					{ L"PtTexture2D", ParameterType::Texture2D },
					{ L"PtTexture3D", ParameterType::Texture3D },
					{ L"PtTextureCube", ParameterType::TextureCube },
					{ L"PtStructBuffer", ParameterType::StructBuffer },
					{ L"PtImage2D", ParameterType::Image2D },
					{ L"PtImage3D", ParameterType::Image3D },
					{ L"PtImageCube", ParameterType::ImageCube },
					{ 0 }
				};

				s >> MemberEnum< ParameterType >(L"type", type, c_ParameterType_Keys);
				s >> Member< std::wstring >(L"samplerId", samplerId);
			}

			m_pin = InputPin(
				m_node,
				id,
				name,
				false
			);
		}
	}

	static InputPin createNew(Node* node)
	{
		return InputPin(node, Guid::create(), L"Unnamed", false);
	}

private:
	Node* m_node;
	value_type& m_pin;
};

class MemberTypedOutputPin : public MemberComplex
{
public:
	typedef TypedOutputPin value_type;

	MemberTypedOutputPin(const wchar_t* const name, Node* node, value_type& pin)
	:	MemberComplex(name, true)
	,	m_node(node)
	,	m_pin(pin)
	{
	}

	virtual void serialize(ISerializer& s) const override final
	{
		const MemberEnum< ParameterType >::Key c_ParameterType_Keys[] =
		{
			{ L"Scalar", ParameterType::Scalar },
			{ L"Vector", ParameterType::Vector },
			{ L"Matrix", ParameterType::Matrix },
			{ L"Texture2D", ParameterType::Texture2D },
			{ L"Texture3D", ParameterType::Texture3D },
			{ L"TextureCube", ParameterType::TextureCube },
			{ L"StructBuffer", ParameterType::StructBuffer },
			{ L"Image2D", ParameterType::Image2D },
			{ L"Image3D", ParameterType::Image3D },
			{ L"ImageCube", ParameterType::ImageCube },
			{ 0 }
		};

		if (s.getDirection() == ISerializer::Direction::Write)
		{
			Guid id = m_pin.getId();
			std::wstring name = m_pin.getName();
			ParameterType type = m_pin.getType();

			s >> Member< Guid >(L"id", id, AttributePrivate());
			s >> Member< std::wstring >(L"name", name);
			s >> MemberEnum< ParameterType >(L"type", type, c_ParameterType_Keys);
		}
		else	// Direction::Read
		{
			Guid id;
			std::wstring name;
			ParameterType type;

			s >> Member< Guid >(L"id", id, AttributePrivate());
			s >> Member< std::wstring >(L"name", name);
			s >> MemberEnum< ParameterType >(L"type", type, c_ParameterType_Keys);

			m_pin = TypedOutputPin(
				m_node,
				id,
				name,
				type
			);
		}
	}

	static TypedOutputPin createNew(Node* node)
	{
		return TypedOutputPin(node, Guid::create(), L"Unnamed", ParameterType::Scalar);
	}

private:
	Node* m_node;
	value_type& m_pin;
};

template < typename PinMember >
class MemberPinArray : public MemberArray
{
public:
	typedef typename PinMember::value_type pin_type;
	typedef StaticVector< pin_type, 64 > value_type;

	MemberPinArray(const wchar_t* const name, Node* node, value_type& pins)
	:	MemberArray(name, nullptr)
	,	m_node(node)
	,	m_pins(pins)
	,	m_index(0)
	{
	}

	virtual void reserve(size_t size, size_t capacity) const override final
	{
		m_pins.resize(size);
	}

	virtual size_t size() const override final
	{
		return m_pins.size();
	}

	virtual void read(ISerializer& s) const override final
	{
		if (m_index >= m_pins.size())
			m_pins.push_back(PinMember::createNew(m_node));
		s >> PinMember(L"item", m_node, m_pins[m_index++]);
	}

	virtual void write(ISerializer& s) const override final
	{
		if (s.ensure(m_index < m_pins.size()))
			s >> PinMember(L"item", m_node, m_pins[m_index++]);
	}

	virtual bool insert() const override final
	{
		m_pins.push_back(PinMember::createNew(m_node));
		return true;
	}

private:
	Node* m_node;
	value_type& m_pins;
	mutable size_t m_index;
};

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Script", 8, Script, Node)

Script::Script()
{
	m_localSize[0] = 1;
	m_localSize[1] = 1;
	m_localSize[2] = 1;
}

void Script::setName(const std::wstring& name)
{
	m_name = name;
}

const std::wstring& Script::getName() const
{
	return m_name;
}

void Script::setTechnique(const std::wstring& technique)
{
	m_technique = technique;
}

const std::wstring& Script::getTechnique() const
{
	return m_technique;
}

void Script::setDomain(Domain domain)
{
	m_domain = domain;
}

Script::Domain Script::getDomain() const
{
	return m_domain;
}

const int32_t* Script::getLocalSize() const
{
	return m_localSize;
}

bool Script::useRayTracing() const
{
	return m_useRayTracing;
}

const AlignedVector< Guid >& Script::getIncludes() const
{
	return m_includes;
}

void Script::setScript(const std::wstring& script)
{
	m_script = script;
}

const std::wstring& Script::getScript() const
{
	return m_script;
}

const InputPin* Script::addInputPin(const Guid& id, const std::wstring& name)
{
	m_inputPins.push_back(InputPin(this, id, name, false));
	return &m_inputPins.back();
}

const OutputPin* Script::addOutputPin(const Guid& id, const std::wstring& name, ParameterType type)
{
	m_outputPins.push_back(TypedOutputPin(this, id, name, type));
	return &m_outputPins.back();
}

void Script::removeInputPin(const std::wstring& name)
{
	auto it = std::find_if(m_inputPins.begin(), m_inputPins.end(), [&](const InputPin& pin) {
		return pin.getName() == name;
	});
	if (it != m_inputPins.end())
		m_inputPins.erase(it);
}

void Script::removeAllInputPins()
{
	m_inputPins.clear();
}

void Script::removeAllOutputPins()
{
	m_outputPins.clear();
}

void Script::removeOutputPin(const std::wstring& name)
{
	auto it = std::find_if(m_outputPins.begin(), m_outputPins.end(), [&](const OutputPin& pin) {
		return pin.getName() == name;
	});
	if (it != m_outputPins.end())
		m_outputPins.erase(it);
}

ParameterType Script::getOutputPinType(int index) const
{
	T_ASSERT(index >= 0 && index < int(m_outputPins.size()));
	return m_outputPins[index].getType();
}

std::wstring Script::getInformation() const
{
	return m_technique;
}

int Script::getInputPinCount() const
{
	return int(m_inputPins.size());
}

const InputPin* Script::getInputPin(int index) const
{
	T_ASSERT(index >= 0 && index < int(m_inputPins.size()));
	return &m_inputPins[index];
}

int Script::getOutputPinCount() const
{
	return int(m_outputPins.size());
}

const OutputPin* Script::getOutputPin(int index) const
{
	T_ASSERT(index >= 0 && index < int(m_outputPins.size()));
	return &m_outputPins[index];
}

void Script::serialize(ISerializer& s)
{
	T_FATAL_ASSERT(s.getVersion< Script >() >= 4);

	Node::serialize(s);

	s >> Member< std::wstring >(L"name", m_name);
	s >> Member< std::wstring >(L"technique", m_technique);

	if (s.getVersion< Script >() >= 7)
	{
		const MemberEnum< Domain >::Key c_Domain_Keys[] =
		{
			{ L"Undefined", Domain::Undefined },
			{ L"Vertex", Domain::Vertex },
			{ L"Pixel", Domain::Pixel },
			{ L"Compute", Domain::Compute },
			{ 0 }
		};
		s >> MemberEnum< Domain >(L"domain", m_domain, c_Domain_Keys);
	}
	else
	{
		if (!m_technique.empty())
			m_domain = Domain::Compute;
	}

	if (s.getVersion< Script >() >= 6)
	{
		const wchar_t* c_localSizeElements[] = { L"X", L"Y", L"Z" };
		s >> MemberStaticArray< int32_t, 3 >(L"localSize", m_localSize, AttributeRange(1), c_localSizeElements);
	}

	if (s.getVersion< Script >() >= 8)
		s >> Member< bool >(L"useRayTracing", m_useRayTracing);

	if (s.getVersion< Script >() >= 5)
		s >> MemberAlignedVector< Guid >(L"include", m_includes); // , AttributeType(type_of< ShaderModule >()));

	s >> MemberPinArray< MemberInputPin >(L"inputPins", this, m_inputPins);
	s >> MemberPinArray< MemberTypedOutputPin >(L"outputPins", this, m_outputPins);
	s >> Member< std::wstring >(L"script", m_script, AttributeMultiLine() | AttributePrivate());
}

}
