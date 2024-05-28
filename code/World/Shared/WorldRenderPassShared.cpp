/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Shader.h"
#include "Render/Context/ProgramParameters.h"
#include "World/WorldHandles.h"
#include "World/WorldRenderView.h"
#include "World/Shared/WorldRenderPassShared.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.WorldRenderPassShared", WorldRenderPassShared, IWorldRenderPass)

WorldRenderPassShared::WorldRenderPassShared(
	render::handle_t technique,
	render::ProgramParameters* sharedParams,
	const WorldRenderView& worldRenderView,
	uint32_t passFlags
)
:	m_technique(technique)
,	m_sharedParams(sharedParams)
,	m_worldRenderView(worldRenderView)
,	m_passFlags(passFlags)
{
}

WorldRenderPassShared::WorldRenderPassShared(
	render::handle_t technique,
	render::ProgramParameters* sharedParams,
	const WorldRenderView& worldRenderView,
	uint32_t passFlags,
	const std::initializer_list< TechniqueFlag >& techniqueFlags
)
:	m_technique(technique)
,	m_sharedParams(sharedParams)
,	m_worldRenderView(worldRenderView)
,	m_passFlags(passFlags)
,	m_techniqueFlags(techniqueFlags)
{
}

render::handle_t WorldRenderPassShared::getTechnique() const
{
	return m_technique;
}

uint32_t WorldRenderPassShared::getPassFlags() const
{
	return m_passFlags;
}

render::Shader::Permutation WorldRenderPassShared::getPermutation(const render::Shader* shader) const
{
	render::Shader::Permutation perm(m_technique);
	for (const auto& tf : m_techniqueFlags)
		shader->setCombination(tf.handle, tf.enable, perm);
	return perm;
}

void WorldRenderPassShared::setProgramParameters(render::ProgramParameters* programParams) const
{
	setWorldProgramParameters(programParams, Transform::identity(), Transform::identity());
}

void WorldRenderPassShared::setProgramParameters(render::ProgramParameters* programParams, const Transform& lastWorld, const Transform& world) const
{
	setWorldProgramParameters(programParams, lastWorld, world);
}

void WorldRenderPassShared::setWorldProgramParameters(render::ProgramParameters* programParams, const Transform& lastWorld, const Transform& world) const
{
	programParams->attachParameters(m_sharedParams);

	const Matrix44 w = world.toMatrix44();
	programParams->setMatrixParameter(s_handleWorld, w);
	programParams->setMatrixParameter(s_handleWorldView, m_worldRenderView.getView() * w);

	if (m_technique == s_techniqueVelocityWrite)
	{
		const Matrix44 w0 = lastWorld.toMatrix44();
		programParams->setMatrixParameter(s_handleLastWorld, w0);
		programParams->setMatrixParameter(s_handleLastView, m_worldRenderView.getLastView());
		programParams->setMatrixParameter(s_handleLastWorldView, m_worldRenderView.getLastView() * w0);
	}
}

	}
}
