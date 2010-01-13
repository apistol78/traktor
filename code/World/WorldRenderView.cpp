#include "World/WorldRenderView.h"
#include "Render/ITexture.h"
#include "Render/Context/ShaderParameters.h"

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
render::handle_t s_handleViewPrevious;
render::handle_t s_handleViewDistance;
render::handle_t s_handleViewSize;
render::handle_t s_handleWorld;
render::handle_t s_handleWorldPrevious;
render::handle_t s_handleEyePosition;
render::handle_t s_handleLightEnableComplex;
render::handle_t s_handleLightPositionAndType;
render::handle_t s_handleLightDirectionAndRange;
render::handle_t s_handleLightSunColor;
render::handle_t s_handleLightBaseColor;
render::handle_t s_handleLightShadowColor;
render::handle_t s_handleShadowEnable;
render::handle_t s_handleShadowMask;
render::handle_t s_handleDepthEnable;
render::handle_t s_handleDepthMap;
render::handle_t s_handleTime;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.WorldRenderView", WorldRenderView, Object)

WorldRenderView::WorldRenderView()
:	m_technique(render::getParameterHandle(L"Default"))
,	m_projection(Matrix44::identity())
,	m_view(Matrix44::identity())
,	m_viewPrevious(Matrix44::identity())
,	m_viewSize(0.0f, 0.0f)
,	m_eyePosition(0.0f, 0.0f, 0.0f, 1.0f)
,	m_lightCount(0)
,	m_shadowMapBias(0.0f)
,	m_time(0.0f)
{
	for (int i = 0; i < MaxLightCount; ++i)
	{
		m_lights[i].type = LtDisabled;
		m_lights[i].position = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
		m_lights[i].direction = Vector4(0.0f, 0.0f, 1.0f, 0.0f);
		m_lights[i].sunColor = Vector4(1.0f, 1.0f, 1.0f, 0.0f);
		m_lights[i].baseColor = Vector4(0.5f, 0.5f, 0.5f, 0.0f);
		m_lights[i].shadowColor = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
		m_lights[i].range = Scalar(0.0f);
	}

	if (!s_handlesInitialized)
	{
		s_handleDefaultTechnique = render::getParameterHandle(L"Default");
		s_handleProjection = render::getParameterHandle(L"Projection");
		s_handleView = render::getParameterHandle(L"View");
		s_handleViewPrevious = render::getParameterHandle(L"ViewPrevious");
		s_handleViewDistance = render::getParameterHandle(L"ViewDistance");
		s_handleViewSize = render::getParameterHandle(L"ViewSize");
		s_handleWorld = render::getParameterHandle(L"World");
		s_handleWorldPrevious = render::getParameterHandle(L"WorldPrevious");
		s_handleEyePosition = render::getParameterHandle(L"EyePosition");
		s_handleLightEnableComplex = render::getParameterHandle(L"LightEnableComplex");
		s_handleLightPositionAndType = render::getParameterHandle(L"LightPositionAndType");
		s_handleLightDirectionAndRange = render::getParameterHandle(L"LightDirectionAndRange");
		s_handleLightSunColor = render::getParameterHandle(L"LightSunColor");
		s_handleLightBaseColor = render::getParameterHandle(L"LightBaseColor");
		s_handleLightShadowColor = render::getParameterHandle(L"LightShadowColor");
		s_handleShadowEnable = render::getParameterHandle(L"ShadowEnable");
		s_handleShadowMask = render::getParameterHandle(L"ShadowMask");
		s_handleDepthEnable = render::getParameterHandle(L"DepthEnable");
		s_handleDepthMap = render::getParameterHandle(L"DepthMap");
		s_handleTime = render::getParameterHandle(L"Time");
	}
}

void WorldRenderView::setTechnique(render::handle_t technique)
{
	m_technique = technique;
}

void WorldRenderView::setViewFrustum(const Frustum& viewFrustum)
{
	m_viewFrustum = viewFrustum;
}

