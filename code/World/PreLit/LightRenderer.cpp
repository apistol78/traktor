#include <limits>
#include "Core/Math/Aabb3.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/Shader.h"
#include "Render/VertexBuffer.h"
#include "Render/VertexElement.h"
#include "Resource/IResourceManager.h"
#include "World/PreLit/LightRenderer.h"

namespace traktor
{
	namespace world
	{
		namespace
		{

const Guid c_lightDirectionalShader(L"{9F5076A9-A090-3242-A395-B0A75DCB2E1F}");
const Guid c_lightPointShader(L"{6389690A-440C-364F-A5DA-5B53392F6B85}");
const Guid c_lightSpotShader(L"{13D5E181-2B54-D94F-9ECB-01D129DA4AE3}");

const float c_pointLightScreenAreaThresholdDim = 4.0f;
const float c_pointLightScreenAreaThreshold = 4.0f * (c_pointLightScreenAreaThresholdDim * c_pointLightScreenAreaThresholdDim) / (1280.0f * 720.0f);

render::handle_t s_handleShadowEnable;
render::handle_t s_handleExtent;
render::handle_t s_handleMagicCoeffs;
render::handle_t s_handleEyePosition;
render::handle_t s_handleDepthMap;
render::handle_t s_handleNormalMap;
render::handle_t s_handleShadowMaskSize;
render::handle_t s_handleShadowMask;
render::handle_t s_handleLightPosition;
render::handle_t s_handleLightPositionAndRadius;
render::handle_t s_handleLightDirectionAndRange;
render::handle_t s_handleLightSunColor;
render::handle_t s_handleLightBaseColor;
render::handle_t s_handleLightShadowColor;

#pragma pack(1)
struct LightVertex
{
	float pos[2];
	float texCoord[2];
};
#pragma pack()

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.LightRenderer", LightRenderer, Object)

LightRenderer::LightRenderer()
:	m_lightDirectionalShader(c_lightDirectionalShader)
,	m_lightPointShader(c_lightPointShader)
,	m_lightSpotShader(c_lightSpotShader)
{
	s_handleShadowEnable = render::getParameterHandle(L"ShadowEnable");
	s_handleExtent = render::getParameterHandle(L"Extent");
	s_handleMagicCoeffs = render::getParameterHandle(L"MagicCoeffs");
	s_handleEyePosition = render::getParameterHandle(L"EyePosition");
	s_handleDepthMap = render::getParameterHandle(L"DepthMap");
	s_handleNormalMap = render::getParameterHandle(L"NormalMap");
	s_handleShadowMaskSize = render::getParameterHandle(L"ShadowMaskSize");
	s_handleShadowMask = render::getParameterHandle(L"ShadowMask");
	s_handleLightPosition = render::getParameterHandle(L"LightPosition");
	s_handleLightPositionAndRadius = render::getParameterHandle(L"LightPositionAndRadius");
	s_handleLightDirectionAndRange = render::getParameterHandle(L"LightDirectionAndRange");
	s_handleLightSunColor = render::getParameterHandle(L"LightSunColor");
	s_handleLightBaseColor = render::getParameterHandle(L"LightBaseColor");
	s_handleLightShadowColor = render::getParameterHandle(L"LightShadowColor");
}

bool LightRenderer::create(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem
)
{
	if (!resourceManager->bind(m_lightDirectionalShader))
		return false;
	if (!resourceManager->bind(m_lightPointShader))
		return false;
	if (!resourceManager->bind(m_lightSpotShader))
		return false;

	std::vector< render::VertexElement > vertexElements;
	vertexElements.push_back(render::VertexElement(render::DuPosition, render::DtFloat2, offsetof(LightVertex, pos)));
	vertexElements.push_back(render::VertexElement(render::DuCustom, render::DtFloat2, offsetof(LightVertex, texCoord)));

	m_vertexBufferQuad = renderSystem->createVertexBuffer(vertexElements, 6 * sizeof(LightVertex), false);
	if (!m_vertexBufferQuad)
		return false;

	LightVertex* vertex = reinterpret_cast< LightVertex* >(m_vertexBufferQuad->lock());
	T_ASSERT (vertex);

	vertex[0].pos[0] = -1.0f; vertex[0].pos[1] =  1.0f; vertex[0].texCoord[0] = 0.0f; vertex[0].texCoord[1] = 0.0f;
	vertex[1].pos[0] =  1.0f; vertex[1].pos[1] =  1.0f; vertex[1].texCoord[0] = 1.0f; vertex[1].texCoord[1] = 0.0f;
	vertex[2].pos[0] =  1.0f; vertex[2].pos[1] = -1.0f; vertex[2].texCoord[0] = 1.0f; vertex[2].texCoord[1] = 1.0f;

	vertex[3].pos[0] = -1.0f; vertex[3].pos[1] =  1.0f; vertex[3].texCoord[0] = 0.0f; vertex[3].texCoord[1] = 0.0f;
	vertex[4].pos[0] =  1.0f; vertex[4].pos[1] = -1.0f; vertex[4].texCoord[0] = 1.0f; vertex[4].texCoord[1] = 1.0f;
	vertex[5].pos[0] = -1.0f; vertex[5].pos[1] = -1.0f; vertex[5].texCoord[0] = 0.0f; vertex[5].texCoord[1] = 1.0f;

	m_vertexBufferQuad->unlock();

	m_primitivesQuad.setNonIndexed(render::PtTriangles, 0, 2);

	return true;
}

void LightRenderer::destroy()
{
	m_lightDirectionalShader.invalidate();
}

void LightRenderer::render(
	render::IRenderView* renderView,
	const Matrix44& projection,
	const Matrix44& view,
	const Vector4& eyePosition,
	const Light& light,
	render::ITexture* depthMap,
	render::ITexture* normalMap,
	float shadowMaskSize,
	render::ITexture* shadowMask
)
{
	Scalar p11 = projection.get(0, 0);
	Scalar p22 = projection.get(1, 1);

	if (light.type == LtDirectional)
	{
		if (!m_lightDirectionalShader.validate())
			return;

		Vector4 lightDirectionAndRange = view * light.direction.xyz0() + Vector4(0.0f, 0.0f, 0.0f, light.range);

		renderView->setVertexBuffer(m_vertexBufferQuad);

		m_lightDirectionalShader->setCombination(s_handleShadowEnable, shadowMask != 0);
		m_lightDirectionalShader->setFloatParameter(s_handleShadowMaskSize, 0.5f / shadowMaskSize);
		m_lightDirectionalShader->setTextureParameter(s_handleShadowMask, shadowMask);

		m_lightDirectionalShader->setVectorParameter(s_handleMagicCoeffs, Vector4(1.0f / p11, 1.0f / p22, 0.0f, 0.0f));
		m_lightDirectionalShader->setVectorParameter(s_handleEyePosition, eyePosition);

		m_lightDirectionalShader->setTextureParameter(s_handleDepthMap, depthMap);
		m_lightDirectionalShader->setTextureParameter(s_handleNormalMap, normalMap);

		m_lightDirectionalShader->setVectorParameter(s_handleLightDirectionAndRange, lightDirectionAndRange);
		m_lightDirectionalShader->setVectorParameter(s_handleLightSunColor, light.sunColor);
		m_lightDirectionalShader->setVectorParameter(s_handleLightBaseColor, light.baseColor);
		m_lightDirectionalShader->setVectorParameter(s_handleLightShadowColor, light.shadowColor);

		m_lightDirectionalShader->draw(renderView, m_primitivesQuad);
	}
	else if (light.type == LtPoint)
	{
		if (!m_lightPointShader.validate())
			return;

		Vector4 lightPosition = view * light.position.xyz1();
		Vector4 lightDirectionAndRange = view * light.direction.xyz0() + Vector4(0.0f, 0.0f, 0.0f, light.range);

		// Calculate screen bounding box of light.
		Aabb3 lightBoundingBox;
		lightBoundingBox.contain(light.position.xyz1(), light.range);

		Vector4 extents[8];
		lightBoundingBox.getExtents(extents);

		Vector4 mn(
			std::numeric_limits< float >::max(),
			std::numeric_limits< float >::max(),
			std::numeric_limits< float >::max(),
			std::numeric_limits< float >::max()
		);
		Vector4 mx(
			-std::numeric_limits< float >::max(),
			-std::numeric_limits< float >::max(),
			-std::numeric_limits< float >::max(),
			-std::numeric_limits< float >::max()
		);

		Matrix44 viewProj = projection * view;
		for (int i = 0; i < sizeof_array(extents); ++i)
		{
			Vector4 p = viewProj * extents[i];
			if (p.w() <= 0.0f)
			{
				// Bounding box clipped to view plane; clip edge with view plane.
				mn = Vector4(-1.0f, -1.0f, 0.0f, 0.0f);
				mx = Vector4(1.0f, 1.0f, 0.0f, 0.0f);
				break;
			}

			// Homogeneous divide.
			p /= p.w();

			// Track screen space extents.
			mn = min(mn, p);
			mx = max(mx, p);
		}

		// Ensure we're visible.
		if (mn.x() > 1.0f || mn.y() > 1.0f || mx.x() < -1.0f || mx.y() < -1.0f)
			return;

		// Calculate area of light quad; do before clipping to screen edges as
		// we don't want to accidentally cull near lights which are close to the edge.
		float area = (mx.x() - mn.x()) * (mx.y() - mn.y());
		if (area < c_pointLightScreenAreaThreshold)
			return;

		// Clip quad to screen edges.
		mn = max(mn, Vector4(-1.0f, -1.0f, 0.0f, 0.0f));
		mx = min(mx, Vector4(1.0f, 1.0f, 0.0f, 0.0f));

		// Render quad primitive.
		renderView->setVertexBuffer(m_vertexBufferQuad);

		m_lightPointShader->setVectorParameter(s_handleExtent, Vector4(mn.x(), mn.y(), mx.x(), mx.y()));

		m_lightPointShader->setVectorParameter(s_handleMagicCoeffs, Vector4(1.0f / p11, 1.0f / p22, 0.0f, 0.0f));
		m_lightPointShader->setVectorParameter(s_handleEyePosition, eyePosition);

		m_lightPointShader->setTextureParameter(s_handleDepthMap, depthMap);
		m_lightPointShader->setTextureParameter(s_handleNormalMap, normalMap);

		m_lightPointShader->setVectorParameter(s_handleLightPosition, lightPosition);
		m_lightPointShader->setVectorParameter(s_handleLightDirectionAndRange, lightDirectionAndRange);
		m_lightPointShader->setVectorParameter(s_handleLightSunColor, light.sunColor);
		m_lightPointShader->setVectorParameter(s_handleLightBaseColor, light.baseColor);
		m_lightPointShader->setVectorParameter(s_handleLightShadowColor, light.shadowColor);

		m_lightPointShader->draw(renderView, m_primitivesQuad);
	}
	else if (light.type == LtSpot)
	{
		if (!m_lightSpotShader.validate())
			return;

		Vector4 lightPositionAndRadius = (view * light.position.xyz1()).xyz0() + Vector4(0.0f, 0.0f, 0.0f, light.radius);
		Vector4 lightDirectionAndRange = view * light.direction.xyz0() + Vector4(0.0f, 0.0f, 0.0f, light.range);

		// Calculate screen bounding box of light.
		Aabb3 lightBoundingBox;
		lightBoundingBox.contain(light.position.xyz1(), light.range);

		Vector4 extents[8];
		lightBoundingBox.getExtents(extents);

		Vector4 mn(
			std::numeric_limits< float >::max(),
			std::numeric_limits< float >::max(),
			std::numeric_limits< float >::max(),
			std::numeric_limits< float >::max()
		);
		Vector4 mx(
			-std::numeric_limits< float >::max(),
			-std::numeric_limits< float >::max(),
			-std::numeric_limits< float >::max(),
			-std::numeric_limits< float >::max()
		);

		Matrix44 viewProj = projection * view;
		for (int i = 0; i < sizeof_array(extents); ++i)
		{
			Vector4 p = viewProj * extents[i];
			if (p.w() <= 0.0f)
			{
				// Bounding box clipped to view plane; clip edge with view plane.
				mn = Vector4(-1.0f, -1.0f, 0.0f, 0.0f);
				mx = Vector4(1.0f, 1.0f, 0.0f, 0.0f);
				break;
			}

			// Homogeneous divide.
			p /= p.w();

			// Track screen space extents.
			mn = min(mn, p);
			mx = max(mx, p);
		}

		// Ensure we're visible.
		if (mn.x() > 1.0f || mn.y() > 1.0f || mx.x() < -1.0f || mx.y() < -1.0f)
			return;

		// Calculate area of light quad; do before clipping to screen edges as
		// we don't want to accidentally cull near lights which are close to the edge.
		float area = (mx.x() - mn.x()) * (mx.y() - mn.y());
		if (area < c_pointLightScreenAreaThreshold)
			return;

		// Clip quad to screen edges.
		mn = max(mn, Vector4(-1.0f, -1.0f, 0.0f, 0.0f));
		mx = min(mx, Vector4(1.0f, 1.0f, 0.0f, 0.0f));

		// Render quad primitive.
		renderView->setVertexBuffer(m_vertexBufferQuad);

		m_lightSpotShader->setCombination(s_handleShadowEnable, shadowMask != 0);
		m_lightSpotShader->setFloatParameter(s_handleShadowMaskSize, 0.5f / shadowMaskSize);
		m_lightSpotShader->setTextureParameter(s_handleShadowMask, shadowMask);

		m_lightSpotShader->setVectorParameter(s_handleExtent, Vector4(mn.x(), mn.y(), mx.x(), mx.y()));

		m_lightSpotShader->setVectorParameter(s_handleMagicCoeffs, Vector4(1.0f / p11, 1.0f / p22, 0.0f, 0.0f));
		m_lightSpotShader->setVectorParameter(s_handleEyePosition, eyePosition);

		m_lightSpotShader->setTextureParameter(s_handleDepthMap, depthMap);
		m_lightSpotShader->setTextureParameter(s_handleNormalMap, normalMap);

		m_lightSpotShader->setVectorParameter(s_handleLightPositionAndRadius, lightPositionAndRadius);
		m_lightSpotShader->setVectorParameter(s_handleLightDirectionAndRange, lightDirectionAndRange);
		m_lightSpotShader->setVectorParameter(s_handleLightSunColor, light.sunColor);
		m_lightSpotShader->setVectorParameter(s_handleLightBaseColor, light.baseColor);
		m_lightSpotShader->setVectorParameter(s_handleLightShadowColor, light.shadowColor);

		m_lightSpotShader->draw(renderView, m_primitivesQuad);
	}
}

	}
}
