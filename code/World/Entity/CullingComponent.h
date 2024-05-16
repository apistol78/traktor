/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Aabb3.h"
#include "Resource/Proxy.h"
#include "World/IWorldComponent.h"

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
class Shader;

}

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::world
{

class IWorldRenderPass;
class WorldBuildContext;
class WorldRenderView;

/*!
 * \ingroup Mesh
 */
class T_DLLCLASS CullingComponent : public IWorldComponent
{
	T_RTTI_CLASS;

public:
#pragma pack(1)
	struct T_MATH_ALIGN16 InstanceRenderData
	{
		float rotation[4];
		float translation[4];
		float boundingBoxMin[4];
		float boundingBoxMax[4];
	};
#pragma pack()

	struct T_DLLCLASS ICullable
	{
		virtual Aabb3 cullableGetBoundingBox() const = 0;

		virtual void cullableBuild(
			const WorldBuildContext& context,
			const world::WorldRenderView& worldRenderView,
			const world::IWorldRenderPass& worldRenderPass,
			render::Buffer* instanceBuffer,
			render::Buffer* visibilityBuffer,
			uint32_t start,
			uint32_t count
		) = 0;
	};

	struct T_DLLCLASS Instance
	{
		CullingComponent* owner;
		ICullable* cullable;
		intptr_t ordinal;
		Transform transform;
		Aabb3 boundingBox;

		void setTransform(const Transform& transform);
	};

	explicit CullingComponent(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem);
	
	virtual void destroy() override final;

	virtual void update(World* world, const UpdateParams& update) override final;

	void build(
		const WorldBuildContext& context,
		const WorldRenderView& worldRenderView,
		const IWorldRenderPass& worldRenderPass
	);

	Instance* allocateInstance(ICullable* cullable, intptr_t ordinal);

	void releaseInstance(Instance*& instance);

private:
	Ref< render::IRenderSystem > m_renderSystem;
	resource::Proxy< render::Shader > m_shaderCull;
	AlignedVector< Instance* > m_instances;
	Ref< render::Buffer > m_instanceBuffer;
	RefArray< render::Buffer > m_visibilityBuffers;
	uint32_t m_instanceAllocatedCount = 0;
	bool m_instanceBufferDirty = false;
};

}
