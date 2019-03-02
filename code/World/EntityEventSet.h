#pragma once

#include <map>
#include <string>
#include "Core/Object.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class IEntityEvent;

class T_DLLCLASS EntityEventSet : public Object
{
	T_RTTI_CLASS;

public:
	world::IEntityEvent* getEvent(const std::wstring& name) const;

private:
	friend class EntityEventSetData;

	std::map< std::wstring, Ref< IEntityEvent > > m_events;
};

	}
}

