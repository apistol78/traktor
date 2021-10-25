#include "Render/Shader.h"
#include "Render/Context/ProgramParameters.h"
#include "World/WorldHandles.h"
#include "World/WorldRenderView.h"
#include "World/Forward/WorldRenderPassForward.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.WorldRenderPassForward", WorldRenderPassForward, IWorldRenderPass)

WorldRenderPassForward::WorldRenderPassForward(
	render::handle_t technique,
	render::ProgramParameters* sharedParams,
	const WorldRenderView& worldRenderView,
	uint32_t passFlags,
	bool irradianceEnable,
	bool shadowEnable,
	bool reflectionsEnable
)
:	m_technique(technique)
,	m_sharedParams(sharedParams)
,	m_worldRenderView(worldRenderView)
,	m_passFlags(passFlags)
,	m_irradianceEnable(irradianceEnable)
,	m_shadowEnable(shadowEnable)
,	m_reflectionsEnable(reflectionsEnable)
{
}

WorldRenderPassForward::WorldRenderPassForward(
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

render::handle_t WorldRenderPassForward::getTechnique() const
{
	return m_technique;
}

uint32_t WorldRenderPassForward::getPassFlags() const
{
	return m_passFlags;
}

render::Shader::Permutation WorldRenderPassForward::getPermutation(const render::Shader* shader) const
{
	render::Shader::Permutation perm(m_technique);
	shader->setCombination(s_handleIrradianceEnable, m_irradianceEnable, perm);
	shader->setCombination(s_handleShadowEnable, m_shadowEnable, perm);
	shader->setCombination(s_handleReflectionsEnable, m_reflectionsEnable, perm);
	return perm;
}

void WorldRenderPassForward::setProgramParameters(render::ProgramParameters* programParams) const
{
	setWorldProgramParameters(programParams, Transform::identity(), Transform::identity());
}

void WorldRenderPassForward::setProgramParameters(render::ProgramParameters* programParams, const Transform& lastWorld, const Transform& world) const
{
	setWorldProgramParameters(programParams, lastWorld, world);
}

void WorldRenderPassForward::setWorldProgramParameters(render::ProgramParameters* programParams, const Transform& lastWorld, const Transform& world) const
{
	programParams->attachParameters(m_sharedParams);

	Matrix44 w = world.toMatrix44();
	programParams->setMatrixParameter(s_handleWorld, w);
	programParams->setMatrixParameter(s_handleWorldView, m_worldRenderView.getView() * w);

	if (m_technique == s_techniqueVelocityWrite)
	{
		Matrix44 w0 = lastWorld.toMatrix44();
		programParams->setMatrixParameter(s_handleLastWorld, w0);
		programParams->setMatrixParameter(s_handleLastWorldView, m_worldRenderView.getLastView() * w0);
	}
}

	}
}
