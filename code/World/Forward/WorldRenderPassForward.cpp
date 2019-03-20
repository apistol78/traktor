#include "Render/ISimpleTexture.h"
#include "Render/Shader.h"
#include "Render/Types.h"
#include "Render/Context/ProgramParameters.h"
#include "World/WorldRenderView.h"
#include "World/Forward/WorldRenderPassForward.h"

namespace traktor
{
	namespace world
	{
		namespace
		{

bool s_handlesInitialized = false;
render::handle_t s_techniqueDefault;
render::handle_t s_handleView;
render::handle_t s_handleViewInverse;
render::handle_t s_handleWorld;
render::handle_t s_handleWorldView;
render::handle_t s_handleColorMap;
render::handle_t s_handleFogEnable;
render::handle_t s_handleFogDistanceAndDensity;
render::handle_t s_handleFogColor;
render::handle_t s_handleShadowEnable;
render::handle_t s_handleShadowMask;
render::handle_t s_handleDepthEnable;
render::handle_t s_handleDepthMap;
render::handle_t s_handleLightCount;
render::handle_t s_handleLights;

void initializeHandles()
{
	if (s_handlesInitialized)
		return;

	s_techniqueDefault = render::getParameterHandle(L"World_ForwardColor");

	s_handleView = render::getParameterHandle(L"World_View");
	s_handleViewInverse = render::getParameterHandle(L"World_ViewInverse");
	s_handleWorld = render::getParameterHandle(L"World_World");
	s_handleWorldView = render::getParameterHandle(L"World_WorldView");
	s_handleColorMap = render::getParameterHandle(L"World_ColorMap");
	s_handleFogEnable = render::getParameterHandle(L"World_FogEnable");
	s_handleFogDistanceAndDensity = render::getParameterHandle(L"World_FogDistanceAndDensity");
	s_handleFogColor = render::getParameterHandle(L"World_FogColor");
	s_handleShadowEnable = render::getParameterHandle(L"World_ShadowEnable");
	s_handleShadowMask = render::getParameterHandle(L"World_ShadowMask");
	s_handleDepthEnable = render::getParameterHandle(L"World_DepthEnable");
	s_handleDepthMap = render::getParameterHandle(L"World_DepthMap");
	s_handleLightCount = render::getParameterHandle(L"World_LightCount");
	s_handleLights = render::getParameterHandle(L"World_Lights");

	s_handlesInitialized = true;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.WorldRenderPassForward", WorldRenderPassForward, IWorldRenderPass)

WorldRenderPassForward::WorldRenderPassForward(
	render::handle_t technique,
	const WorldRenderView& worldRenderView,
	uint32_t passFlags,
	render::StructBuffer* lightSBuffer,
	uint32_t lightCount,
	bool fogEnabled,
	float fogDistanceY,
	float fogDistanceZ,
	float fogDensityY,
	float fogDensityZ,
	const Vector4& fogColor,
	render::ISimpleTexture* colorMap,
	render::ISimpleTexture* depthMap,
	render::ISimpleTexture* shadowMask
)
:	m_technique(technique)
,	m_worldRenderView(worldRenderView)
,	m_passFlags(passFlags)
,	m_lightSBuffer(lightSBuffer)
,	m_lightCount(lightCount)
,	m_fogEnabled(fogEnabled)
,	m_fogDistanceY(fogDistanceY)
,	m_fogDistanceZ(fogDistanceZ)
,	m_fogDensityY(fogDensityY)
,	m_fogDensityZ(fogDensityZ)
,	m_fogColor(fogColor)
,	m_colorMap(colorMap)
,	m_depthMap(depthMap)
,	m_shadowMask(shadowMask)
{
	initializeHandles();

	m_viewInverse = m_worldRenderView.getView().inverse();
}

WorldRenderPassForward::WorldRenderPassForward(
	render::handle_t technique,
	const WorldRenderView& worldRenderView,
	uint32_t passFlags,
	render::ISimpleTexture* colorMap,
	render::ISimpleTexture* depthMap
)
:	m_technique(technique)
,	m_worldRenderView(worldRenderView)
,	m_passFlags(passFlags)
,	m_lightSBuffer(nullptr)
,	m_lightCount(0)
,	m_fogEnabled(false)
,	m_fogDistanceY(0.0f)
,	m_fogDistanceZ(0.0f)
,	m_fogDensityY(0.0f)
,	m_fogDensityZ(0.0f)
,	m_fogColor(0.0f, 0.0f, 0.0f, 0.0f)
,	m_colorMap(colorMap)
,	m_depthMap(depthMap)
,	m_shadowMask(nullptr)
{
	initializeHandles();

	m_viewInverse = m_worldRenderView.getView().inverse();
}

render::handle_t WorldRenderPassForward::getTechnique() const
{
	return m_technique;
}

uint32_t WorldRenderPassForward::getPassFlags() const
{
	return m_passFlags;
}

void WorldRenderPassForward::setShaderTechnique(render::Shader* shader) const
{
	shader->setTechnique(m_technique);
}

void WorldRenderPassForward::setShaderCombination(render::Shader* shader) const
{
	if (m_technique == s_techniqueDefault)
	{
		shader->setCombination(s_handleFogEnable, m_fogEnabled);
		shader->setCombination(s_handleShadowEnable, m_shadowMask != 0);
		shader->setCombination(s_handleDepthEnable, m_depthMap != 0);
	}
}

void WorldRenderPassForward::setProgramParameters(render::ProgramParameters* programParams) const
{
	setWorldProgramParameters(programParams, Transform::identity());

	// Set these parameters only if we're rendering using default technique.
	if (m_technique == s_techniqueDefault)
	{
		setColorMapProgramParameters(programParams);
		setLightProgramParameters(programParams);
		setFogProgramParameters(programParams);
		setShadowMapProgramParameters(programParams);
		setDepthMapProgramParameters(programParams);
	}
}

void WorldRenderPassForward::setProgramParameters(render::ProgramParameters* programParams, const Transform& lastWorld, const Transform& world, const Aabb3& bounds) const
{
	setWorldProgramParameters(programParams, world);

	// Set these parameters only if we're rendering using default technique.
	if (m_technique == s_techniqueDefault)
	{
		setColorMapProgramParameters(programParams);
		setLightProgramParameters(programParams);
		setFogProgramParameters(programParams);
		setShadowMapProgramParameters(programParams);
		setDepthMapProgramParameters(programParams);
	}
}

void WorldRenderPassForward::setWorldProgramParameters(render::ProgramParameters* programParams, const Transform& world) const
{
	Matrix44 w = world.toMatrix44();
	const Matrix44& v = m_worldRenderView.getView();
	programParams->setMatrixParameter(s_handleView, v);
	programParams->setMatrixParameter(s_handleViewInverse, m_viewInverse);
	programParams->setMatrixParameter(s_handleWorld, w);
	programParams->setMatrixParameter(s_handleWorldView, v * w);
}

void WorldRenderPassForward::setLightProgramParameters(render::ProgramParameters* programParams) const
{
	programParams->setFloatParameter(s_handleLightCount, (float)m_lightCount);
	programParams->setStructBufferParameter(s_handleLights, m_lightSBuffer);
}

void WorldRenderPassForward::setFogProgramParameters(render::ProgramParameters* programParams) const
{
	if (m_fogEnabled)
	{
		programParams->setVectorParameter(s_handleFogDistanceAndDensity, Vector4(m_fogDistanceY, m_fogDistanceZ, m_fogDensityY, m_fogDensityZ));
		programParams->setVectorParameter(s_handleFogColor, m_fogColor);
	}
}

void WorldRenderPassForward::setColorMapProgramParameters(render::ProgramParameters* programParams) const
{
	if (m_colorMap)
		programParams->setTextureParameter(s_handleColorMap, m_colorMap);
}

void WorldRenderPassForward::setShadowMapProgramParameters(render::ProgramParameters* programParams) const
{
	if (m_shadowMask)
		programParams->setTextureParameter(s_handleShadowMask, m_shadowMask);
}

void WorldRenderPassForward::setDepthMapProgramParameters(render::ProgramParameters* programParams) const
{
	if (m_depthMap)
		programParams->setTextureParameter(s_handleDepthMap, m_depthMap);
}

	}
}
