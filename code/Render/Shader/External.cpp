#include <algorithm>
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComplex.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Core/Serialization/MemberStl.h"
#include "Render/Shader/Edge.h"
#include "Render/Shader/External.h"
#include "Render/Shader/InputPin.h"
#include "Render/Shader/Nodes.h"
#include "Render/Shader/OutputPin.h"
#include "Render/Shader/ShaderGraph.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

class MemberInputPin : public MemberComplex
{
public:
	typedef InputPin* value_type;

	MemberInputPin(const std::wstring& name, value_type& pin)
	:	MemberComplex(name, true)
	,	m_pin(pin)
	{
	}

	virtual bool serialize(ISerializer& s) const
	{
		if (s.getDirection() == ISerializer::SdWrite)
		{
			std::wstring name = m_pin->getName();
			bool optional = m_pin->isOptional();

			s >> Member< std::wstring >(L"name", name);

			if (s.getVersion() >= 1)
				s >> Member< bool >(L"optional", optional);
		}
		else	// SdRead
		{
			std::wstring name = L"";
			bool optional = false;

			s >> Member< std::wstring >(L"name", name);

			if (s.getVersion() >= 1)
				s >> Member< bool >(L"optional", optional);

			m_pin = new InputPin(
				s.getCurrentObject< Node >(),
				name,
				optional
			);
		}
		return true;
	}

private:
	value_type& m_pin;
};

class MemberOutputPin : public MemberComplex
{
public:
	typedef OutputPin* value_type;

	MemberOutputPin(const std::wstring& name, value_type& pin)
	:	MemberComplex(name, true)
	,	m_pin(pin)
	{
	}

	virtual bool serialize(ISerializer& s) const
	{
		if (s.getDirection() == ISerializer::SdWrite)
		{
			std::wstring name = m_pin->getName();
			s >> Member< std::wstring >(L"name", name);
		}
		else	// SdRead
		{
			std::wstring name;
			s >> Member< std::wstring >(L"name", name);
			m_pin = new OutputPin(s.getCurrentObject< Node >(), name);
		}
		return true;
	}

private:
	value_type& m_pin;
};

struct SortInputPinPredicate
{
	bool operator () (const InputPin* pl, const InputPin* pr) const
	{
		if (!pl->isOptional() && pr->isOptional())
			return true;

		if (pl->isOptional() && !pr->isOptional())
			return false;

		return pl->getName().compare(pr->getName()) < 0;
	}
};

