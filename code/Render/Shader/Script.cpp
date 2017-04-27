/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <algorithm>
#include "Core/Serialization/AttributeMultiLine.h"
#include "Core/Serialization/AttributeReadOnly.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Render/Shader/Script.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

template < typename PinType >
class NamedPinPredicate
{
public:
	NamedPinPredicate(const std::wstring& name)
	:	m_name(name)
	{
	}

	bool operator () (const PinType* pin) const
	{
		return pin->getName() == m_name;
	}

private:
	std::wstring m_name;
};

class MemberTypedInputPin : public MemberComplex
{
public:
	typedef TypedInputPin* value_type;

	MemberTypedInputPin(const wchar_t* const name, value_type& pin)
	:	MemberComplex(name, true)
	,	m_pin(pin)
	{
	}

	virtual void serialize(ISerializer& s) const
	{
		const MemberEnum< ParameterType >::Key c_ParameterType_Keys[] =
		{
			{ L"PtScalar", PtScalar },
			{ L"PtVector", PtVector },
			{ L"PtMatrix", PtMatrix },
			{ L"PtTexture2D", PtTexture2D },
			{ L"PtTexture3D", PtTexture3D },
			{ L"PtTextureCube", PtTextureCube },
			{ 0, 0 }
		};

		if (s.getDirection() == ISerializer::SdWrite)
		{
			std::wstring name = m_pin->getName();
			ParameterType type = m_pin->getType();
			std::wstring samplerId = m_pin->getSamplerId();

			s >> Member< std::wstring >(L"name", name);
			s >> MemberEnum< ParameterType >(L"type", type, c_ParameterType_Keys);
			s >> Member< std::wstring >(L"samplerId", samplerId);
		}
		else	// SdRead
		{
			std::wstring name = L"";
			ParameterType type;
			std::wstring samplerId = L"";

			s >> Member< std::wstring >(L"name", name);
			s >> MemberEnum< ParameterType >(L"type", type, c_ParameterType_Keys);
			s >> Member< std::wstring >(L"samplerId", samplerId);

			if (m_pin)
			{
				*m_pin = TypedInputPin(
					s.getCurrentObject< Node >(),
					name,
					false,
					type,
					samplerId
				);
			}
			else
			{
				m_pin = new TypedInputPin(
					s.getCurrentObject< Node >(),
					name,
					false,
					type,
					samplerId
				);
			}
		}
	}

private:
	value_type& m_pin;
};

class MemberTypedOutputPin : public MemberComplex
{
public:
	typedef TypedOutputPin* value_type;

	MemberTypedOutputPin(const wchar_t* const name, value_type& pin)
	:	MemberComplex(name, true)
	,	m_pin(pin)
	{
	}

	virtual void serialize(ISerializer& s) const
	{
		const MemberEnum< ParameterType >::Key c_ParameterType_Keys[] =
		{
			{ L"PtScalar", PtScalar },
			{ L"PtVector", PtVector },
			{ L"PtMatrix", PtMatrix },
			{ L"PtTexture2D", PtTexture2D },
			{ L"PtTexture3D", PtTexture3D },
			{ L"PtTextureCube", PtTextureCube },
			{ 0, 0 }
		};

		if (s.getDirection() == ISerializer::SdWrite)
		{
			std::wstring name = m_pin->getName();
			ParameterType type = m_pin->getType();

			s >> Member< std::wstring >(L"name", name);
			s >> MemberEnum< ParameterType >(L"type", type, c_ParameterType_Keys);
		}
		else	// SdRead
		{
			std::wstring name;
			ParameterType type;

			s >> Member< std::wstring >(L"name", name);
			s >> MemberEnum< ParameterType >(L"type", type, c_ParameterType_Keys);

			if (m_pin)
			{
				*m_pin = TypedOutputPin(
					s.getCurrentObject< Node >(),
					name,
					type
				);
			}
			else
			{
				m_pin = new TypedOutputPin(
					s.getCurrentObject< Node >(),
					name,
					type
				);
			}
		}
	}

private:
	value_type& m_pin;
};

template < typename PinMember >
class MemberPinArray : public MemberArray
{
public:
	typedef typename PinMember::value_type pin_type;
	typedef std::vector< pin_type > value_type;

	MemberPinArray(const wchar_t* const name, value_type& pins)
	:	MemberArray(name, &m_attribute)
	,	m_pins(pins)
	,	m_index(0)
	{
	}

	virtual void reserve(size_t size, size_t capacity) const
	{
		m_pins.reserve(capacity);
	}

	virtual size_t size() const
	{
		return m_pins.size();
	}

	virtual void read(ISerializer& s) const
	{
		if (m_index >= m_pins.size())
			m_pins.push_back(0);
		s >> PinMember(L"item", m_pins[m_index++]);
	}

	virtual void write(ISerializer& s) const
	{
		if (s.ensure(m_index < m_pins.size()))
			s >> PinMember(L"item", m_pins[m_index++]);
	}

	virtual bool insert() const
	{
		return false;
	}

private:
	AttributeReadOnly m_attribute;
	value_type& m_pins;
	mutable size_t m_index;
};

class MemberSamplerState : public MemberComplex
{
public:
	MemberSamplerState(const wchar_t* const name, SamplerState& ref)
	:	MemberComplex(name, true)
	,	m_ref(ref)
	{
	}

