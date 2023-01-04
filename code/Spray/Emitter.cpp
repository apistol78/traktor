/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Shader.h"
#include "Spray/Emitter.h"
#include "Spray/EmitterInstance.h"
#include "Spray/Modifier.h"
#include "Spray/Source.h"

namespace traktor::spray
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.Emitter", Emitter, Object)

Emitter::Emitter(
	const Source* source,
	const RefArray< const Modifier >& modifiers,
	const resource::Proxy< render::Shader >& shader,
	const resource::Proxy< mesh::InstanceMesh >& mesh,
	const Effect* effect,
	float middleAge,
	float cullNearDistance,
	float cullMeshDistance,
	float fadeNearRange,
	float viewOffset,
	float warmUp,
	bool sort,
	bool worldSpace,
	bool meshOrientationFromVelocity
)
:	m_source(source)
,	m_modifiers(modifiers)
,	m_shader(shader)
,	m_mesh(mesh)
,	m_effect(effect)
,	m_middleAge(middleAge)
,	m_cullNearDistance(cullNearDistance)
,	m_cullMeshDistance(cullMeshDistance)
,	m_fadeNearRange(fadeNearRange)
,	m_viewOffset(viewOffset)
,	m_warmUp(warmUp)
,	m_sort(sort)
,	m_worldSpace(worldSpace)
,	m_meshOrientationFromVelocity(meshOrientationFromVelocity)
{
}

Ref< EmitterInstance > Emitter::createInstance(float duration) const
{
	return new EmitterInstance(this, duration);
}

}
