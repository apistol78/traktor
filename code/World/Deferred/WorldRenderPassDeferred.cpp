#include "Render/Shader.h"
#include "Render/Types.h"
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
render::handle_t s_handleWorld;
render::handle_t s_handleWorldView;
render::handle_t s_handleFogEnable;
render::handle_t s_handleDepthEnable;
render::handle_t s_handleLightPositionAndType;
render::handle_t s_handleLightDirectionAndRange;
render::handle_t s_handleLightSunColor;
render::handle_t s_handleLightBaseColor;
render::handle_t s_handleLightShadowColor;

void initializeHandles()
{
	if (s_handlesInitialized)
		return;

	s_techniqueDeferredColor = render::getParameterHandle(L"World_DeferredColor");

	s_handleWorld = render::getParameterHandle(L"World_World");
	s_handleWorldView = render::getParameterHandle(L"World_WorldView");
	s_handleFogEnable = render::getParameterHandle(L"World_FogEnable");
	s_handleDepthEnable = render::getParameterHandle(L"World_DepthEnable");
	s_handleLightPositionAndType = render::getParameterHandle(L"World_LightPositionAndType");
	s_handleLightDirectionAndRange = render::getParameterHandle(L"World_LightDirectionAndRange");
	s_handleLightSunColor = render::getParameterHandle(L"World_LightSunColor");
	s_handleLightBaseColor = render::getParameterHandle(L"World_LightBaseColor");
	s_handleLightShadowColor = render::getParameterHandle(L"World_LightShadowColor");

	s_handlesInitialized = true;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.WorldRenderPassDeferred", WorldRenderPassDeferred, IWorldRenderPass)

WorldRenderPassDeferred::WorldRenderPassDeferred(
	render::handle_t technique,
	const WorldRenderView& worldRenderView,
	bool fogEnabled,
	bool depthEnable
)
:	m_technique(technique)
,	m_worldRenderView(worldRenderView)
,	m_fogEnabled(fogEnabled)
,	m_depthEnable(depthEnable)
{
	initializeHandles();
}

WorldRenderPassDeferred::WorldRenderPassDeferred(
	render::handle_t technique,
	const WorldRenderView& worldRenderView
)
:	m_technique(technique)
,	m_worldRenderView(worldRenderView)
,	m_fogEnabled(false)
,	m_depthEnable(false)
{
	initializeHandles();
}

render::handle_t WorldRenderPassDeferred::getTechnique() const
{
	return m_technique;
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
}

void WorldRenderPassDeferred::setShaderCombination(render::Shader* shader, const Matrix44& world, const Aabb3& bounds) const
{
	if (m_technique == s_techniqueDeferredColor)
	{
		shader->setCombination(s_handleFogEnable, m_fogEnabled);
		shader->setCombination(s_handleDepthEnable, m_depthEnable);
	}
}

void WorldRenderPassDeferred::setProgramParameters(render::ProgramParameters* programParams, uint32_t priority) const
{
	setWorldProgramParameters(programParams, Matrix44::identity());

	if (m_technique == s_techniqueDeferredColor)
	{
		if ((priority & (render::RpAlphaBlend | render::RpPostAlphaBlend)) != 0)
			setLightProgramParameters(programParams);
	}
}

void WorldRenderPassDeferred::setProgramParameters(render::ProgramParameters* programParams, uint32_t priority, const Matrix44& world, const Aabb3& bounds) const
{
	setWorldProgramParameters(programParams, world);

	if (m_technique == s_techniqueDeferredColor)
	{
		if ((priority & (render::RpAlphaBlend | render::RpPostAlphaBlend)) != 0)
			setLightProgramParameters(programParams);
	}
}

void WorldRenderPassDeferred::setWorldProgramParameters(render::ProgramParameters* programParams, const Matrix44& world) const
{
	programParams->setMatrixParameter(s_handleWorld, world);
	programParams->setMatrixParameter(s_handleWorldView, m_worldRenderView.getView() * world);
}

void WorldRenderPassDeferred::setLightProgramParameters(render::ProgramParameters* programParams) const
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

	}
}
