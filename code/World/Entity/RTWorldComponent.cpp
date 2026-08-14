/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>

#include "Core/Math/Const.h"
#include "Core/Misc/Align.h"
#include "Core/Misc/SafeDestroy.h"
#include "Render/IAccelerationStructure.h"
#include "Render/IRenderSystem.h"
#include "World/Entity/RTWorldComponent.h"

namespace traktor::world
{
namespace
{

// Top level instance capacity is grown in multiples of this so small fluctuations in the
// instance count do not trigger a reallocation of the structure every frame.
const uint32_t c_tlasCapacityGranularity = 1024;

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.RTWorldComponent", RTWorldComponent, IWorldComponent)

RTWorldComponent::RTWorldComponent(render::IRenderSystem* renderSystem)
:	m_renderSystem(renderSystem)
{
	ensureTopLevelCapacity(0);
}

void RTWorldComponent::destroy()
{
	T_FATAL_ASSERT_M(m_instances.empty(), L"RT instances not empty.");
	safeDestroy(m_tlas);
	m_renderSystem = nullptr;
}

void RTWorldComponent::update(World* world, const UpdateParams& update)
{
}

RTWorldComponent::Instance* RTWorldComponent::createInstance(const render::IAccelerationStructure* blas, const render::Buffer* perVertexData)
{
	AlignedVector< Part > parts;
	parts.push_back({ blas, perVertexData });
	return createInstance(parts, true);
}

RTWorldComponent::Instance* RTWorldComponent::createInstance(const AlignedVector< Part >& parts, bool cullingEnable)
{
	Instance* instance = new Instance();
	instance->owner = this;
	instance->transform = Transform::identity();
	instance->parts = parts;
	instance->cullingEnable = cullingEnable;

	m_instances.push_back(instance);
	m_instanceBufferDirty = true;

	// Grow the top level structure here, on the instance-mutation path, so the (possibly
	// recreated) structure is in place before the world renderer gathers it for the frame.
	ensureTopLevelCapacity(countInstances());

	return instance;
}

render::IAccelerationStructure* RTWorldComponent::gatherTopLevelInstances(const Vector4& eyePosition, float farDistance, AlignedVector< render::IAccelerationStructure::Instance >& outInstances)
{
	// Distance culling depends on the camera, so re-dirty when the eye moves; otherwise the
	// culled set would never refresh on a static scene with a moving camera.
	const Scalar movmentGranularity = 0.5_simd;
	if ((eyePosition - m_lastEyePosition).length() > movmentGranularity)
	{
		m_lastEyePosition = eyePosition;
		m_instanceBufferDirty = true;
	}

	if (!m_instanceBufferDirty || m_tlas == nullptr)
		return nullptr;

	const Scalar cullDistance(farDistance * 0.5f);

	outInstances.resize(0);
	outInstances.reserve(m_instances.size() * 2); // Assume 2 parts per instance on average.

	for (const auto& instance : m_instances)
	{
		if (instance->cullingEnable)
		{
			const Scalar distance = (instance->transform.translation().xyz0() - eyePosition.xyz0()).length();
			if (distance > cullDistance)
				continue;
		}
		for (auto part : instance->parts)
		{
			outInstances.push_back({
				.blas = part.blas,
				.perVertexData = part.perVertexData,
				.transform = instance->transform.toMatrix44()
			});
		}
	}

	m_instanceBufferDirty = false;
	return m_tlas;
}

void RTWorldComponent::destroyInstance(Instance* instance)
{
	T_FATAL_ASSERT(instance->owner == this);

	auto it = std::find(m_instances.begin(), m_instances.end(), instance);
	T_FATAL_ASSERT(it != m_instances.end());
	
	m_instances.erase(it);
	m_instanceBufferDirty = true;

	delete instance;
}

uint32_t RTWorldComponent::countInstances() const
{
	uint32_t count = 0;
	for (const auto& instance : m_instances)
		count += (uint32_t)instance->parts.size();
	return count;
}

void RTWorldComponent::ensureTopLevelCapacity(uint32_t numInstances)
{
	if (m_tlas != nullptr && numInstances <= m_tlasCapacity)
		return;

	// Round up so the structure has headroom and does not need to be recreated for every
	// added instance. The previous structure, if any, is retired safely (deferred until its
	// retirement fence has passed) so in-flight frames keep a valid one.
	const uint32_t capacity = alignUp(std::max(numInstances, c_tlasCapacityGranularity), c_tlasCapacityGranularity);

	Ref< render::IAccelerationStructure > tlas = m_renderSystem->createTopLevelAccelerationStructure(capacity);
	if (!tlas)
		return;

	safeDestroy(m_tlas);
	m_tlas = tlas;
	m_tlasCapacity = capacity;
	m_instanceBufferDirty = true;
}

void RTWorldComponent::Instance::destroy()
{
	T_FATAL_ASSERT(this->owner);
	this->owner->destroyInstance(this);
}

void RTWorldComponent::Instance::setTransform(const Transform& transform)
{
	this->transform = transform;
	this->owner->m_instanceBufferDirty = true;
}

void RTWorldComponent::Instance::setDirty()
{
	this->owner->m_instanceBufferDirty = true;
}

}