void WorldRenderView::setCullFrustum(const Frustum& cullFrustum)
{
	m_cullFrustum = cullFrustum;
}

void WorldRenderView::setProjection(const Matrix44& projection)
{
	m_projection = projection;
}

void WorldRenderView::setView(const Matrix44& view)
{
	m_viewPrevious = m_view;
	m_view = view;
}

void WorldRenderView::setViewSize(const Vector2& viewSize)
{
	m_viewSize = viewSize;
}

void WorldRenderView::setEyePosition(const Vector4& eyePosition)
{
	m_eyePosition = eyePosition;
}

void WorldRenderView::setShadowBox(const Aabb& shadowBox)
{
	m_shadowBox = shadowBox;
}

void WorldRenderView::setShadowMask(render::ITexture* shadowMask)
{
	m_shadowMask = shadowMask;
}

void WorldRenderView::setDepthMap(render::ITexture* depthMap)
{
	m_depthMap = depthMap;
}

void WorldRenderView::setTime(float time)
{
	m_time = time;
}

void WorldRenderView::addLight(const Light& light)
{
	if (m_lightCount < MaxLightCount)
		m_lights[m_lightCount++] = light;
}

void WorldRenderView::resetLights()
{
	m_lightCount = 0;
}

void WorldRenderView::setTechniqueShaderParameters(render::ShaderParameters* shaderParams) const
{
	shaderParams->setTechnique(m_technique);
	shaderParams->setFloatParameter(s_handleTime, m_time);
}

void WorldRenderView::setWorldShaderParameters(render::ShaderParameters* shaderParams, const Matrix44& world, const Matrix44& worldPrevious) const
{
	float viewSizeInvX = m_viewSize.x != 0.0f ? 1.0f / m_viewSize.x : 0.0f;
	float viewSizeInvY = m_viewSize.y != 0.0f ? 1.0f / m_viewSize.y : 0.0f;

	shaderParams->setMatrixParameter(s_handleProjection, m_projection);
	shaderParams->setMatrixParameter(s_handleView, m_view);
	shaderParams->setMatrixParameter(s_handleViewPrevious, m_viewPrevious);
	shaderParams->setMatrixParameter(s_handleWorld, world);
	shaderParams->setMatrixParameter(s_handleWorldPrevious, worldPrevious);
	shaderParams->setVectorParameter(s_handleViewSize, Vector4(m_viewSize.x, m_viewSize.y, viewSizeInvX, viewSizeInvY));
	shaderParams->setVectorParameter(s_handleEyePosition, m_eyePosition);

	Vector4 viewDistance(m_viewFrustum.getNearZ(), m_viewFrustum.getFarZ(), m_cullFrustum.getNearZ(), m_cullFrustum.getFarZ());
	shaderParams->setVectorParameter(s_handleViewDistance, viewDistance);
}

