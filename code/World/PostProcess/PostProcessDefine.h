#ifndef traktor_world_PostProcessDefine_H
#define traktor_world_PostProcessDefine_H

#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class IRenderSystem;

	}

	namespace resource
	{

class IResourceManager;

	}

	namespace world
	{

class PostProcess;

/*! \brief Post process definition.
 * \ingroup World
 */
class T_DLLCLASS PostProcessDefine : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual bool define(PostProcess* postProcess, resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem, uint32_t screenWidth, uint32_t screenHeight) = 0;
};

	}
}

#endif	// traktor_world_PostProcessDefine_H
