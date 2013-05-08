#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberStl.h"
#include "Input/Binding/IInputSourceData.h"
#include "Input/Binding/InputMappingSourceData.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.input.InputMappingSourceData", 0, InputMappingSourceData, ISerializable)

void InputMappingSourceData::setSourceData(const std::wstring& id, IInputSourceData* data)
{
	m_sourceData[id] = data;
}

const std::map< std::wstring, Ref< IInputSourceData > >& InputMappingSourceData::getSourceData() const
{
	return m_sourceData;
}

void InputMappingSourceData::serialize(ISerializer& s)
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
}

	}
}