void WorldRenderView::setLightShaderParameters(render::ShaderParameters* shaderParams) const
{
	T_ASSERT (m_lightCount <= MaxLightCount);

	if (m_lightCount == 1 && m_lights[0].type == LtDirectional)
	{
		// Single directional light; enable simple lighting path.
		shaderParams->setBooleanParameter(s_handleLightEnableComplex, false);
	}
	else
	{
		// Enable complex lighting path with dynamic branching.
		shaderParams->setBooleanParameter(s_handleLightEnableComplex, true);
	}

	// Pack light parameters.
	Vector4 lightPositionAndType[MaxLightCount], *lightPositionAndTypePtr = lightPositionAndType;
	Vector4 lightDirectionAndRange[MaxLightCount], *lightDirectionAndRangePtr = lightDirectionAndRange;
	Vector4 lightSunColor[MaxLightCount], *lightSunColorPtr = lightSunColor;
	Vector4 lightBaseColor[MaxLightCount], *lightBaseColorPtr = lightBaseColor;
	Vector4 lightShadowColor[MaxLightCount], *lightShadowColorPtr = lightShadowColor;

	for (int i = 0; i < m_lightCount; ++i)
	{
		*lightPositionAndTypePtr++ = m_lights[i].position.xyz0() + Vector4(0.0f, 0.0f, 0.0f, float(m_lights[i].type));
		*lightDirectionAndRangePtr++ = m_lights[i].direction.xyz0() + Vector4(0.0f, 0.0f, 0.0f, m_lights[i].range);
		*lightSunColorPtr++ = m_lights[i].sunColor;
		*lightBaseColorPtr++ = m_lights[i].baseColor;
		*lightShadowColorPtr++ = m_lights[i].shadowColor;
	}

	// Disable excessive lights.
	for (int i = m_lightCount; i < MaxLightCount; ++i)
	{
		const static Vector4 c_typeDisabled(0.0f, 0.0f, 0.0f, float(LtDisabled));
		*lightPositionAndTypePtr++ = c_typeDisabled;
		*lightDirectionAndRangePtr++ = Vector4::zero();
		*lightSunColorPtr++ = Vector4::zero();
		*lightBaseColorPtr++ = Vector4::zero();
		*lightShadowColorPtr++ = Vector4::zero();
	}

	// Finally set shader parameters.
	shaderParams->setVectorArrayParameter(s_handleLightPositionAndType, lightPositionAndType, MaxLightCount);
	shaderParams->setVectorArrayParameter(s_handleLightDirectionAndRange, lightDirectionAndRange, MaxLightCount);
	shaderParams->setVectorArrayParameter(s_handleLightSunColor, lightSunColor, MaxLightCount);
	shaderParams->setVectorArrayParameter(s_handleLightBaseColor, lightBaseColor, MaxLightCount);
	shaderParams->setVectorArrayParameter(s_handleLightShadowColor, lightShadowColor, MaxLightCount);
}

