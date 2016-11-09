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

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.ControlInputSourceData", 1, ControlInputSourceData, IInputSourceData)

ControlInputSourceData::ControlInputSourceData()
:	m_category(CtUnknown)
,	m_controlType(DtInvalid)
,	m_controlQuery(CqConnectedDevice)
,	m_analogue(false)
,	m_index(-1)
{
}

ControlInputSourceData::ControlInputSourceData(
	InputCategory category,
	InputDefaultControlType controlType,
	ControlQuery controlQuery,
	bool analogue,
	bool normalize
)
:	m_category(category)
,	m_controlType(controlType)
,	m_controlQuery(controlQuery)
,	m_analogue(analogue)
,	m_index(-1)
{
}

ControlInputSourceData::ControlInputSourceData(
	InputCategory category,
	InputDefaultControlType controlType,
	ControlQuery controlQuery,
	bool analogue,
	bool normalize,
	int32_t index
)
:	m_category(category)
,	m_controlType(controlType)
,	m_controlQuery(controlQuery)
,	m_analogue(analogue)
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

void ControlInputSourceData::setControlType(InputDefaultControlType controlType)
{
	m_controlType = controlType;
}

InputDefaultControlType ControlInputSourceData::getControlType() const
{
	return m_controlType;
}

void ControlInputSourceData::setControlQuery(ControlQuery controlQuery)
{
	m_controlQuery = controlQuery;
}

ControlInputSourceData::ControlQuery ControlInputSourceData::getControlQuery() const
{
	return m_controlQuery;
}

void ControlInputSourceData::setAnalogue(bool analogue)
{
	m_analogue = analogue;
}

bool ControlInputSourceData::isAnalogue() const
{
	return m_analogue;
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
	s >> MemberEnum< InputDefaultControlType >(L"controlType", m_controlType, g_InputDefaultControlType_Keys);

	if (s.getVersion() >= 1)
		s >> MemberEnum< ControlQuery >(L"controlQuery", m_controlQuery, c_ControlQuery_Keys);

	s >> Member< bool >(L"analogue", m_analogue);
	s >> Member< int32_t >(L"index", m_index);
}
	
	}
}
