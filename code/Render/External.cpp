#include <algorithm>
#include "Render/External.h"
#include "Render/InputPin.h"
#include "Render/OutputPin.h"
#include "Render/ShaderGraph.h"
#include "Render/Nodes.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/MemberComplex.h"
#include "Core/Serialization/MemberRef.h"
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

	virtual bool serialize(Serializer& s) const
	{
		if (s.getDirection() == Serializer::SdWrite)
		{
			std::wstring name = m_pin->getName();
			s >> Member< std::wstring >(L"name", name);
		}
		else	// SdRead
		{
			std::wstring name;
			s >> Member< std::wstring >(L"name", name);
			m_pin = gc_new< InputPin >(s.getCurrentObject< Node >(), name, false);
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

	virtual bool serialize(Serializer& s) const
	{
		if (s.getDirection() == Serializer::SdWrite)
		{
			std::wstring name = m_pin->getName();
			s >> Member< std::wstring >(L"name", name);
		}
		else	// SdRead
		{
			std::wstring name;
			s >> Member< std::wstring >(L"name", name);
			m_pin = gc_new< OutputPin >(s.getCurrentObject< Node >(), name);
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

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.render.External", External, Node)

External::External()
{
}

External::External(const Guid& fragmentGuid, ShaderGraph* fragmentGraph)
:	m_fragmentGuid(fragmentGuid)
{
	// Create pins for every port found in fragment.
	const RefArray< Node >& fragmentNodes = fragmentGraph->getNodes();
	for (RefArray< Node >::const_iterator i = fragmentNodes.begin(); i != fragmentNodes.end(); ++i)
	{
		const Node* fragmentNode = *i;
		if (is_a< InputPort >(fragmentNode))
		{
			std::wstring name = static_cast< const InputPort* >(fragmentNode)->getName();
			m_inputPins.push_back(gc_new< InputPin >(this, name, false));
		}
		else if (is_a< OutputPort >(fragmentNode))
		{
			std::wstring name = static_cast< const OutputPort* >(fragmentNode)->getName();
			m_outputPins.push_back(gc_new< OutputPin >(this, name));
		}
	}

	// Sort pins lexiographically.
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

bool External::serialize(Serializer& s)
{
	if (!Node::serialize(s))
		return false;

	s >> Member< Guid >(L"fragmentGuid", m_fragmentGuid, &type_of< ShaderGraph >());
	s >> MemberRefArray< InputPin, MemberInputPinRef >(L"inputPins", m_inputPins);
	s >> MemberRefArray< OutputPin, MemberOutputPinRef >(L"outputPins", m_outputPins);

	return true;
}

	}
}
