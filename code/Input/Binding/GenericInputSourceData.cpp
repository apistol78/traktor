#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberEnum.h"
#include "Input/Binding/EnumKeys.h"
#include "Input/Binding/GenericInputSource.h"
#include "Input/Binding/GenericInputSourceData.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.GenericInputSourceData", 0, GenericInputSourceData, IInputSourceData)

GenericInputSourceData::GenericInputSourceData()
:	m_category(CtUnknown)
,	m_controlType(DtInvalid)
,	m_index(-1)
{
}

GenericInputSourceData::GenericInputSourceData(
	InputCategory category,
	InputDefaultControlType controlType
)
:	m_category(category)
,	m_controlType(controlType)
,	m_index(-1)
{
}

GenericInputSourceData::GenericInputSourceData(
	InputCategory category,
	int32_t index,
	InputDefaultControlType controlType
)
:	m_category(category)
,	m_controlType(controlType)
,	m_index(index)
{
}

void GenericInputSourceData::setCategory(InputCategory category)
{
	m_category = category;
}

InputCategory GenericInputSourceData::getCategory() const
{
	return m_category;
}

void GenericInputSourceData::setControlType(InputDefaultControlType controlType)
{
	m_controlType = controlType;
}

InputDefaultControlType GenericInputSourceData::getControlType() const
{
	return m_controlType;
}

void GenericInputSourceData::setIndex(int32_t index)
{
	m_index = index;
}

int32_t GenericInputSourceData::getIndex() const
{
	return m_index;
}

Ref< IInputSource > GenericInputSourceData::createInstance() const
{
	return new GenericInputSource(this);
}

bool GenericInputSourceData::serialize(ISerializer& s)
{
	s >> MemberEnum< InputCategory >(L"category", m_category, g_InputCategory_Keys);
	s >> MemberEnum< InputDefaultControlType >(L"controlType", m_controlType, g_InputDefaultControlType_Keys);
	s >> Member< int32_t >(L"index", m_index);
	return true;
}
	
	}
}
