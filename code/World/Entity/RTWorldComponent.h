/*
 * TRAKTOR
 * Copyright (c) 2024-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Core/RefArray.h"
#include "Core/Math/Transform.h"
#include "Core/Math/Vector4.h"
#include "Render/IAccelerationStructure.h"
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
class IProgram;
class IRenderSystem;

}

namespace traktor::world
{

/*! Ray tracing world.
 * \ingroup World
 *
 * The RT world contains information about all instances
 * and ensure the top level (TLAS) structure is updated
 * as required.
 * 
 * The world renderer uses component's TLAS when
 * exposing to entity renderers during rasterization
 * passes.
 */
class T_DLLCLASS RTWorldComponent : public IWorldComponent
{
	T_RTTI_CLASS;

public:
	struct Part
	{
		Ref< const render::IAccelerationStructure > blas;
		Ref< const render::Buffer > perVertexData;
	};

	struct T_DLLCLASS Instance
	{
		RTWorldComponent* owner;
		Transform transform;
		AlignedVector< Part > parts;
		bool cullingEnable;

		void destroy();

		/*! Set transform of instance, will automatically tag top level as dirty. */
		void setTransform(const Transform& transform);

		/*! Tag top level as dirty, might be necessary after bottom level has been updated. */
		void setDirty();
	};

	explicit RTWorldComponent(render::IRenderSystem* renderSystem);

	virtual void destroy() override final;

	virtual void update(World* world, const UpdateParams& update) override final;

	Instance* createInstance(const render::IAccelerationStructure* blas, const render::Buffer* perVertexData);

	Instance* createInstance(const AlignedVector< Part >& parts, bool cullingEnable);

	render::IAccelerationStructure* gatherTopLevelInstances(const Vector4& eyePosition, float farDistance, AlignedVector< render::IAccelerationStructure::Instance >& outInstances);

	const render::IAccelerationStructure* getTopLevel() const { return m_tlas; }

private:
	Ref< render::IRenderSystem > m_renderSystem;
	Ref< render::IAccelerationStructure > m_tlas;
	AlignedVector< Instance* > m_instances;
	Vector4 m_lastEyePosition = Vector4::zero();
	uint32_t m_tlasCapacity = 0;
	bool m_instanceBufferDirty = true;

	void destroyInstance(Instance* instance);

	//! Ensure the top level structure can hold at least the given number of instances,
	//! recreating it at a larger capacity if required. Growth is handled here rather than
	//! in the renderer backend so backends only ever implement a fixed-capacity build.
	void ensureTopLevelCapacity(uint32_t numInstances);

	//! Total number of top level instances (summed across all instances' parts).
	uint32_t countInstances() const;
};

}
