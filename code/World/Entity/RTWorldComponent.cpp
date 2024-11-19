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
	T_FATAL_ASSERT_M(m_instances.empty(), L"RT instances not empty.");
	safeDestroy(m_tlas);
	m_renderSystem = nullptr;
}

void RTWorldComponent::update(World* world, const UpdateParams& update)
{
}

RTWorldComponent::Instance* RTWorldComponent::createInstance(const render::IAccelerationStructure* blas, const render::Buffer* perPrimitiveColor)
{
	RefArray< const render::IAccelerationStructure > blases;
	blases.push_back(blas);
	return createInstance(blases, perPrimitiveColor);
}

RTWorldComponent::Instance* RTWorldComponent::createInstance(const RefArray< const render::IAccelerationStructure >& blas, const render::Buffer* perPrimitiveColor)
{
	Instance* instance = new Instance();
	instance->owner = this;
	instance->transform = Transform::identity();
	instance->blas = blas;
	instance->perPrimitiveColor = perPrimitiveColor;

	m_instances.push_back(instance);
	m_instanceBufferDirty = true;

	return instance;
}

void RTWorldComponent::build(const WorldBuildContext& context)
{
	if (m_instanceBufferDirty)
	{
		AlignedVector< render::IAccelerationStructure::Instance > tlasInstances;
		for (const auto& instance : m_instances)
		{
			for (auto blas : instance->blas)
			{
				tlasInstances.push_back({
					.blas = blas,
					.perPrimitiveVec4 = instance->perPrimitiveColor,
					.transform = instance->transform.toMatrix44()
				});
			}
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
