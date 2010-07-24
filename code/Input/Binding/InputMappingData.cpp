#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberStl.h"
#include "Input/Binding/IInputSourceData.h"
#include "Input/Binding/InputMappingData.h"
#include "Input/Binding/InputStateData.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.InputMappingData", 0, InputMappingData, ISerializable)

void InputMappingData::setSourceData(const std::wstring& id, IInputSourceData* data)
{
	m_sourceData[id] = data;
}

const std::map< std::wstring, Ref< IInputSourceData > >& InputMappingData::getSourceData() const
{
	return m_sourceData;
}

void InputMappingData::setStateData(const std::wstring& id, InputStateData* data)
{
	m_stateData[id] = data;
}

const std::map< std::wstring, Ref< InputStateData > >& InputMappingData::getStateData() const
{
	return m_stateData;
}

bool InputMappingData::serialize(ISerializer& s)
{
	s >> MemberStlMap<
		std::wstring,
		Ref< IInputSourceData >,
		MemberStlPair<
			std::wstring,
			Ref< IInputSourceData >,
			Member< std::wstring >,
			MemberRef< IInputSourceData >
		>
	>(L"sourceData", m_sourceData);

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

	return true;
}

	}
}
