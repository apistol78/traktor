#pragma once

#include "Core/Containers/SmallMap.h"
#include "World/IEntityComponentData.h"

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

class EventSetComponent;
class IEntityBuilder;
class IEntityEventData;

/*! \brief
 * \ingroup World
 */
class T_DLLCLASS EventSetComponentData : public IEntityComponentData
{
	T_RTTI_CLASS;

public:
	Ref< EventSetComponent > createComponent(const IEntityBuilder* entityBuilder) const;

	virtual void serialize(ISerializer& s) override final;

private:
	friend class WorldEntityPipeline;

	SmallMap< std::wstring, Ref< IEntityEventData > > m_eventData;
};

	}
}

