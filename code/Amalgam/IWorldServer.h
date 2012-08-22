#ifndef traktor_amalgam_IWorldServer_H
#define traktor_amalgam_IWorldServer_H

#include "Amalgam/IServer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class IEntityBuilder;
class IEntityFactory;
class IEntityRenderer;
class IWorldRenderer;
class WorldEntityRenderers;
class WorldRenderSettings;

	}

	namespace amalgam
	{

/*! \brief World server.
 * \ingroup Amalgam
 *
 * "World.ShadowQuality" - Shadow filter quality.
 */
class T_DLLCLASS IWorldServer : public IServer
{
	T_RTTI_CLASS;

public:
	virtual void addEntityFactory(world::IEntityFactory* entityFactory) = 0;

	virtual void removeEntityFactory(world::IEntityFactory* entityFactory) = 0;

	virtual void addEntityRenderer(world::IEntityRenderer* entityRenderer) = 0;

	virtual void removeEntityRenderer(world::IEntityRenderer* entityRenderer) = 0;

	virtual world::IEntityBuilder* getEntityBuilder() = 0;

	virtual world::WorldEntityRenderers* getEntityRenderers() = 0;

	virtual Ref< world::IWorldRenderer > createWorldRenderer(const world::WorldRenderSettings& worldRenderSettings) = 0;

	virtual int32_t getFrameCount() const = 0;
};

	}
}

#endif	// traktor_amalgam_IWorldServer_H
