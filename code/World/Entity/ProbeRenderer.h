/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Resource/Proxy.h"
#include "World/IEntityRenderer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class Buffer;
class IRenderSystem;
class IVertexLayout;
class ScreenRenderer;
class Shader;

}

namespace traktor::resource
{
	
class IResourceManager;

}

namespace traktor::world
{

class IWorldRenderer;
class ProbeComponent;

/*! Probe entity renderer.
 * \ingroup World
 */
class T_DLLCLASS ProbeRenderer : public IEntityRenderer
{
	T_RTTI_CLASS;

public:
	explicit ProbeRenderer(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		const TypeInfo& worldRendererType
	);

	virtual ~ProbeRenderer();

	virtual bool initialize(const ObjectStore& objectStore) override final;

	virtual const TypeInfoSet getRenderableTypes() const override final;

	virtual void setup(
		const WorldSetupContext& context,
		const WorldRenderView& worldRenderView,
		Object* renderable
	) override final;

	virtual void setup(
		const WorldSetupContext& context
	) override final;

	virtual void build(
		const WorldBuildContext& context,
		const WorldRenderView& worldRenderView,
		const IWorldRenderPass& worldRenderPass,
		Object* renderable
	) override final;

	virtual void build(
		const WorldBuildContext& context,
		const WorldRenderView& worldRenderView,
		const IWorldRenderPass& worldRenderPass
	) override final;

private:
	Ref< resource::IResourceManager > m_resourceManager;
	Ref< render::IRenderSystem > m_renderSystem;
	const TypeInfo& m_worldRendererType;
	Ref< IWorldRenderer > m_worldRenderer;
	resource::Proxy< render::Shader > m_filterShader;
	Ref< const render::IVertexLayout > m_vertexLayout;
	Ref< render::Buffer > m_vertexBuffer;
	Ref< render::Buffer > m_indexBuffer;
	Ref< render::ScreenRenderer > m_screenRenderer;
	RefArray< const ProbeComponent > m_captureQueue;	//!< Probes queued for capture.
	Ref< const ProbeComponent > m_capture;				//!< Probe being captured.
	int32_t m_captureState;
	int32_t m_captureMip;
};

}
