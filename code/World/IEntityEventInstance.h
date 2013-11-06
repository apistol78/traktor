#ifndef traktor_world_IEntityEventInstance_H
#define traktor_world_IEntityEventInstance_H

#include "Core/Object.h"
#include "World/WorldTypes.h"

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

class IWorldRenderer;

/*! \brief
 * \ingroup World
 */
class T_DLLCLASS IEntityEventInstance : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool update(const UpdateParams& update) = 0;

	virtual void build(IWorldRenderer* worldRenderer) = 0;

	virtual void cancel() = 0;
};

	}
}

#endif	// traktor_world_IEntityEventInstance_H
