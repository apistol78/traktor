#include "Render/ISimpleTexture.h"
#include "Render/Shader.h"
#include "Render/Types.h"
#include "Render/Context/ProgramParameters.h"
#include "World/WorldRenderView.h"
#include "World/PreLit/WorldRenderPassPreLit.h"

namespace traktor
{
	namespace world
	{
		namespace
		{

bool s_handlesInitialized = false;
render::handle_t s_techniquePreLitColor;
render::handle_t s_handleProjection;
render::handle_t s_handleSquareProjection;
render::handle_t s_handleView;
render::handle_t s_handleWorld;
render::handle_t s_handleLightMap;
render::handle_t s_handleFogEnable;
render::handle_t s_handleFogDistanceAndRange;
render::handle_t s_handleFogColor;
render::handle_t s_handleShadowEnable;
render::handle_t s_handleShadowMask;
render::handle_t s_handleShadowMaskSize;
render::handle_t s_handleDepthEnable;
render::handle_t s_handleDepthMap;
render::handle_t s_handleTime;

void initializeHandles()
{
	if (s_handlesInitialized)
		return;

	s_techniquePreLitColor = render::getParameterHandle(L"World_PreLitColor");

	s_handleProjection = render::getParameterHandle(L"Projection");
	s_handleSquareProjection = render::getParameterHandle(L"SquareProjection");
	s_handleView = render::getParameterHandle(L"View");
	s_handleWorld = render::getParameterHandle(L"World");
	s_handleLightMap = render::getParameterHandle(L"LightMap");
	s_handleFogEnable = render::getParameterHandle(L"FogEnable");
	s_handleFogDistanceAndRange = render::getParameterHandle(L"FogDistanceAndRange");
	s_handleFogColor = render::getParameterHandle(L"FogColor");
	s_handleShadowEnable = render::getParameterHandle(L"ShadowEnable");
	s_handleShadowMask = render::getParameterHandle(L"ShadowMask");
	s_handleShadowMaskSize = render::getParameterHandle(L"ShadowMaskSize");
	s_handleDepthEnable = render::getParameterHandle(L"DepthEnable");
	s_handleDepthMap = render::getParameterHandle(L"DepthMap");
	s_handleTime = render::getParameterHandle(L"Time");

	s_handlesInitialized = true;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.WorldRenderPassPreLit", WorldRenderPassPreLit, IWorldRenderPass)

WorldRenderPassPreLit::WorldRenderPassPreLit(
	render::handle_t technique,
	const WorldRenderView& worldRenderView,
	bool fogEnabled,
	float fogDistance,
	float fogRange,
	const Vector4& fogColor,
	render::ISimpleTexture* depthMap,
	render::ISimpleTexture* lightMap
)
:	m_technique(technique)
,	m_worldRenderView(worldRenderView)
,	m_fogEnabled(fogEnabled)
,	m_fogDistance(fogDistance)
,	m_fogRange(fogRange)
,	m_fogColor(fogColor)
,	m_depthMap(depthMap)
,	m_shadowMask(0)
,	m_lightMap(lightMap)
{
	initializeHandles();
}

WorldRenderPassPreLit::WorldRenderPassPreLit(
	render::handle_t technique,
	const WorldRenderView& worldRenderView
)
:	m_technique(technique)
,	m_worldRenderView(worldRenderView)
,	m_fogEnabled(false)
,	m_fogDistance(0.0f)
,	m_fogRange(0.0f)
,	m_fogColor(0.0f, 0.0f, 0.0f, 0.0f)
,	m_depthMap(0)
,	m_shadowMask(0)
,	m_lightMap(0)
{
	initializeHandles();
}

render::handle_t WorldRenderPassPreLit::getTechnique() const
{
	return m_technique;
}

void WorldRenderPassPreLit::setShaderTechnique(render::Shader* shader) const
{
	shader->setTechnique(m_technique);
}

void WorldRenderPassPreLit::setShaderCombination(render::Shader* shader) const
{
	if (m_technique == s_techniquePreLitColor)
	{
		shader->setCombination(s_handleFogEnable, m_fogEnabled);
		shader->setCombination(s_handleDepthEnable, m_depthMap != 0);
	}
}

void WorldRenderPassPreLit::setShaderCombination(render::Shader* shader, const Matrix44& world, const Aabb3& bounds) const
{
	if (m_technique == s_techniquePreLitColor)
	{
		shader->setCombination(s_handleFogEnable, m_fogEnabled);
		shader->setCombination(s_handleDepthEnable, m_depthMap != 0);
	}
}

void WorldRenderPassPreLit::setProgramParameters(render::ProgramParameters* programParams) const
{
	setWorldProgramParameters(programParams, Matrix44::identity());

	if (m_technique == s_techniquePreLitColor)
	{
		setDepthMapProgramParameters(programParams);
		setLightMapProgramParameters(programParams);
		setFogProgramParameters(programParams);
	}
}

void WorldRenderPassPreLit::setProgramParameters(render::ProgramParameters* programParams, const Matrix44& world, const Aabb3& bounds) const
{
	setWorldProgramParameters(programParams, world);

	if (m_technique == s_techniquePreLitColor)
	{
		setDepthMapProgramParameters(programParams);
		setLightMapProgramParameters(programParams);
		setFogProgramParameters(programParams);
	}
}

void WorldRenderPassPreLit::setWorldProgramParameters(render::ProgramParameters* programParams, const Matrix44& world) const
{
	programParams->setFloatParameter(s_handleTime, m_worldRenderView.getTime());
	programParams->setMatrixParameter(s_handleProjection, m_worldRenderView.getProjection());
	programParams->setMatrixParameter(s_handleSquareProjection, m_worldRenderView.getSquareProjection());
	programParams->setMatrixParameter(s_handleView, m_worldRenderView.getView());
	programParams->setMatrixParameter(s_handleWorld, world);
}

void WorldRenderPassPreLit::setShadowMapProgramParameters(render::ProgramParameters* programParams) const
{
	if (m_shadowMask)
	{
		programParams->setTextureParameter(s_handleShadowMask, m_shadowMask);
		programParams->setFloatParameter(s_handleShadowMaskSize, float(0.5f / m_shadowMask->getWidth()));
	}
}

void WorldRenderPassPreLit::setDepthMapProgramParameters(render::ProgramParameters* programParams) const
{
	if (m_depthMap)
		programParams->setTextureParameter(s_handleDepthMap, m_depthMap);
}

void WorldRenderPassPreLit::setLightMapProgramParameters(render::ProgramParameters* programParams) const
{
	if (m_lightMap)
		programParams->setTextureParameter(s_handleLightMap, m_lightMap);
}

void WorldRenderPassPreLit::setFogProgramParameters(render::ProgramParameters* programParams) const
{
	if (m_fogEnabled)
	{
		programParams->setVectorParameter(s_handleFogDistanceAndRange, Vector4(m_fogDistance, m_fogRange, 1.0f / m_fogDistance, 1.0f / m_fogRange));
		programParams->setVectorParameter(s_handleFogColor, m_fogColor);
	}
}

	}
}
