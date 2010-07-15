#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Input/Binding/IInputNode.h"
#include "Input/Binding/InputStateData.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.InputStateData", 0, InputStateData, ISerializable)

const std::wstring& InputStateData::getId() const
{
	return m_id;
}

const IInputNode* InputStateData::getSource() const
{
	return m_source;
}

bool InputStateData::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"id", m_id);
	s >> MemberRef< IInputNode >(L"source", m_source);
	return true;
}

	}
}
