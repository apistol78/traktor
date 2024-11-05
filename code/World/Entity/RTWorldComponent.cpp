/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/SafeDestroy.h"
#include "Render/IAccelerationStructure.h"
#include "Render/IProgramDispatchTable.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/Context/RenderContext.h"
#include "World/WorldBuildContext.h"
#include "World/Entity/RTWorldComponent.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.RTWorldComponent", RTWorldComponent, IWorldComponent)

RTWorldComponent::RTWorldComponent(render::IRenderSystem* renderSystem)
:	m_renderSystem(renderSystem)
{
	m_tlas = renderSystem->createTopLevelAccelerationStructure(1024);
}

void RTWorldComponent::destroy()
{
	T_FATAL_ASSERT_M(m_instances.empty(), L"Culling instances not empty.");
	safeDestroy(m_tlas);
	m_renderSystem = nullptr;
}

void RTWorldComponent::update(World* world, const UpdateParams& update)
{
}

RTWorldComponent::Instance* RTWorldComponent::allocateInstance(const render::IAccelerationStructure* blas, render::IProgram* program)
{
	Instance* instance = new Instance();
	instance->owner = this;
	instance->transform = Transform::identity();
	instance->blas = blas;
	instance->program = program;

	m_instances.push_back(instance);

	m_instanceBufferDirty = true;
	m_programTableDirty = true;

	return instance;
}

void RTWorldComponent::releaseInstance(Instance*& instance)
{
	T_FATAL_ASSERT(instance->owner == this);

	auto it = std::find(m_instances.begin(), m_instances.end(), instance);
	T_FATAL_ASSERT(it != m_instances.end());
	
	m_instances.erase(it);

	delete instance;
	instance = nullptr;
	
	m_instanceBufferDirty = true;
	m_programTableDirty = true;
}

void RTWorldComponent::build(const WorldBuildContext& context)
{
	if (m_instanceBufferDirty)
	{
		// Update TLAS with all instances.
		AlignedVector< render::IAccelerationStructure::Instance > tlasInstances;
		for (const auto& instance : m_instances)
		{
			tlasInstances.push_back({
				.transform = instance->transform.toMatrix44(),
				.blas = instance->blas
			});
		}

		render::RenderContext* renderContext = context.getRenderContext();
		T_ASSERT(renderContext);

		auto rb = renderContext->allocNamed< render::LambdaRenderBlock >(L"RTWorldComponent");
		rb->lambda = [=, this](render::IRenderView* renderView)
		{
			renderView->writeAccelerationStructure(m_tlas, tlasInstances);
		};
		renderContext->compute(rb);

		m_instanceBufferDirty = false;
	}

	if (m_programTableDirty)
	{
		m_programTable = m_renderSystem->createProgramDispatchTable();

		for (const auto& instance : m_instances)
			m_programTable->addProgram(instance->program);

		m_programTableDirty = false;
	}
}

void RTWorldComponent::Instance::setTransform(const Transform& transform)
{
	this->transform = transform;
	this->owner->m_instanceBufferDirty = true;
}

}
