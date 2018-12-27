/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_world_DecalRenderer_H
#define traktor_world_DecalRenderer_H

#include "Core/RefArray.h"
#include "World/IEntityRenderer.h"

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

class IndexBuffer;
class IRenderSystem;
class VertexBuffer;

	}

	namespace world
	{

class DecalComponent;

/*! \brief Decal entity renderer.
 * \ingroup World
 */
class T_DLLCLASS DecalRenderer : public IEntityRenderer
{
	T_RTTI_CLASS;

public:
	DecalRenderer(render::IRenderSystem* renderSystem);

	virtual const TypeInfoSet getRenderableTypes() const override final;

	virtual void render(
		WorldContext& worldContext,
		WorldRenderView& worldRenderView,
		IWorldRenderPass& worldRenderPass,
		Object* renderable
	) override final;

	virtual void flush(
		WorldContext& worldContext,
		WorldRenderView& worldRenderView,
		IWorldRenderPass& worldRenderPass
	) override final;

private:
	Ref< render::VertexBuffer > m_vertexBuffer;
	Ref< render::IndexBuffer > m_indexBuffer;
	RefArray< DecalComponent > m_decalComponents;
};

	}
}

#endif	// traktor_world_DecalRenderer_H
