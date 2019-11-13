#include "Render/Shader.h"
#include "Render/Context/ProgramParameters.h"
#include "World/WorldRenderView.h"
#include "World/Deferred/WorldRenderPassDeferred.h"

namespace traktor
{
	namespace world
	{
		namespace
		{

enum { MaxForwardLightCount = 2 };

bool s_handlesInitialized = false;
render::handle_t s_techniqueDeferredColor;
render::handle_t s_techniqueVelocityWrite;
render::handle_t s_techniqueIrradianceWrite;
render::handle_t s_handleWorld;
render::handle_t s_handleWorldView;
render::handle_t s_handleLastWorld;
render::handle_t s_handleLastWorldView;
render::handle_t s_handleFogEnable;
render::handle_t s_handleDepthEnable;
render::handle_t s_handleIrradianceEnable;

void initializeHandles()
{
	if (s_handlesInitialized)
		return;

	s_techniqueDeferredColor = render::getParameterHandle(L"World_DeferredColor");
	s_techniqueVelocityWrite = render::getParameterHandle(L"World_VelocityWrite");
	s_techniqueIrradianceWrite = render::getParameterHandle(L"World_IrradianceWrite");

	s_handleWorld = render::getParameterHandle(L"World_World");
	s_handleWorldView = render::getParameterHandle(L"World_WorldView");
	s_handleLastWorld = render::getParameterHandle(L"World_LastWorld");
	s_handleLastWorldView = render::getParameterHandle(L"World_LastWorldView");
	s_handleFogEnable = render::getParameterHandle(L"World_FogEnable");
	s_handleDepthEnable = render::getParameterHandle(L"World_DepthEnable");
	s_handleIrradianceEnable = render::getParameterHandle(L"World_IrradianceEnable");

	s_handlesInitialized = true;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.WorldRenderPassDeferred", WorldRenderPassDeferred, IWorldRenderPass)

WorldRenderPassDeferred::WorldRenderPassDeferred(
	render::handle_t technique,
	const WorldRenderView& worldRenderView,
	uint32_t passFlags,
	bool fogEnabled,
	bool depthEnable
)
:	m_technique(technique)
,	m_worldRenderView(worldRenderView)
,	m_passFlags(passFlags)
,	m_fogEnabled(fogEnabled)
,	m_depthEnable(depthEnable)
,	m_irradianceEnable(false)
{
	initializeHandles();
}

WorldRenderPassDeferred::WorldRenderPassDeferred(
	render::handle_t technique,
	const WorldRenderView& worldRenderView,
	uint32_t passFlags,
	bool irradianceEnable
)
:	m_technique(technique)
,	m_worldRenderView(worldRenderView)
,	m_passFlags(passFlags)
,	m_fogEnabled(false)
,	m_depthEnable(false)
,	m_irradianceEnable(irradianceEnable)
{
	initializeHandles();
}

render::handle_t WorldRenderPassDeferred::getTechnique() const
{
	return m_technique;
}

uint32_t WorldRenderPassDeferred::getPassFlags() const
{
	return m_passFlags;
}

void WorldRenderPassDeferred::setShaderTechnique(render::Shader* shader) const
{
	shader->setTechnique(m_technique);
}

void WorldRenderPassDeferred::setShaderCombination(render::Shader* shader) const
{
	if (m_technique == s_techniqueDeferredColor)
	{
		shader->setCombination(s_handleFogEnable, m_fogEnabled);
		shader->setCombination(s_handleDepthEnable, m_depthEnable);
	}
	else if (m_technique == s_techniqueIrradianceWrite)
		shader->setCombination(s_handleIrradianceEnable, m_irradianceEnable);
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
