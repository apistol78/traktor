/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/Transform.h"
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

class IAccelerationStructure;
class IRenderSystem;

}

namespace traktor::world
{

class IWorldRenderPass;
class WorldBuildContext;
class WorldRenderView;

/*! Ray tracing world.
 * \ingroup World
 *
 * The RT world contains information about all instances
 * and ensure the top level (TLAS) structure is updated
 * as required.
 * 
 * The world renderer uses component's TLAS when
 * exposing to entity renderers during rastization
 * passes.
 */
class T_DLLCLASS RTWorldComponent : public IWorldComponent
{
	T_RTTI_CLASS;

public:
	struct T_DLLCLASS Instance
	{
		RTWorldComponent* owner;
		Transform transform;
		const render::IAccelerationStructure* blas;

		void setTransform(const Transform& transform);
	};

	explicit RTWorldComponent(render::IRenderSystem* renderSystem);
	
	virtual void destroy() override final;

	virtual void update(World* world, const UpdateParams& update) override final;

	Instance* allocateInstance(const render::IAccelerationStructure* blas);

	void releaseInstance(Instance*& instance);

	void setup();

	const render::IAccelerationStructure* getTLAS() const { return m_tlas; }

private:
	Ref< render::IRenderSystem > m_renderSystem;
	Ref< render::IAccelerationStructure > m_tlas;
	AlignedVector< Instance* > m_instances;
	bool m_instanceBufferDirty = false;
};

}
