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

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.ControlInputSourceData", 0, ControlInputSourceData, IInputSourceData)

ControlInputSourceData::ControlInputSourceData()
:	m_category(CtUnknown)
,	m_controlType(DtInvalid)
,	m_analogue(false)
,	m_index(-1)
{
}

ControlInputSourceData::ControlInputSourceData(
	InputCategory category,
	InputDefaultControlType controlType,
	bool analogue,
	bool normalize
)
:	m_category(category)
,	m_controlType(controlType)
,	m_analogue(analogue)
,	m_index(-1)
{
}

ControlInputSourceData::ControlInputSourceData(
	InputCategory category,
	int32_t index,
	InputDefaultControlType controlType,
	bool analogue,
	bool normalize
)
:	m_category(category)
,	m_controlType(controlType)
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
	s >> MemberEnum< InputCategory >(L"category", m_category, g_InputCategory_Keys);
	s >> MemberEnum< InputDefaultControlType >(L"controlType", m_controlType, g_InputDefaultControlType_Keys);
	s >> Member< bool >(L"analogue", m_analogue);
	s >> Member< int32_t >(L"index", m_index);
}
	
	}
}
