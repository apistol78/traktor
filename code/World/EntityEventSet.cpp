/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "World/EntityEventSet.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.EntityEventSet", EntityEventSet, Object)

world::IEntityEvent* EntityEventSet::getEvent(const std::wstring& name) const
{
	std::map< std::wstring, Ref< IEntityEvent > >::const_iterator i = m_events.find(name);
	return i != m_events.end() ? i->second : 0;
}

	}
}