struct SortOutputPinPredicate
{
	bool operator () (const OutputPin* pl, const OutputPin* pr) const
	{
		return pl->getName().compare(pr->getName()) < 0;
	}
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.External", 1, External, Node)

External::External()
{
}

External::External(const Guid& fragmentGuid, ShaderGraph* fragmentGraph)
:	m_fragmentGuid(fragmentGuid)
{
	const RefArray< Node >& fragmentNodes = fragmentGraph->getNodes();
	for (RefArray< Node >::const_iterator i = fragmentNodes.begin(); i != fragmentNodes.end(); ++i)
	{
		const Node* fragmentNode = *i;
		if (const InputPort* inputPort = dynamic_type_cast< const InputPort* >(fragmentNode))
		{
			std::wstring name = inputPort->getName();

			if (inputPort->isConnectable())
			{
				m_inputPins.push_back(new InputPin(
					this,
					name,
					inputPort->isOptional()
				));
			}

			if (!inputPort->isConnectable() || inputPort->isOptional())
				m_values[name] = inputPort->getDefaultValue();
		}
		else if (const OutputPort* outputPort = dynamic_type_cast< const OutputPort* >(fragmentNode))
		{
			std::wstring name = outputPort->getName();
			m_outputPins.push_back(new OutputPin(
				this,
				name
			));
		}
	}

	// Sort pins lexicographically.
	std::sort(m_inputPins.begin(), m_inputPins.end(), SortInputPinPredicate());
	std::sort(m_outputPins.begin(), m_outputPins.end(), SortOutputPinPredicate());
}

void External::setFragmentGuid(const Guid& fragmentGuid)
{
	m_fragmentGuid = fragmentGuid;
}

const Guid& External::getFragmentGuid() const
{
	return m_fragmentGuid;
}

void External::setValue(const std::wstring& name, float value)
{
	m_values[name] = value;
}

float External::getValue(const std::wstring& name, float defaultValue) const
{
	std::map< std::wstring, float >::const_iterator i = m_values.find(name);
	return i != m_values.end() ? i->second : defaultValue;
}

void External::removeValue(const std::wstring& name)
{
	std::map< std::wstring, float >::iterator i = m_values.find(name);
	if (i != m_values.end())
		m_values.erase(i);
}

const InputPin* External::createInputPin(const std::wstring& name, bool optional)
{
	InputPin* inputPin = new InputPin(this, name, optional);

	m_inputPins.push_back(inputPin);
	std::sort(m_inputPins.begin(), m_inputPins.end(), SortInputPinPredicate());

	return inputPin;
}

const OutputPin* External::createOutputPin(const std::wstring& name)
{
	OutputPin* outputPin = new OutputPin(this, name);

	m_outputPins.push_back(outputPin);
	std::sort(m_outputPins.begin(), m_outputPins.end(), SortOutputPinPredicate());

	return outputPin;
}

void External::removeInputPin(const InputPin* inputPin)
{
	std::vector< InputPin* >::iterator i = std::find(m_inputPins.begin(), m_inputPins.end(), inputPin);
	T_ASSERT (i != m_inputPins.end());
	delete *i; m_inputPins.erase(i);
}

void External::removeOutputPin(const OutputPin* outputPin)
{
	std::vector< OutputPin* >::iterator i = std::find(m_outputPins.begin(), m_outputPins.end(), outputPin);
	T_ASSERT (i != m_outputPins.end());
	delete *i; m_outputPins.erase(i);
}

std::wstring External::getInformation() const
{
	return m_fragmentGuid.format();
}

int External::getInputPinCount() const
{
	return int(m_inputPins.size());
}

const InputPin* External::getInputPin(int index) const
{
	T_ASSERT (index >= 0 && index < int(m_inputPins.size()));
	return m_inputPins[index];
}

int External::getOutputPinCount() const
{
	return int(m_outputPins.size());
}

const OutputPin* External::getOutputPin(int index) const
{
	T_ASSERT (index >= 0 && index < int(m_outputPins.size()));
	return m_outputPins[index];
}

bool External::serialize(ISerializer& s)
{
	if (!Node::serialize(s))
		return false;

	s >> Member< Guid >(L"fragmentGuid", m_fragmentGuid, &type_of< ShaderGraph >());
	s >> MemberStlVector< InputPin*, MemberInputPin >(L"inputPins", m_inputPins);
	s >> MemberStlVector< OutputPin*, MemberOutputPin >(L"outputPins", m_outputPins);

	if (s.getVersion() >= 1)
		s >> MemberStlMap< std::wstring, float >(L"values", m_values);
	
	if (s.getDirection() == ISerializer::SdRead)
	{
		// Update edges; we have created new pins but the shader graph might still
		// have edges which reference our old pins.
		const ShaderGraph* shaderGraph = s.getOuterObject< ShaderGraph >();
		if (shaderGraph)
		{
			const RefArray< Edge >& edges = shaderGraph->getEdges();
			for (RefArray< Edge >::const_iterator i = edges.begin(); i != edges.end(); ++i)
			{
				const OutputPin* sourcePin = (*i)->getSource();
				const InputPin* destinationPin = (*i)->getDestination();

				if (sourcePin->getNode() == this)
					sourcePin = findOutputPin(sourcePin->getName());

				if (destinationPin->getNode() == this)
					destinationPin = findInputPin(destinationPin->getName());

				(*i)->setSource(sourcePin);
				(*i)->setDestination(destinationPin);
			}
		}
	}

	return true;
}

	}
}
