#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Input/Binding/IInputNode.h"
#include "Input/Binding/InputStateData.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.InputStateData", 0, InputStateData, ISerializable)

InputStateData::InputStateData()
{
}

InputStateData::InputStateData(IInputNode* source)
:	m_source(source)
{
}

const IInputNode* InputStateData::getSource() const
{
	return m_source;
}

void InputStateData::serialize(ISerializer& s)
{
	s >> MemberRef< IInputNode >(L"source", m_source);
}

	}
}
