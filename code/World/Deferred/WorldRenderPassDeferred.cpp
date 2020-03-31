#include "Render/Shader.h"
#include "Render/Context/ProgramParameters.h"
#include "World/WorldHandles.h"
#include "World/WorldRenderView.h"
#include "World/Deferred/WorldRenderPassDeferred.h"

namespace traktor
{
	namespace world
	{
		namespace
		{

enum { MaxForwardLightCount = 2 };

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.WorldRenderPassDeferred", WorldRenderPassDeferred, IWorldRenderPass)

WorldRenderPassDeferred::WorldRenderPassDeferred(
	render::handle_t technique,
	render::ProgramParameters* sharedParams,
	const WorldRenderView& worldRenderView,
	uint32_t passFlags,
	bool fogEnabled,
	bool depthEnable,
	bool irradianceEnable
)
:	m_technique(technique)
,	m_sharedParams(sharedParams)
,	m_worldRenderView(worldRenderView)
,	m_passFlags(passFlags)
,	m_fogEnabled(fogEnabled)
,	m_depthEnable(depthEnable)
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

	if (m_technique == s_techniqueDeferredColor)
	{
		shader->setCombination(s_handleFogEnable, m_fogEnabled, perm);
		shader->setCombination(s_handleDepthEnable, m_depthEnable, perm);
		shader->setCombination(s_handleIrradianceEnable, m_irradianceEnable, perm);
	}
	else
		shader->setCombination(s_handleIrradianceEnable, m_irradianceEnable, perm);

	return perm;
}

void WorldRenderPassDeferred::setProgramParameters(render::ProgramParameters* programParams) const
{
	setWorldProgramParameters(programParams, Transform::identity(), Transform::identity());
}

void WorldRenderPassDeferred::setProgramParameters(render::ProgramParameters* programParams, const Transform& lastWorld, const Transform& world, const Aabb3& bounds) const
{
	setWorldProgramParameters(programParams, lastWorld, world);
}

void WorldRenderPassDeferred::setWorldProgramParameters(render::ProgramParameters* programParams, const Transform& lastWorld, const Transform& world) const
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
