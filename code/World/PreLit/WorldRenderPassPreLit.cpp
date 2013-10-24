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

enum { MaxForwardLightCount = 2 };

bool s_handlesInitialized = false;
render::handle_t s_techniquePreLitColor;
render::handle_t s_handleProjection;
render::handle_t s_handleSquareProjection;
render::handle_t s_handleView;
render::handle_t s_handleWorld;
render::handle_t s_handleColorMap;
render::handle_t s_handleLightMap;
render::handle_t s_handleNormalMap;
render::handle_t s_handleFogEnable;
render::handle_t s_handleFogDistanceAndRange;
render::handle_t s_handleFogColor;
render::handle_t s_handleShadowEnable;
render::handle_t s_handleShadowMask;
render::handle_t s_handleDepthEnable;
render::handle_t s_handleDepthMap;
render::handle_t s_handleTime;
render::handle_t s_handleLightPositionAndType;
render::handle_t s_handleLightDirectionAndRange;
render::handle_t s_handleLightSunColor;
render::handle_t s_handleLightBaseColor;
render::handle_t s_handleLightShadowColor;

void initializeHandles()
{
	if (s_handlesInitialized)
		return;

	s_techniquePreLitColor = render::getParameterHandle(L"World_PreLitColor");

	s_handleProjection = render::getParameterHandle(L"Projection");
	s_handleSquareProjection = render::getParameterHandle(L"SquareProjection");
	s_handleView = render::getParameterHandle(L"View");
	s_handleWorld = render::getParameterHandle(L"World");
	s_handleColorMap = render::getParameterHandle(L"ColorMap");
	s_handleLightMap = render::getParameterHandle(L"LightMap");
	s_handleNormalMap = render::getParameterHandle(L"NormalMap");
	s_handleFogEnable = render::getParameterHandle(L"FogEnable");
	s_handleFogDistanceAndRange = render::getParameterHandle(L"FogDistanceAndRange");
	s_handleFogColor = render::getParameterHandle(L"FogColor");
	s_handleShadowEnable = render::getParameterHandle(L"ShadowEnable");
	s_handleShadowMask = render::getParameterHandle(L"ShadowMask");
	s_handleDepthEnable = render::getParameterHandle(L"DepthEnable");
	s_handleDepthMap = render::getParameterHandle(L"DepthMap");
	s_handleTime = render::getParameterHandle(L"Time");
	s_handleLightPositionAndType = render::getParameterHandle(L"LightPositionAndType");
	s_handleLightDirectionAndRange = render::getParameterHandle(L"LightDirectionAndRange");
	s_handleLightSunColor = render::getParameterHandle(L"LightSunColor");
	s_handleLightBaseColor = render::getParameterHandle(L"LightBaseColor");
	s_handleLightShadowColor = render::getParameterHandle(L"LightShadowColor");

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
	render::ISimpleTexture* colorMap,
	render::ISimpleTexture* depthMap,
	render::ISimpleTexture* normalMap,
	render::ISimpleTexture* lightMap
)
:	m_technique(technique)
,	m_worldRenderView(worldRenderView)
,	m_fogEnabled(fogEnabled)
,	m_fogDistance(fogDistance)
,	m_fogRange(fogRange)
,	m_fogColor(fogColor)
,	m_colorMap(colorMap)
,	m_depthMap(depthMap)
,	m_normalMap(normalMap)
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
,	m_colorMap(0)
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

void WorldRenderPassPreLit::setProgramParameters(render::ProgramParameters* programParams, uint32_t priority) const
{
	setWorldProgramParameters(programParams, Matrix44::identity());

	if (m_technique == s_techniquePreLitColor)
	{
		setColorMapProgramParameters(programParams);
		setDepthMapProgramParameters(programParams);
		setLightMapProgramParameters(programParams);
		setNormalMapProgramParameters(programParams);
		setFogProgramParameters(programParams);

		if ((priority & (render::RpAlphaBlend | render::RpPostAlphaBlend)) != 0)
			setLightProgramParameters(programParams);
	}
}

