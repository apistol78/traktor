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
render::handle_t s_handleShadowCascade;
render::handle_t s_handleShadowAtlas;
render::handle_t s_handleDepthMap;
render::handle_t s_handleOcclusionMap;
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
	s_handleShadowCascade = render::getParameterHandle(L"World_ShadowCascade");
	s_handleShadowAtlas = render::getParameterHandle(L"World_ShadowAtlas");
	s_handleDepthMap = render::getParameterHandle(L"World_DepthMap");
	s_handleOcclusionMap = render::getParameterHandle(L"World_OcclusionMap");
	s_handleLightCount = render::getParameterHandle(L"World_LightCount");
	s_handleLights = render::getParameterHandle(L"World_Lights");

	s_handlesInitialized = true;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.WorldRenderPassForward", WorldRenderPassForward, IWorldRenderPass)

WorldRenderPassForward::WorldRenderPassForward(
	render::handle_t technique,
	uint32_t passFlags,
	const Matrix44& view,
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
	render::ISimpleTexture* occlusionMap,
	render::ISimpleTexture* shadowCascade,
	render::ISimpleTexture* shadowAtlas
)
:	m_technique(technique)
,	m_passFlags(passFlags)
,	m_view(view)
,	m_viewInverse(view.inverse())
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
,	m_occlusionMap(occlusionMap)
,	m_shadowCascade(shadowCascade)
,	m_shadowAtlas(shadowAtlas)
{
	initializeHandles();
}

WorldRenderPassForward::WorldRenderPassForward(
	render::handle_t technique,
	uint32_t passFlags,
	const Matrix44& view,
	render::ISimpleTexture* colorMap,
	render::ISimpleTexture* depthMap,
	render::ISimpleTexture* occlusionMap
)
:	m_technique(technique)
,	m_passFlags(passFlags)
,	m_view(view)
,	m_viewInverse(view.inverse())
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
,	m_occlusionMap(occlusionMap)
,	m_shadowCascade(nullptr)
{
	initializeHandles();
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
		shader->setCombination(s_handleShadowEnable, m_shadowCascade != nullptr && m_shadowAtlas != nullptr);
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
		setOcclusionMapProgramParameters(programParams);
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
		setOcclusionMapProgramParameters(programParams);
	}
}

void WorldRenderPassForward::setWorldProgramParameters(render::ProgramParameters* programParams, const Transform& world) const
{
	Matrix44 w = world.toMatrix44();
	programParams->setMatrixParameter(s_handleView, m_view);
	programParams->setMatrixParameter(s_handleViewInverse, m_viewInverse);
	programParams->setMatrixParameter(s_handleWorld, w);
	programParams->setMatrixParameter(s_handleWorldView, m_view * w);
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
	if (m_shadowCascade)
		programParams->setTextureParameter(s_handleShadowCascade, m_shadowCascade);
	if (m_shadowAtlas)
		programParams->setTextureParameter(s_handleShadowAtlas, m_shadowAtlas);
}

void WorldRenderPassForward::setDepthMapProgramParameters(render::ProgramParameters* programParams) const
{
	if (m_depthMap)
		programParams->setTextureParameter(s_handleDepthMap, m_depthMap);
}

void WorldRenderPassForward::setOcclusionMapProgramParameters(render::ProgramParameters* programParams) const
{
	if (m_occlusionMap)
		programParams->setTextureParameter(s_handleOcclusionMap, m_occlusionMap);
}

	}
}
