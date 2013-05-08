#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Core/Serialization/MemberStl.h"
#include "Input/Binding/IInputFilter.h"
#include "Input/Binding/InputMappingStateData.h"
#include "Input/Binding/InputStateData.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.input.InputMappingStateData", 1, InputMappingStateData, ISerializable)

void InputMappingStateData::addFilter(IInputFilter* valueFilter)
{
	m_filters.push_back(valueFilter);
}
	
const RefArray< IInputFilter >& InputMappingStateData::getFilters() const
{
	return m_filters;
}

void InputMappingStateData::setStateData(const std::wstring& id, InputStateData* data)
{
	m_stateData[id] = data;
}

const std::map< std::wstring, Ref< InputStateData > >& InputMappingStateData::getStateData() const
{
	return m_stateData;
}

void InputMappingStateData::serialize(ISerializer& s)
{
	if (s.getVersion() >= 1)
		s >> MemberRefArray< IInputFilter >(L"filters", m_filters);

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
