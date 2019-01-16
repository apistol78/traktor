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

enum { MaxForwardLightCount = 2 };

bool s_handlesInitialized = false;
render::handle_t s_techniqueDefault;
render::handle_t s_handleView;
render::handle_t s_handleViewInverse;
render::handle_t s_handleWorld;
render::handle_t s_handleWorldView;
render::handle_t s_handleColorMap;
render::handle_t s_handleLightAmbientColor;
render::handle_t s_handleLightPositionAndType;
render::handle_t s_handleLightDirectionAndRange;
render::handle_t s_handleLightColor;
render::handle_t s_handleFogEnable;
render::handle_t s_handleFogDistanceAndDensity;
render::handle_t s_handleFogColor;
render::handle_t s_handleShadowEnable;
render::handle_t s_handleShadowMask;
render::handle_t s_handleDepthEnable;
render::handle_t s_handleDepthMap;

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
	s_handleLightAmbientColor = render::getParameterHandle(L"World_LightAmbientColor");
	s_handleLightPositionAndType = render::getParameterHandle(L"World_LightPositionAndType");
	s_handleLightDirectionAndRange = render::getParameterHandle(L"World_LightDirectionAndRange");
	s_handleLightColor = render::getParameterHandle(L"World_LightColor");
	s_handleFogEnable = render::getParameterHandle(L"World_FogEnable");
	s_handleFogDistanceAndDensity = render::getParameterHandle(L"World_FogDistanceAndDensity");
	s_handleFogColor = render::getParameterHandle(L"World_FogColor");
	s_handleShadowEnable = render::getParameterHandle(L"World_ShadowEnable");
	s_handleShadowMask = render::getParameterHandle(L"World_ShadowMask");
	s_handleDepthEnable = render::getParameterHandle(L"World_DepthEnable");
	s_handleDepthMap = render::getParameterHandle(L"World_DepthMap");

	s_handlesInitialized = true;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.WorldRenderPassForward", WorldRenderPassForward, IWorldRenderPass)

