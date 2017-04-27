/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_physics_EntityRenderer_H
#define traktor_physics_EntityRenderer_H

#include "World/IEntityRenderer.h"

#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

/*! \brief Physics entity renderer.
 * \ingroup Physics
 */
class T_DLLCLASS EntityRenderer : public world::IEntityRenderer
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfoSet getRenderableTypes() const T_OVERRIDE T_FINAL;

	virtual void render(
		world::WorldContext& worldContext,
		world::WorldRenderView& worldRenderView,
		world::IWorldRenderPass& worldRenderPass,
		Object* renderable
	) T_OVERRIDE T_FINAL;

	virtual void flush(
		world::WorldContext& worldContext,
		world::WorldRenderView& worldRenderView,
		world::IWorldRenderPass& worldRenderPass
	) T_OVERRIDE T_FINAL
	{ /* Not implemented */ }
};

	}
}

#endif	// traktor_physics_EntityRenderer_H
