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

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.GenericInputSourceData", 3, GenericInputSourceData, IInputSourceData)

GenericInputSourceData::GenericInputSourceData()
:	m_category(CtUnknown)
,	m_controlType(DtInvalid)
,	m_analogue(false)
,	m_normalize(false)
,	m_index(-1)
{
}

GenericInputSourceData::GenericInputSourceData(
	InputCategory category,
	InputDefaultControlType controlType,
	bool analogue,
	bool normalize
)
:	m_category(category)
,	m_controlType(controlType)
,	m_analogue(analogue)
,	m_normalize(normalize)
,	m_index(-1)
{
}

GenericInputSourceData::GenericInputSourceData(
	InputCategory category,
	int32_t index,
	InputDefaultControlType controlType,
	bool analogue,
	bool normalize
)
:	m_category(category)
,	m_controlType(controlType)
,	m_analogue(analogue)
,	m_normalize(normalize)
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

void GenericInputSourceData::setNormalize(bool normalize)
{
	m_normalize = normalize;
}

bool GenericInputSourceData::normalize() const
{
	return m_normalize;
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

	if (s.getVersion() >= 1 && s.getVersion() <= 2)
	{
		bool inverted = false;
		s >> Member< bool >(L"inverted", inverted);
	}

	if (s.getVersion() >= 2)
		s >> Member< bool >(L"normalize", m_normalize);

	s >> Member< int32_t >(L"index", m_index);
}
	
	}
}