WorldRenderPassForward::WorldRenderPassForward(
	render::handle_t technique,
	const WorldRenderView& worldRenderView,
	uint32_t passFlags,
	const Vector4& ambientColor,
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
,	m_ambientColor(ambientColor)
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
,	m_ambientColor(0.0f, 0.0f, 0.0f, 0.0f)
,	m_fogEnabled(false)
,	m_fogDistanceY(0.0f)
,	m_fogDistanceZ(0.0f)
,	m_fogDensityY(0.0f)
,	m_fogDensityZ(0.0f)
,	m_fogColor(0.0f, 0.0f, 0.0f, 0.0f)
,	m_colorMap(colorMap)
,	m_depthMap(depthMap)
,	m_shadowMask(0)
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

void WorldRenderPassForward::setShaderCombination(render::Shader* shader, const Transform& world, const Aabb3& bounds) const
{
	if (m_technique == s_techniqueDefault)
	{
		int lightDirectionalCount = 0;
		int lightPointCount = 0;

		Scalar radius = bounds.empty() ? Scalar(0.0f) : bounds.getExtent().length();

		// Collect lights affecting entity.
		for (int i = 0; i < m_worldRenderView.getLightCount(); ++i)
		{
			const Light& light = m_worldRenderView.getLight(i);
			if (light.type == LtDirectional)
			{
				lightDirectionalCount++;
			}
			else if (light.type == LtPoint)
			{
				Scalar distance = (world.translation() - light.position).length();
				if (distance - radius <= light.range)
					lightPointCount++;
			}
		}

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
		setLightProgramParameters(programParams, world, bounds);
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
	const Matrix44& view = m_worldRenderView.getView();

	// Pack light parameters.
	Vector4 lightPositionAndType[MaxForwardLightCount], *lightPositionAndTypePtr = lightPositionAndType;
	Vector4 lightDirectionAndRange[MaxForwardLightCount], *lightDirectionAndRangePtr = lightDirectionAndRange;
	Vector4 lightColor[MaxForwardLightCount], *lightColorPtr = lightColor;

	int lightCount = std::min< int >(m_worldRenderView.getLightCount(), MaxForwardLightCount);
	for (int i = 0; i < lightCount; ++i)
	{
		const Light& light = m_worldRenderView.getLight(i);
		*lightPositionAndTypePtr++ = (view * light.position).xyz0() + Vector4(0.0f, 0.0f, 0.0f, float(light.type));
		*lightDirectionAndRangePtr++ = (view * light.direction).xyz0() + Vector4(0.0f, 0.0f, 0.0f, light.range);
		*lightColorPtr++ = light.color;
	}

	// Disable excessive lights.
	for (int i = lightCount; i < MaxForwardLightCount; ++i)
	{
		const static Vector4 c_typeDisabled(0.0f, 0.0f, 0.0f, float(LtDisabled));
		*lightPositionAndTypePtr++ = c_typeDisabled;
		*lightDirectionAndRangePtr++ = Vector4::zero();
		*lightColorPtr++ = Vector4::zero();
	}

	// Finally set shader parameters.
	programParams->setVectorParameter(s_handleLightAmbientColor, m_ambientColor);
	programParams->setVectorArrayParameter(s_handleLightPositionAndType, lightPositionAndType, MaxForwardLightCount);
	programParams->setVectorArrayParameter(s_handleLightDirectionAndRange, lightDirectionAndRange, MaxForwardLightCount);
	programParams->setVectorArrayParameter(s_handleLightColor, lightColor, MaxForwardLightCount);
}

void WorldRenderPassForward::setLightProgramParameters(render::ProgramParameters* programParams, const Transform& world, const Aabb3& bounds) const
{
	const Light* lightDirectional[MaxForwardLightCount]; int lightDirectionalCount = 0;
	const Light* lightPoint[MaxForwardLightCount]; int lightPointCount = 0;

	Scalar radius = bounds.empty() ? Scalar(0.0f) : bounds.getExtent().length();

	// Collect lights affecting entity.
	int lightCount = std::min< int >(m_worldRenderView.getLightCount(), MaxForwardLightCount);
	for (int i = 0; i < lightCount; ++i)
	{
		const Light& light = m_worldRenderView.getLight(i);
		if (light.type == LtDirectional)
		{
			lightDirectional[lightDirectionalCount++] = &light;
		}
		else if (light.type == LtPoint)
		{
			Scalar distance = (world.translation() - light.position).length();
			if (distance - radius <= light.range)
				lightPoint[lightPointCount++] = &light;
		}
	}

	const Matrix44& view = m_worldRenderView.getView();

	// Pack light parameters.
	Vector4 lightPositionAndType[MaxForwardLightCount], *lightPositionAndTypePtr = lightPositionAndType;
	Vector4 lightDirectionAndRange[MaxForwardLightCount], *lightDirectionAndRangePtr = lightDirectionAndRange;
	Vector4 lightColor[MaxForwardLightCount], *lightColorPtr = lightColor;

	for (int i = 0; i < lightDirectionalCount; ++i)
	{
		const static Vector4 c_typeDirectional(0.0f, 0.0f, 0.0f, float(LtDirectional));
		*lightPositionAndTypePtr++ = c_typeDirectional;
		*lightDirectionAndRangePtr++ = (view * lightDirectional[i]->direction).xyz0();
		*lightColorPtr++ = lightDirectional[i]->color;
	}
	for (int i = 0; i < lightPointCount; ++i)
	{
		const static Vector4 c_typePoint(0.0f, 0.0f, 0.0f, float(LtPoint));
		*lightPositionAndTypePtr++ = (view * lightPoint[i]->position).xyz0() + c_typePoint;
		*lightDirectionAndRangePtr++ = Vector4(0.0f, 0.0f, 0.0f, lightPoint[i]->range);
		*lightColorPtr++ = lightPoint[i]->color;
	}

	// Disable excessive lights.
	for (int i = lightDirectionalCount + lightPointCount; i < MaxForwardLightCount; ++i)
	{
		const static Vector4 c_typeDisabled(0.0f, 0.0f, 0.0f, float(LtDisabled));
		*lightPositionAndTypePtr++ = c_typeDisabled;
		*lightDirectionAndRangePtr++ = Vector4::zero();
		*lightColorPtr++ = Vector4::zero();
	}

	// Finally set shader parameters.
	programParams->setVectorParameter(s_handleLightAmbientColor, m_ambientColor);
	programParams->setVectorArrayParameter(s_handleLightPositionAndType, lightPositionAndType, MaxForwardLightCount);
	programParams->setVectorArrayParameter(s_handleLightDirectionAndRange, lightDirectionAndRange, MaxForwardLightCount);
	programParams->setVectorArrayParameter(s_handleLightColor, lightColor, MaxForwardLightCount);
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
