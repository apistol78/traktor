#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberEnum.h"
#include "Input/Binding/InputValueSourceData.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.InputValueSourceData", 0, InputValueSourceData, ISerializable)

InputValueSourceData::InputValueSourceData()
:	m_category(CtUnknown)
,	m_controlType(DtInvalid)
,	m_index(-1)
{
}

InputValueSourceData::InputValueSourceData(InputCategory category, InputDefaultControlType controlType)
:	m_category(category)
,	m_controlType(controlType)
,	m_index(-1)
{
}

InputValueSourceData::InputValueSourceData(InputCategory category, int32_t index, InputDefaultControlType controlType)
:	m_category(category)
,	m_controlType(controlType)
,	m_index(index)
{
}

InputCategory InputValueSourceData::getCategory() const
{
	return m_category;
}

InputDefaultControlType InputValueSourceData::getControlType() const
{
	return m_controlType;
}

int32_t InputValueSourceData::getIndex() const
{
	return m_index;
}

const std::wstring& InputValueSourceData::getValueId() const
{
	return m_valueId;
}

bool InputValueSourceData::serialize(ISerializer& s)
{
//	s >> MemberEnum< InputCategory >(L"category", m_category, c_InputCategory_Keys);
//	s >> MemberEnum< InputDefaultControlType >(L"controlType", m_controlType, c_InputDefaultControlType_Keys);
	s >> Member< int32_t >(L"index", m_index);
	s >> Member< std::wstring >(L"valueId", m_valueId);
	return true;
}
	
	}
}
