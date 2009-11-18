#include <algorithm>
#include "Render/External.h"
#include "Render/InputPin.h"
#include "Render/OutputPin.h"
#include "Render/ShaderGraph.h"
#include "Render/Nodes.h"
#include "Render/Edge.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComplex.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Core/Serialization/MemberStl.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

class MemberInputPinRef : public MemberComplex
{
public:
	typedef Ref< InputPin > value_type;

	MemberInputPinRef(const std::wstring& name, value_type& pin)
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

class MemberOutputPinRef : public MemberComplex
{
public:
	typedef Ref< OutputPin > value_type;

	MemberOutputPinRef(const std::wstring& name, value_type& pin)
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

template < typename PinType >
struct SortPinPredicate
{
	bool operator () (const PinType* pl, const PinType* pr) const
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
	std::sort(m_inputPins.begin(), m_inputPins.end(), SortPinPredicate< InputPin >());
	std::sort(m_outputPins.begin(), m_outputPins.end(), SortPinPredicate< OutputPin >());
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

std::wstring External::getInformation() const
{
	return m_fragmentGuid.format();
}

int External::getInputPinCount() const
{
	return int(m_inputPins.size());
}

Ref< const InputPin > External::getInputPin(int index) const
{
	T_ASSERT (index >= 0 && index < int(m_inputPins.size()));
	return m_inputPins[index];
}

int External::getOutputPinCount() const
{
	return int(m_outputPins.size());
}

Ref< const OutputPin > External::getOutputPin(int index) const
{
	T_ASSERT (index >= 0 && index < int(m_outputPins.size()));
	return m_outputPins[index];
}

bool External::serialize(ISerializer& s)
{
	if (!Node::serialize(s))
		return false;

	s >> Member< Guid >(L"fragmentGuid", m_fragmentGuid, &type_of< ShaderGraph >());
	s >> MemberRefArray< InputPin, MemberInputPinRef >(L"inputPins", m_inputPins);
	s >> MemberRefArray< OutputPin, MemberOutputPinRef >(L"outputPins", m_outputPins);

	if (s.getVersion() >= 1)
		s >> MemberStlMap< std::wstring, float >(L"values", m_values);
	
	if (s.getDirection() == ISerializer::SdRead)
	{
		// Update edges; we have created new pins but the shader graph might still
		// have edges which reference our old pins.
		Ref< ShaderGraph > shaderGraph = s.getOuterObject< ShaderGraph >();
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
