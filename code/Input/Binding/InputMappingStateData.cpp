#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Core/Serialization/MemberStl.h"
#include "Input/Binding/InputMappingStateData.h"
#include "Input/Binding/InputStateData.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.input.InputMappingStateData", 0, InputMappingStateData, ISerializable)

void InputMappingStateData::setStateData(const std::wstring& id, InputStateData* data)
{
	if (data)
		m_stateData[id] = data;
	else
		m_stateData.erase(id);
}

const std::map< std::wstring, Ref< InputStateData > >& InputMappingStateData::getStateData() const
{
	return m_stateData;
}

void InputMappingStateData::serialize(ISerializer& s)
{
	s >> MemberStlMap<
		std::wstring,
		Ref< InputStateData >,
		MemberStlPair<
			std::wstring,
			Ref< InputStateData >,
			Member< std::wstring >,
			MemberRef< InputStateData >
		>
	>(L"stateData", m_stateData);
}

	}
}
