#include "Render/ITexture.h"
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
render::handle_t s_handleDefaultTechnique;
render::handle_t s_handleLightTechnique;
render::handle_t s_handleProjection;
render::handle_t s_handleSquareProjection;
render::handle_t s_handleView;
render::handle_t s_handleWorld;
render::handle_t s_handleEyePosition;
render::handle_t s_handleLightPositionAndType;
render::handle_t s_handleLightDirectionAndRange;
render::handle_t s_handleLightSunColor;
render::handle_t s_handleLightBaseColor;
render::handle_t s_handleLightShadowColor;
render::handle_t s_handleLightMap;
render::handle_t s_handleShadowEnable;
render::handle_t s_handleShadowMask;
render::handle_t s_handleShadowMaskSize;
render::handle_t s_handleDepthEnable;
render::handle_t s_handleDepthRange;
render::handle_t s_handleDepthMap;
render::handle_t s_handleTime;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.WorldRenderPassPreLit", WorldRenderPassPreLit, IWorldRenderPass)

WorldRenderPassPreLit::WorldRenderPassPreLit(
	render::handle_t technique,
	const WorldRenderView& worldRenderView,
	float depthRange,
	render::ITexture* depthMap,
	render::ITexture* shadowMask,
	render::ITexture* lightMap
)
:	m_technique(technique)
,	m_worldRenderView(worldRenderView)
,	m_depthRange(depthRange)
,	m_depthMap(depthMap)
,	m_shadowMask(shadowMask)
,	m_lightMap(lightMap)
{
	if (!s_handlesInitialized)
	{
		// Techniques
		s_handleDefaultTechnique = render::getParameterHandle(L"Default");
		s_handleLightTechnique = render::getParameterHandle(L"Light");

		// Parameters
		s_handleProjection = render::getParameterHandle(L"Projection");
		s_handleSquareProjection = render::getParameterHandle(L"SquareProjection");
		s_handleView = render::getParameterHandle(L"View");
		s_handleWorld = render::getParameterHandle(L"World");
		s_handleEyePosition = render::getParameterHandle(L"EyePosition");
		s_handleLightPositionAndType = render::getParameterHandle(L"LightPositionAndType");
		s_handleLightDirectionAndRange = render::getParameterHandle(L"LightDirectionAndRange");
		s_handleLightSunColor = render::getParameterHandle(L"LightSunColor");
		s_handleLightBaseColor = render::getParameterHandle(L"LightBaseColor");
		s_handleLightShadowColor = render::getParameterHandle(L"LightShadowColor");
		s_handleLightMap = render::getParameterHandle(L"LightMap");
		s_handleShadowEnable = render::getParameterHandle(L"ShadowEnable");
		s_handleShadowMask = render::getParameterHandle(L"ShadowMask");
		s_handleShadowMaskSize = render::getParameterHandle(L"ShadowMaskSize");
		s_handleDepthEnable = render::getParameterHandle(L"DepthEnable");
		s_handleDepthRange = render::getParameterHandle(L"DepthRange");
		s_handleDepthMap = render::getParameterHandle(L"DepthMap");
		s_handleTime = render::getParameterHandle(L"Time");

		s_handlesInitialized = true;
	}
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
	if (m_technique == s_handleLightTechnique)
		shader->setCombination(s_handleShadowEnable, m_shadowMask != 0);
	if (m_technique == s_handleDefaultTechnique)
		shader->setCombination(s_handleDepthEnable, m_depthMap != 0);
}

void WorldRenderPassPreLit::setShaderCombination(render::Shader* shader, const Matrix44& world, const Aabb3& bounds) const
{
	if (m_technique == s_handleLightTechnique)
		shader->setCombination(s_handleShadowEnable, m_shadowMask != 0);
	if (m_technique == s_handleDefaultTechnique)
		shader->setCombination(s_handleDepthEnable, m_depthMap != 0);
}

void WorldRenderPassPreLit::setProgramParameters(render::ProgramParameters* programParams) const
{
	setWorldProgramParameters(programParams, Matrix44::identity());

	if (m_technique == s_handleLightTechnique)
	{
		setLightProgramParameters(programParams);
		setShadowMapProgramParameters(programParams);
	}
	else if (m_technique == s_handleDefaultTechnique)
	{
		setDepthMapProgramParameters(programParams);
		setLightMapProgramParameters(programParams);
	}
}

void WorldRenderPassPreLit::setProgramParameters(render::ProgramParameters* programParams, const Matrix44& world, const Aabb3& bounds) const
{
	setWorldProgramParameters(programParams, world);

	if (m_technique == s_handleLightTechnique)
	{
		setLightProgramParameters(programParams);
		setShadowMapProgramParameters(programParams);
	}
	else if (m_technique == s_handleDefaultTechnique)
	{
		setDepthMapProgramParameters(programParams);
		setLightMapProgramParameters(programParams);
	}
}

void WorldRenderPassPreLit::setWorldProgramParameters(render::ProgramParameters* programParams, const Matrix44& world) const
{
	programParams->setFloatParameter(s_handleTime, m_worldRenderView.getTime());
	programParams->setMatrixParameter(s_handleProjection, m_worldRenderView.getProjection());
	programParams->setMatrixParameter(s_handleSquareProjection, m_worldRenderView.getSquareProjection());
	programParams->setMatrixParameter(s_handleView, m_worldRenderView.getView());
	programParams->setMatrixParameter(s_handleWorld, world);
	programParams->setVectorParameter(s_handleEyePosition, m_worldRenderView.getEyePosition());
	programParams->setFloatParameter(s_handleDepthRange, m_depthRange);
}

void WorldRenderPassPreLit::setLightProgramParameters(render::ProgramParameters* programParams) const
{
	const WorldRenderView::Light& light = m_worldRenderView.getLight(0);
	Vector4 lightPositionAndType = light.position.xyz0() + Vector4(0.0f, 0.0f, 0.0f, float(light.type));
	Vector4 lightDirectionAndRange = light.direction.xyz0() + Vector4(0.0f, 0.0f, 0.0f, light.range);
	Vector4 lightSunColor = light.sunColor;
	Vector4 lightBaseColor = light.baseColor;
	Vector4 lightShadowColor = light.shadowColor;

	programParams->setVectorParameter(s_handleLightPositionAndType, lightPositionAndType);
	programParams->setVectorParameter(s_handleLightDirectionAndRange, lightDirectionAndRange);
	programParams->setVectorParameter(s_handleLightSunColor, lightSunColor);
	programParams->setVectorParameter(s_handleLightBaseColor, lightBaseColor);
	programParams->setVectorParameter(s_handleLightShadowColor, lightShadowColor);
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

	}
}
