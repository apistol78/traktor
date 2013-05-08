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

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.GenericInputSourceData", 1, GenericInputSourceData, IInputSourceData)

GenericInputSourceData::GenericInputSourceData()
:	m_category(CtUnknown)
,	m_controlType(DtInvalid)
,	m_analogue(false)
,	m_inverted(false)
,	m_index(-1)
{
}

GenericInputSourceData::GenericInputSourceData(
	InputCategory category,
	InputDefaultControlType controlType,
	bool analogue,
	bool inverted
)
:	m_category(category)
,	m_controlType(controlType)
,	m_analogue(analogue)
,	m_inverted(inverted)
,	m_index(-1)
{
}

GenericInputSourceData::GenericInputSourceData(
	InputCategory category,
	int32_t index,
	InputDefaultControlType controlType,
	bool analogue,
	bool inverted
)
:	m_category(category)
,	m_controlType(controlType)
,	m_analogue(analogue)
,	m_inverted(inverted)
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

void GenericInputSourceData::setAnalogue(bool analogue)
{
	m_analogue = analogue;
}

bool GenericInputSourceData::isAnalogue() const
{
	return m_analogue;
}

void GenericInputSourceData::setIndex(int32_t index)
{
	m_index = index;
}

void GenericInputSourceData::setInverted(bool inverted)
{
	m_inverted = inverted;
}

bool GenericInputSourceData::isInverted() const
{
	return m_inverted;
}

int32_t GenericInputSourceData::getIndex() const
{
	return m_index;
}

Ref< IInputSource > GenericInputSourceData::createInstance(DeviceControlManager* deviceControlManager) const
{
	return new GenericInputSource(this, deviceControlManager);
}

void GenericInputSourceData::serialize(ISerializer& s)
{
	s >> MemberEnum< InputCategory >(L"category", m_category, g_InputCategory_Keys);
	s >> MemberEnum< InputDefaultControlType >(L"controlType", m_controlType, g_InputDefaultControlType_Keys);
	s >> Member< bool >(L"analogue", m_analogue);

	if (s.getVersion() >= 1)
		s >> Member< bool >(L"inverted", m_inverted);

	s >> Member< int32_t >(L"index", m_index);
}
	
	}
}