void WorldRenderView::setLightShaderParameters(render::ShaderParameters* shaderParams, const Matrix44& world, const Aabb& bounds) const
{
	T_ASSERT (m_lightCount <= MaxLightCount);

	const Light* lightDirectional[MaxLightCount]; int lightDirectionalCount = 0;
	const Light* lightPoint[MaxLightCount]; int lightPointCount = 0;

	Scalar radius = bounds.empty() ? Scalar(0.0f) : bounds.getExtent().length();

	// Collect lights affecting entity.
	for (int i = 0; i < m_lightCount; ++i)
	{
		const Light& light = m_lights[i];
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

	if (lightPointCount > 0 || lightDirectionalCount > 1)
	{
		// Enable complex lighting path with dynamic branching.
		shaderParams->setBooleanParameter(s_handleLightEnableComplex, true);
	}
	else
	{
		// Single directional light; enable simple lighting path.
		shaderParams->setBooleanParameter(s_handleLightEnableComplex, false);
	}

	// Pack light parameters.
	Vector4 lightPositionAndType[MaxLightCount], *lightPositionAndTypePtr = lightPositionAndType;
	Vector4 lightDirectionAndRange[MaxLightCount], *lightDirectionAndRangePtr = lightDirectionAndRange;
	Vector4 lightSunColor[MaxLightCount], *lightSunColorPtr = lightSunColor;
	Vector4 lightBaseColor[MaxLightCount], *lightBaseColorPtr = lightBaseColor;
	Vector4 lightShadowColor[MaxLightCount], *lightShadowColorPtr = lightShadowColor;

	for (int i = 0; i < lightDirectionalCount; ++i)
	{
		const static Vector4 c_typeDirectional(0.0f, 0.0f, 0.0f, float(LtDirectional));
		*lightPositionAndTypePtr++ = c_typeDirectional;
		*lightDirectionAndRangePtr++ = lightDirectional[i]->direction.xyz0();
		*lightSunColorPtr++ = lightDirectional[i]->sunColor;
		*lightBaseColorPtr++ = lightDirectional[i]->baseColor;
		*lightShadowColorPtr++ = lightDirectional[i]->shadowColor;
	}
	for (int i = 0; i < lightPointCount; ++i)
	{
		const static Vector4 c_typePoint(0.0f, 0.0f, 0.0f, float(LtPoint));
		*lightPositionAndTypePtr++ = lightPoint[i]->position.xyz0() + c_typePoint;
		*lightDirectionAndRangePtr++ = Vector4(0.0f, 0.0f, 0.0f, lightPoint[i]->range);
		*lightSunColorPtr++ = lightPoint[i]->sunColor;
		*lightBaseColorPtr++ = lightPoint[i]->baseColor;
		*lightShadowColorPtr++ = lightPoint[i]->shadowColor;
	}

	// Disable excessive lights.
	for (int i = lightDirectionalCount + lightPointCount; i < MaxLightCount; ++i)
	{
		const static Vector4 c_typeDisabled(0.0f, 0.0f, 0.0f, float(LtDisabled));
		*lightPositionAndTypePtr++ = c_typeDisabled;
		*lightDirectionAndRangePtr++ = Vector4::zero();
		*lightSunColorPtr++ = Vector4::zero();
		*lightBaseColorPtr++ = Vector4::zero();
		*lightShadowColorPtr++ = Vector4::zero();
	}

	// Finally set shader parameters.
	shaderParams->setVectorArrayParameter(s_handleLightPositionAndType, lightPositionAndType, MaxLightCount);
	shaderParams->setVectorArrayParameter(s_handleLightDirectionAndRange, lightDirectionAndRange, MaxLightCount);
	shaderParams->setVectorArrayParameter(s_handleLightSunColor, lightSunColor, MaxLightCount);
	shaderParams->setVectorArrayParameter(s_handleLightBaseColor, lightBaseColor, MaxLightCount);
	shaderParams->setVectorArrayParameter(s_handleLightShadowColor, lightShadowColor, MaxLightCount);
}

void WorldRenderView::setShadowMapShaderParameters(render::ShaderParameters* shaderParams) const
{
	if (m_shadowMask)
	{
		shaderParams->setBooleanParameter(s_handleShadowEnable, true);
		shaderParams->setSamplerTexture(s_handleShadowMask, m_shadowMask);
	}
	else
		shaderParams->setBooleanParameter(s_handleShadowEnable, false);
}

void WorldRenderView::setDepthMapShaderParameters(render::ShaderParameters* shaderParams) const
{
	if (m_depthMap)
	{
		shaderParams->setBooleanParameter(s_handleDepthEnable, true);
		shaderParams->setSamplerTexture(s_handleDepthMap, m_depthMap);
	}
	else
		shaderParams->setBooleanParameter(s_handleDepthEnable, false);
}

void WorldRenderView::setShaderParameters(render::ShaderParameters* shaderParams) const
{
	setTechniqueShaderParameters(shaderParams);
	setWorldShaderParameters(shaderParams, Matrix44::identity(), Matrix44::identity());

	// Set these parameters only if we're rendering using default technique.
	if (m_technique == s_handleDefaultTechnique)
	{
		setLightShaderParameters(shaderParams);
		setShadowMapShaderParameters(shaderParams);
		setDepthMapShaderParameters(shaderParams);
	}
}

void WorldRenderView::setShaderParameters(render::ShaderParameters* shaderParams, const Matrix44& world, const Matrix44& worldPrevious, const Aabb& bounds) const
{
	setTechniqueShaderParameters(shaderParams);
	setWorldShaderParameters(shaderParams, world, worldPrevious);

	// Set these parameters only if we're rendering using default technique.
	if (m_technique == s_handleDefaultTechnique)
	{
		setLightShaderParameters(shaderParams, world, bounds);
		setShadowMapShaderParameters(shaderParams);
		setDepthMapShaderParameters(shaderParams);
	}
}

	}
}
