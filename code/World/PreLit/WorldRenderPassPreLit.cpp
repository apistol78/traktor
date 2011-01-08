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
render::handle_t s_handleProjection;
render::handle_t s_handleView;
render::handle_t s_handleWorld;
render::handle_t s_handleEyePosition;
render::handle_t s_handleLightPositionAndType;
render::handle_t s_handleLightDirectionAndRange;
render::handle_t s_handleLightSunColor;
render::handle_t s_handleLightBaseColor;
render::handle_t s_handleLightShadowColor;
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
:	m_worldRenderView(worldRenderView)
,	m_depthRange(depthRange)
,	m_depthMap(depthMap)
,	m_shadowMask(shadowMask)
{
	if (!s_handlesInitialized)
	{
		// Techniques
		s_handleDefaultTechnique = render::getParameterHandle(L"Default");

		// Parameters
		s_handleProjection = render::getParameterHandle(L"Projection");
		s_handleView = render::getParameterHandle(L"View");
		s_handleWorld = render::getParameterHandle(L"World");
		s_handleEyePosition = render::getParameterHandle(L"EyePosition");
		s_handleLightPositionAndType = render::getParameterHandle(L"LightPositionAndType");
		s_handleLightDirectionAndRange = render::getParameterHandle(L"LightDirectionAndRange");
		s_handleLightSunColor = render::getParameterHandle(L"LightSunColor");
		s_handleLightBaseColor = render::getParameterHandle(L"LightBaseColor");
		s_handleLightShadowColor = render::getParameterHandle(L"LightShadowColor");
		s_handleShadowEnable = render::getParameterHandle(L"ShadowEnable");
		s_handleShadowMask = render::getParameterHandle(L"ShadowMask");
		s_handleShadowMaskSize = render::getParameterHandle(L"ShadowMaskSize");
		s_handleDepthEnable = render::getParameterHandle(L"DepthEnable");
		s_handleDepthRange = render::getParameterHandle(L"DepthRange");
		s_handleDepthMap = render::getParameterHandle(L"DepthMap");
		s_handleTime = render::getParameterHandle(L"Time");
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
	if (m_technique == s_handleDefaultTechnique)
		shader->setCombination(s_handleDepthEnable, m_depthMap != 0);
}

void WorldRenderPassPreLit::setShaderCombination(render::Shader* shader, const Matrix44& world, const Aabb& bounds) const
{
	if (m_technique == s_handleDefaultTechnique)
		shader->setCombination(s_handleDepthEnable, m_depthMap != 0);
}

void WorldRenderPassPreLit::setProgramParameters(render::ProgramParameters* programParams) const
{
	setWorldProgramParameters(programParams, Matrix44::identity());

	// Set these parameters only if we're rendering using default technique.
	if (m_technique == s_handleDefaultTechnique)
	{
		setLightProgramParameters(programParams);
		setShadowMapProgramParameters(programParams);
		setDepthMapProgramParameters(programParams);
	}
}

void WorldRenderPassPreLit::setProgramParameters(render::ProgramParameters* programParams, const Matrix44& world, const Aabb& bounds) const
{
	setWorldProgramParameters(programParams, world);

	// Set these parameters only if we're rendering using default technique.
	if (m_technique == s_handleDefaultTechnique)
	{
		setLightProgramParameters(programParams, world, bounds);
		setShadowMapProgramParameters(programParams);
		setDepthMapProgramParameters(programParams);
	}
}

void WorldRenderPassPreLit::setWorldProgramParameters(render::ProgramParameters* programParams, const Matrix44& world) const
{
	programParams->setFloatParameter(s_handleTime, m_worldRenderView.getTime());
	programParams->setMatrixParameter(s_handleProjection, m_worldRenderView.getProjection());
	programParams->setMatrixParameter(s_handleView, m_worldRenderView.getView());
	programParams->setMatrixParameter(s_handleWorld, world);
	programParams->setVectorParameter(s_handleEyePosition, m_worldRenderView.getEyePosition());
	programParams->setFloatParameter(s_handleDepthRange, m_depthRange);
}

void WorldRenderPassPreLit::setLightProgramParameters(render::ProgramParameters* programParams) const
{
	// Pack light parameters.
	Vector4 lightPositionAndType[WorldRenderView::MaxLightCount], *lightPositionAndTypePtr = lightPositionAndType;
	Vector4 lightDirectionAndRange[WorldRenderView::MaxLightCount], *lightDirectionAndRangePtr = lightDirectionAndRange;
	Vector4 lightSunColor[WorldRenderView::MaxLightCount], *lightSunColorPtr = lightSunColor;
	Vector4 lightBaseColor[WorldRenderView::MaxLightCount], *lightBaseColorPtr = lightBaseColor;
	Vector4 lightShadowColor[WorldRenderView::MaxLightCount], *lightShadowColorPtr = lightShadowColor;

	for (int i = 0; i < m_worldRenderView.getLightCount(); ++i)
	{
		const WorldRenderView::Light& light = m_worldRenderView.getLight(i);
		*lightPositionAndTypePtr++ = light.position.xyz0() + Vector4(0.0f, 0.0f, 0.0f, float(light.type));
		*lightDirectionAndRangePtr++ = light.direction.xyz0() + Vector4(0.0f, 0.0f, 0.0f, light.range);
		*lightSunColorPtr++ = light.sunColor;
		*lightBaseColorPtr++ = light.baseColor;
		*lightShadowColorPtr++ = light.shadowColor;
	}

	// Disable excessive lights.
	for (int i = m_worldRenderView.getLightCount(); i < WorldRenderView::MaxLightCount; ++i)
	{
		const static Vector4 c_typeDisabled(0.0f, 0.0f, 0.0f, float(WorldRenderView::LtDisabled));
		*lightPositionAndTypePtr++ = c_typeDisabled;
		*lightDirectionAndRangePtr++ = Vector4::zero();
		*lightSunColorPtr++ = Vector4::zero();
		*lightBaseColorPtr++ = Vector4::zero();
		*lightShadowColorPtr++ = Vector4::zero();
	}

	// Finally set shader parameters.
	programParams->setVectorArrayParameter(s_handleLightPositionAndType, lightPositionAndType, WorldRenderView::MaxLightCount);
	programParams->setVectorArrayParameter(s_handleLightDirectionAndRange, lightDirectionAndRange, WorldRenderView::MaxLightCount);
	programParams->setVectorArrayParameter(s_handleLightSunColor, lightSunColor, WorldRenderView::MaxLightCount);
	programParams->setVectorArrayParameter(s_handleLightBaseColor, lightBaseColor, WorldRenderView::MaxLightCount);
	programParams->setVectorArrayParameter(s_handleLightShadowColor, lightShadowColor, WorldRenderView::MaxLightCount);
}

void WorldRenderPassPreLit::setLightProgramParameters(render::ProgramParameters* programParams, const Matrix44& world, const Aabb& bounds) const
{
	const WorldRenderView::Light* lightDirectional[WorldRenderView::MaxLightCount]; int lightDirectionalCount = 0;
	const WorldRenderView::Light* lightPoint[WorldRenderView::MaxLightCount]; int lightPointCount = 0;

	Scalar radius = bounds.empty() ? Scalar(0.0f) : bounds.getExtent().length();

	// Collect lights affecting entity.
	for (int i = 0; i < m_worldRenderView.getLightCount(); ++i)
	{
		const WorldRenderView::Light& light = m_worldRenderView.getLight(i);
		if (light.type == WorldRenderView::LtDirectional)
		{
			lightDirectional[lightDirectionalCount++] = &light;
		}
		else if (light.type == WorldRenderView::LtPoint)
		{
			Scalar distance = (world.translation() - light.position).length();
			if (distance - radius <= light.range)
				lightPoint[lightPointCount++] = &light;
		}
	}

	// Pack light parameters.
	Vector4 lightPositionAndType[WorldRenderView::MaxLightCount], *lightPositionAndTypePtr = lightPositionAndType;
	Vector4 lightDirectionAndRange[WorldRenderView::MaxLightCount], *lightDirectionAndRangePtr = lightDirectionAndRange;
	Vector4 lightSunColor[WorldRenderView::MaxLightCount], *lightSunColorPtr = lightSunColor;
	Vector4 lightBaseColor[WorldRenderView::MaxLightCount], *lightBaseColorPtr = lightBaseColor;
	Vector4 lightShadowColor[WorldRenderView::MaxLightCount], *lightShadowColorPtr = lightShadowColor;

	for (int i = 0; i < lightDirectionalCount; ++i)
	{
		const static Vector4 c_typeDirectional(0.0f, 0.0f, 0.0f, float(WorldRenderView::LtDirectional));
		*lightPositionAndTypePtr++ = c_typeDirectional;
		*lightDirectionAndRangePtr++ = lightDirectional[i]->direction.xyz0();
		*lightSunColorPtr++ = lightDirectional[i]->sunColor;
		*lightBaseColorPtr++ = lightDirectional[i]->baseColor;
		*lightShadowColorPtr++ = lightDirectional[i]->shadowColor;
	}
	for (int i = 0; i < lightPointCount; ++i)
	{
		const static Vector4 c_typePoint(0.0f, 0.0f, 0.0f, float(WorldRenderView::LtPoint));
		*lightPositionAndTypePtr++ = lightPoint[i]->position.xyz0() + c_typePoint;
		*lightDirectionAndRangePtr++ = Vector4(0.0f, 0.0f, 0.0f, lightPoint[i]->range);
		*lightSunColorPtr++ = lightPoint[i]->sunColor;
		*lightBaseColorPtr++ = lightPoint[i]->baseColor;
		*lightShadowColorPtr++ = lightPoint[i]->shadowColor;
	}

	// Disable excessive lights.
	for (int i = lightDirectionalCount + lightPointCount; i < WorldRenderView::MaxLightCount; ++i)
	{
		const static Vector4 c_typeDisabled(0.0f, 0.0f, 0.0f, float(WorldRenderView::LtDisabled));
		*lightPositionAndTypePtr++ = c_typeDisabled;
		*lightDirectionAndRangePtr++ = Vector4::zero();
		*lightSunColorPtr++ = Vector4::zero();
		*lightBaseColorPtr++ = Vector4::zero();
		*lightShadowColorPtr++ = Vector4::zero();
	}

	// Finally set shader parameters.
	programParams->setVectorArrayParameter(s_handleLightPositionAndType, lightPositionAndType, WorldRenderView::MaxLightCount);
	programParams->setVectorArrayParameter(s_handleLightDirectionAndRange, lightDirectionAndRange, WorldRenderView::MaxLightCount);
	programParams->setVectorArrayParameter(s_handleLightSunColor, lightSunColor, WorldRenderView::MaxLightCount);
	programParams->setVectorArrayParameter(s_handleLightBaseColor, lightBaseColor, WorldRenderView::MaxLightCount);
	programParams->setVectorArrayParameter(s_handleLightShadowColor, lightShadowColor, WorldRenderView::MaxLightCount);
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

	}
}
