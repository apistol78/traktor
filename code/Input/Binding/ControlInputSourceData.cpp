#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberEnum.h"
#include "Input/Binding/EnumKeys.h"
#include "Input/Binding/ControlInputSource.h"
#include "Input/Binding/ControlInputSourceData.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.ControlInputSourceData", 2, ControlInputSourceData, IInputSourceData)

ControlInputSourceData::ControlInputSourceData()
:	m_category(CtUnknown)
,	m_controlQuery(CqConnectedDevice)
,	m_index(-1)
{
}

ControlInputSourceData::ControlInputSourceData(
	InputCategory category,
	ControlQuery controlQuery
)
:	m_category(category)
,	m_controlQuery(controlQuery)
,	m_index(-1)
{
}

ControlInputSourceData::ControlInputSourceData(
	InputCategory category,
	ControlQuery controlQuery,
	int32_t index
)
:	m_category(category)
,	m_controlQuery(controlQuery)
,	m_index(index)
{
}

void ControlInputSourceData::setCategory(InputCategory category)
{
	m_category = category;
}

InputCategory ControlInputSourceData::getCategory() const
{
	return m_category;
}

void ControlInputSourceData::setControlQuery(ControlQuery controlQuery)
{
	m_controlQuery = controlQuery;
}

ControlInputSourceData::ControlQuery ControlInputSourceData::getControlQuery() const
{
	return m_controlQuery;
}

void ControlInputSourceData::setIndex(int32_t index)
{
	m_index = index;
}

int32_t ControlInputSourceData::getIndex() const
{
	return m_index;
}

Ref< IInputSource > ControlInputSourceData::createInstance(DeviceControlManager* deviceControlManager) const
{
	return new ControlInputSource(this, deviceControlManager);
}

void ControlInputSourceData::serialize(ISerializer& s)
{
	const MemberEnum< ControlQuery >::Key c_ControlQuery_Keys[] =
	{
		{ L"CqMatchingDevice", CqMatchingDevice },
		{ L"CqConnectedDevice", CqConnectedDevice },
		{ 0 }
	};

	s >> MemberEnum< InputCategory >(L"category", m_category, g_InputCategory_Keys);

	if (s.getVersion() < 2)
	{
		InputDefaultControlType controlType;
		s >> MemberEnum< InputDefaultControlType >(L"controlType", controlType, g_InputDefaultControlType_Keys);
	}

	if (s.getVersion() >= 1)
		s >> MemberEnum< ControlQuery >(L"controlQuery", m_controlQuery, c_ControlQuery_Keys);

	if (s.getVersion() < 2)
	{
		bool analogue;
		s >> Member< bool >(L"analogue", analogue);
	}

	s >> Member< int32_t >(L"index", m_index);
}
	
	}
}
