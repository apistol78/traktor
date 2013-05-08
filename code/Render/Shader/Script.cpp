#include "Core/Serialization/AttributeMultiLine.h"
#include "Core/Serialization/AttributeReadOnly.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComplex.h"
#include "Core/Serialization/MemberStl.h"
#include "Render/Shader/Script.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

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

			s >> Member< std::wstring >(L"name", name);
			s >> MemberEnum< ParameterType >(L"type", type, c_ParameterType_Keys);
		}
		else	// SdRead
		{
			std::wstring name = L"";
			ParameterType type;

			s >> Member< std::wstring >(L"name", name);
			s >> MemberEnum< ParameterType >(L"type", type, c_ParameterType_Keys);

			if (m_pin)
			{
				*m_pin = TypedInputPin(
					s.getCurrentObject< Node >(),
					name,
					false,
					type
				);
			}
			else
			{
				m_pin = new TypedInputPin(
					s.getCurrentObject< Node >(),
					name,
					false,
					type
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

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Script", 0, Script, Node)

const std::wstring& Script::getName() const
{
	return m_name;
}

void Script::setScript(const std::wstring& platform, const std::wstring& script)
{
	if (!script.empty())
		m_scripts[platform] = script;
	else
		m_scripts.erase(platform);
}

std::wstring Script::getScript(const std::wstring& platform) const
{
	const std::map< std::wstring, std::wstring >::const_iterator i = m_scripts.find(platform);
	return i != m_scripts.end() ? i->second : L"";
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

void Script::serialize(ISerializer& s)
{
	Node::serialize(s);

	s >> Member< std::wstring >(L"name", m_name);
	s >> MemberPinArray< MemberTypedInputPin >(L"inputPins", m_inputPins);
	s >> MemberPinArray< MemberTypedOutputPin >(L"outputPins", m_outputPins);
	s >> MemberStlMap< std::wstring, std::wstring >(L"scripts", m_scripts, AttributeMultiLine());
}

	}
}
