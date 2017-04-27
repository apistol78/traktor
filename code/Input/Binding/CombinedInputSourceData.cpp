/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Input/Binding/CombinedInputSourceData.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.CombinedInputSourceData", 0, CombinedInputSourceData, IInputSourceData)

CombinedInputSourceData::CombinedInputSourceData()
:	m_mode(CombinedInputSource::CmAny)
{
}

CombinedInputSourceData::CombinedInputSourceData(CombinedInputSource::CombineMode mode)
:	m_mode(mode)
{
}

CombinedInputSourceData::CombinedInputSourceData(const RefArray< IInputSourceData >& sources, CombinedInputSource::CombineMode mode)
:	m_sources(sources)
,	m_mode(mode)
{
}

void CombinedInputSourceData::addSource(IInputSourceData* source)
{
	m_sources.push_back(source);
}

const RefArray< IInputSourceData >& CombinedInputSourceData::getSources() const
{
	return m_sources;
}

Ref< IInputSource > CombinedInputSourceData::createInstance(DeviceControlManager* deviceControlManager) const
{
	RefArray< IInputSource > sources(m_sources.size());
	for (uint32_t i = 0; i < m_sources.size(); ++i)
	{
		Ref< IInputSource > source = m_sources[i]->createInstance(deviceControlManager);
		if (!source)
			return 0;
			
		sources[i] = source;
	}
	return new CombinedInputSource(sources, m_mode);
}

void CombinedInputSourceData::serialize(ISerializer& s)
{
	const MemberEnum< CombinedInputSource::CombineMode >::Key c_CombineMode_Keys[] =
	{
		{ L"CmAny", CombinedInputSource::CmAny },
		{ L"CmExclusive", CombinedInputSource::CmExclusive },
		{ L"CmAll", CombinedInputSource::CmAll },
		{ 0 }
	};

	s >> MemberRefArray< IInputSourceData >(L"sources", m_sources);
	s >> MemberEnum< CombinedInputSource::CombineMode >(L"mode", m_mode, c_CombineMode_Keys);
}
	
	}
}
