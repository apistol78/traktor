#include "Render/Shader.h"
#include "Render/Context/ProgramParameters.h"
#include "World/WorldHandles.h"
#include "World/WorldRenderView.h"
#include "World/Deferred/WorldRenderPassDeferred.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.WorldRenderPassDeferred", WorldRenderPassDeferred, IWorldRenderPass)

WorldRenderPassDeferred::WorldRenderPassDeferred(
	render::handle_t technique,
	render::ProgramParameters* sharedParams,
	const WorldRenderView& worldRenderView,
	uint32_t passFlags,
	bool irradianceEnable
)
:	m_technique(technique)
,	m_sharedParams(sharedParams)
,	m_worldRenderView(worldRenderView)
,	m_passFlags(passFlags)
,	m_irradianceEnable(irradianceEnable)
{
}

render::handle_t WorldRenderPassDeferred::getTechnique() const
{
	return m_technique;
}

uint32_t WorldRenderPassDeferred::getPassFlags() const
{
	return m_passFlags;
}

render::Shader::Permutation WorldRenderPassDeferred::getPermutation(const render::Shader* shader) const
{
	render::Shader::Permutation perm(m_technique);
	shader->setCombination(s_handleIrradianceEnable, m_irradianceEnable, perm);
	return perm;
}

void WorldRenderPassDeferred::setProgramParameters(render::ProgramParameters* programParams) const
{
	setWorldProgramParameters(programParams, Transform::identity(), Transform::identity());
}

void WorldRenderPassDeferred::setProgramParameters(render::ProgramParameters* programParams, const Transform& lastWorld, const Transform& world) const
{
	setWorldProgramParameters(programParams, lastWorld, world);
}

void WorldRenderPassDeferred::setWorldProgramParameters(render::ProgramParameters* programParams, const Transform& lastWorld, const Transform& world) const
{
	programParams->attachParameters(m_sharedParams);

	const Matrix44 w = world.toMatrix44();
	programParams->setMatrixParameter(s_handleWorld, w);
	programParams->setMatrixParameter(s_handleWorldView, m_worldRenderView.getView() * w);

	if (m_technique == s_techniqueVelocityWrite)
	{
		const Matrix44 w0 = lastWorld.toMatrix44();
		programParams->setMatrixParameter(s_handleLastWorld, w0);
		programParams->setMatrixParameter(s_handleLastWorldView, m_worldRenderView.getLastView() * w0);
	}
}

}
