#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Input/Binding/InputMappingData.h"
#include "Input/Binding/InputStateData.h"
#include "Input/Binding/InputValueSourceData.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.InputMappingData", 0, InputMappingData, ISerializable)

void InputMappingData::addSourceData(InputValueSourceData* data)
{
	m_sourceData.push_back(data);
}

const RefArray< InputValueSourceData >& InputMappingData::getSourceData() const
{
	return m_sourceData;
}

void InputMappingData::addStateData(InputStateData* data)
{
	m_stateData.push_back(data);
}

const RefArray< InputStateData >& InputMappingData::getStateData() const
{
	return m_stateData;
}

bool InputMappingData::serialize(ISerializer& s)
{
	s >> MemberRefArray< InputValueSourceData >(L"sourceData", m_sourceData);
	s >> MemberRefArray< InputStateData >(L"stateData", m_stateData);
	return true;
}

	}
}