void WorldRenderPassPreLit::setProgramParameters(render::ProgramParameters* programParams, uint32_t priority, const Matrix44& world, const Aabb3& bounds) const
{
	setWorldProgramParameters(programParams, world);

	if (m_technique == s_techniquePreLitColor)
	{
		setColorMapProgramParameters(programParams);
		setDepthMapProgramParameters(programParams);
		setLightMapProgramParameters(programParams);
		setNormalMapProgramParameters(programParams);
		setFogProgramParameters(programParams);

		if ((priority & (render::RpAlphaBlend | render::RpPostAlphaBlend)) != 0)
			setLightProgramParameters(programParams);
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

void WorldRenderPassPreLit::setLightProgramParameters(render::ProgramParameters* programParams) const
{
	const Matrix44& view = m_worldRenderView.getView();

	// Pack light parameters.
	Vector4 lightPositionAndType[MaxForwardLightCount], *lightPositionAndTypePtr = lightPositionAndType;
	Vector4 lightDirectionAndRange[MaxForwardLightCount], *lightDirectionAndRangePtr = lightDirectionAndRange;
	Vector4 lightSunColor[MaxForwardLightCount], *lightSunColorPtr = lightSunColor;
	Vector4 lightBaseColor[MaxForwardLightCount], *lightBaseColorPtr = lightBaseColor;
	Vector4 lightShadowColor[MaxForwardLightCount], *lightShadowColorPtr = lightShadowColor;

	int lightCount = std::min< int >(m_worldRenderView.getLightCount(), MaxForwardLightCount);
	for (int i = 0; i < lightCount; ++i)
	{
		const Light& light = m_worldRenderView.getLight(i);
		*lightPositionAndTypePtr++ = (view * light.position).xyz0() + Vector4(0.0f, 0.0f, 0.0f, float(light.type));
		*lightDirectionAndRangePtr++ = (view * light.direction).xyz0() + Vector4(0.0f, 0.0f, 0.0f, light.range);
		*lightSunColorPtr++ = light.sunColor;
		*lightBaseColorPtr++ = light.baseColor;
		*lightShadowColorPtr++ = light.shadowColor;
	}

	// Disable excessive lights.
	for (int i = lightCount; i < MaxForwardLightCount; ++i)
	{
		const static Vector4 c_typeDisabled(0.0f, 0.0f, 0.0f, float(LtDisabled));
		*lightPositionAndTypePtr++ = c_typeDisabled;
		*lightDirectionAndRangePtr++ = Vector4::zero();
		*lightSunColorPtr++ = Vector4::zero();
		*lightBaseColorPtr++ = Vector4::zero();
		*lightShadowColorPtr++ = Vector4::zero();
	}

	// Finally set shader parameters.
	programParams->setVectorArrayParameter(s_handleLightPositionAndType, lightPositionAndType, MaxForwardLightCount);
	programParams->setVectorArrayParameter(s_handleLightDirectionAndRange, lightDirectionAndRange, MaxForwardLightCount);
	programParams->setVectorArrayParameter(s_handleLightSunColor, lightSunColor, MaxForwardLightCount);
	programParams->setVectorArrayParameter(s_handleLightBaseColor, lightBaseColor, MaxForwardLightCount);
	programParams->setVectorArrayParameter(s_handleLightShadowColor, lightShadowColor, MaxForwardLightCount);
}

void WorldRenderPassPreLit::setColorMapProgramParameters(render::ProgramParameters* programParams) const
{
	if (m_colorMap)
		programParams->setTextureParameter(s_handleColorMap, m_colorMap);
}

void WorldRenderPassPreLit::setShadowMapProgramParameters(render::ProgramParameters* programParams) const
{
	if (m_shadowMask)
		programParams->setTextureParameter(s_handleShadowMask, m_shadowMask);
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

void WorldRenderPassPreLit::setNormalMapProgramParameters(render::ProgramParameters* programParams) const
{
	if (m_normalMap)
		programParams->setTextureParameter(s_handleNormalMap, m_normalMap);
}

void WorldRenderPassPreLit::setFogProgramParameters(render::ProgramParameters* programParams) const
{
	if (m_fogEnabled)
	{
		programParams->setVectorParameter(s_handleFogDistanceAndRange, Vector4(m_fogDistance, m_fogRange, 1.0f / m_fogDistance, 1.0f / m_fogRange));
		programParams->setVectorParameter(s_handleFogColor, m_fogColor);
	}
	else
		programParams->setVectorParameter(s_handleFogDistanceAndRange, Vector4::zero());
}

	}
}