	virtual void serialize(ISerializer& s) const
	{
		const MemberEnum< Filter >::Key kFilter[] =
		{
			{ L"FtPoint", FtPoint },
			{ L"FtLinear", FtLinear },
			{ 0, 0 }
		};

		const MemberEnum< Address >::Key kAddress[] =
		{
			{ L"AdWrap", AdWrap },
			{ L"AdMirror", AdMirror },
			{ L"AdClamp", AdClamp },
			{ L"AdBorder", AdBorder },
			{ 0, 0 }
		};

		const MemberEnum< CompareFunction >::Key kCompareFunctions[] =
		{
			{ L"CfAlways", CfAlways },
			{ L"CfNever", CfNever },
			{ L"CfLess", CfLess },
			{ L"CfLessEqual", CfLessEqual },
			{ L"CfGreater", CfGreater },
			{ L"CfGreaterEqual", CfGreaterEqual },
			{ L"CfEqual", CfEqual },
			{ L"CfNotEqual", CfNotEqual },
			{ L"CfNone", CfNone },
			{ 0, 0 }
		};

		s >> MemberEnum< Filter >(L"minFilter", m_ref.minFilter, kFilter);
		s >> MemberEnum< Filter >(L"mipFilter", m_ref.mipFilter, kFilter);
		s >> MemberEnum< Filter >(L"magFilter", m_ref.magFilter, kFilter);
		s >> MemberEnum< Address >(L"addressU", m_ref.addressU, kAddress);
		s >> MemberEnum< Address >(L"addressV", m_ref.addressV, kAddress);
		s >> MemberEnum< Address >(L"addressW", m_ref.addressW, kAddress);
		s >> MemberEnum< CompareFunction >(L"compare", m_ref.compare, kCompareFunctions);
		s >> Member< float >(L"mipBias", m_ref.mipBias);
		s >> Member< bool >(L"ignoreMips", m_ref.ignoreMips);
		s >> Member< bool >(L"useAnisotropic", m_ref.useAnisotropic);
	}

private:
	SamplerState& m_ref;
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Script", 0, Script, Node)

Script::~Script()
{
	for (std::vector< TypedInputPin* >::iterator i = m_inputPins.begin(); i != m_inputPins.end(); ++i)
		delete *i;
	for (std::vector< TypedOutputPin* >::iterator i = m_outputPins.begin(); i != m_outputPins.end(); ++i)
		delete *i;
}

const std::wstring& Script::getName() const
{
	return m_name;
}

void Script::setScript(const std::wstring& script)
{
	m_script = script;
}

const std::wstring& Script::getScript() const
{
	return m_script;
}

const InputPin* Script::addInputPin(const std::wstring& name, ParameterType type)
{
	TypedInputPin* inputPin = new TypedInputPin(this, name, false, type);
	m_inputPins.push_back(inputPin);
	return inputPin;
}

const OutputPin* Script::addOutputPin(const std::wstring& name, ParameterType type)
{
	TypedOutputPin* outputPin = new TypedOutputPin(this, name, type);
	m_outputPins.push_back(outputPin);
	return outputPin;
}

void Script::removeInputPin(const std::wstring& name)
{
	std::vector< TypedInputPin* >::iterator i = std::find_if(m_inputPins.begin(), m_inputPins.end(), NamedPinPredicate< TypedInputPin >(name));
	if (i != m_inputPins.end())
	{
		delete *i;
		m_inputPins.erase(i);
	}
}

void Script::removeAllInputPins()
{
	for (std::vector< TypedInputPin* >::iterator i = m_inputPins.begin(); i != m_inputPins.end(); ++i)
		delete *i;
	m_inputPins.clear();
}

void Script::removeAllOutputPins()
{
	for (std::vector< TypedOutputPin* >::iterator i = m_outputPins.begin(); i != m_outputPins.end(); ++i)
		delete *i;
	m_outputPins.clear();
}

void Script::removeOutputPin(const std::wstring& name)
{
	std::vector< TypedOutputPin* >::iterator i = std::find_if(m_outputPins.begin(), m_outputPins.end(), NamedPinPredicate< TypedOutputPin >(name));
	if (i != m_outputPins.end())
	{
		delete *i;
		m_outputPins.erase(i);
	}
}

ParameterType Script::getInputPinType(int index) const
{
	T_ASSERT (index >= 0 && index < int(m_inputPins.size()));
	return m_inputPins[index]->getType();
}

std::wstring Script::getInputPinSamplerId(int index) const
{
	T_ASSERT (index >= 0 && index < int(m_inputPins.size()));
	return m_inputPins[index]->getSamplerId();
}

ParameterType Script::getOutputPinType(int index) const
{
	T_ASSERT (index >= 0 && index < int(m_outputPins.size()));
	return m_outputPins[index]->getType();
}

int Script::getInputPinCount() const
{
	return int(m_inputPins.size());
}

const InputPin* Script::getInputPin(int index) const
{
	T_ASSERT (index >= 0 && index < int(m_inputPins.size()));
	return m_inputPins[index];
}

int Script::getOutputPinCount() const
{
	return int(m_outputPins.size());
}

const OutputPin* Script::getOutputPin(int index) const
{
	T_ASSERT (index >= 0 && index < int(m_outputPins.size()));
	return m_outputPins[index];
}

const std::map< std::wstring, SamplerState >& Script::getSamplers() const
{
	return m_samplers;
}

void Script::serialize(ISerializer& s)
{
	Node::serialize(s);

	s >> Member< std::wstring >(L"name", m_name);
	s >> MemberPinArray< MemberTypedInputPin >(L"inputPins", m_inputPins);
	s >> MemberPinArray< MemberTypedOutputPin >(L"outputPins", m_outputPins);
	s >> MemberStlMap<
		std::wstring,
		SamplerState,
		MemberStlPair<
			std::wstring,
			SamplerState,
			Member< std::wstring >,
			MemberSamplerState
		>
	>(L"samplers", m_samplers);
	s >> Member< std::wstring >(L"script", m_script, AttributeMultiLine());
}

	}
}
