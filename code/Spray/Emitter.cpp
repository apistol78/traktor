/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Shader.h"
#include "Spray/Emitter.h"
#include "Spray/EmitterInstanceCPU.h"
#include "Spray/EmitterInstanceGPU.h"
#include "Spray/Modifier.h"
#include "Spray/Source.h"

namespace traktor::spray
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.Emitter", Emitter, Object)

Emitter::Emitter(
	const EmitterData* data,
	const Source* source,
	const RefArray< const Modifier >& modifiers,
	const resource::Proxy< render::Shader >& shader,
	const resource::Proxy< mesh::InstanceMesh >& mesh,
	const Effect* effect
)
:	m_data(data)
,	m_source(source)
,	m_modifiers(modifiers)
,	m_shader(shader)
,	m_mesh(mesh)
,	m_effect(effect)
{
}

Ref< IEmitterInstance > Emitter::createInstance(render::IRenderSystem* renderSystem, resource::IResourceManager* resourceManager, float duration) const
{
	if (m_data->m_gpu)
		return EmitterInstanceGPU::createInstance(renderSystem, resourceManager, this, duration);
	else
		return EmitterInstanceCPU::createInstance(renderSystem, this, duration);
}

}
