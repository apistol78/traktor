#include <limits>
#include "Core/Misc/SafeDestroy.h"
#include "Render/IRenderSystem.h"
#include "Render/Shader.h"
#include "Render/VertexBuffer.h"
#include "Render/VertexElement.h"
#include "Render/Context/RenderBlock.h"
#include "Render/Context/RenderContext.h"
#include "Resource/IResourceManager.h"
#include "World/IrradianceGrid.h"
#include "World/Deferred/LightRendererDeferred.h"

namespace traktor
{
	namespace world
	{
		namespace
		{

const resource::Id< render::Shader > c_lightShader(L"{707DE0B0-0E2B-A44A-9441-9B1FCFD428AA}");
const resource::Id< render::Shader > c_reflectionShader(L"{F04EEA34-85E0-974F-BE97-79D24C6ACFBD}");
const resource::Id< render::Shader > c_fogShader(L"{9453D74C-76C4-8748-9A5B-9E3D6D4F9406}");

const render::Handle s_handleTime(L"World_Time");
const render::Handle s_handleShadowEnable(L"World_ShadowEnable");
const render::Handle s_handleReflectionsEnable(L"World_ReflectionsEnable");
const render::Handle s_handleIrradianceEnable(L"World_IrradianceEnable");
const render::Handle s_handleExtent(L"World_Extent");
const render::Handle s_handleProjection(L"World_Projection");
const render::Handle s_handleView(L"World_View");
const render::Handle s_handleViewInverse(L"World_ViewInverse");
const render::Handle s_handleMagicCoeffs(L"World_MagicCoeffs");
const render::Handle s_handleScreenMap(L"World_ScreenMap");
const render::Handle s_handleDepthMap(L"World_DepthMap");
const render::Handle s_handleNormalMap(L"World_NormalMap");
const render::Handle s_handleMiscMap(L"World_MiscMap");
const render::Handle s_handleColorMap(L"World_ColorMap");
const render::Handle s_handleOcclusionMap(L"World_OcclusionMap");
const render::Handle s_handleLightDiffuseMap(L"World_LightDiffuseMap");
const render::Handle s_handleLightSpecularMap(L"World_LightSpecularMap");
const render::Handle s_handleProbeDiffuse(L"World_ProbeDiffuse");
const render::Handle s_handleProbeSpecular(L"World_ProbeSpecular");
const render::Handle s_handleProbeSpecularMips(L"World_ProbeSpecularMips");
const render::Handle s_handleFogDistanceAndDensity(L"World_FogDistanceAndDensity");
const render::Handle s_handleFogColor(L"World_FogColor");
const render::Handle s_handleLightCount(L"World_LightCount");
const render::Handle s_handleShadowMask(L"World_ShadowMask");
const render::Handle s_handleShadowMapAtlas(L"World_ShadowMapAtlas");
const render::Handle s_handleReflectionMap(L"World_ReflectionMap");
const render::Handle s_handleLightSBuffer(L"World_LightSBuffer");
const render::Handle s_handleTileSBuffer(L"World_TileSBuffer");
const render::Handle s_handleIrradianceGridSize(L"World_IrradianceGridSize");
const render::Handle s_handleIrradianceGridSBuffer(L"World_IrradianceGridSBuffer");
const render::Handle s_handleIrradianceGridBoundsMin(L"World_IrradianceGridBoundsMin");
const render::Handle s_handleIrradianceGridBoundsMax(L"World_IrradianceGridBoundsMax");

#pragma pack(1)

struct LightVertex
{
	float pos[2];
	float texCoord[2];
};

#pragma pack()

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.LightRendererDeferred", LightRendererDeferred, Object)

bool LightRendererDeferred::create(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem
)
{
	if (!resourceManager->bind(c_lightShader, m_lightShader))
		return false;
	if (!resourceManager->bind(c_reflectionShader, m_reflectionShader))
		return false;
	if (!resourceManager->bind(c_fogShader, m_fogShader))
		return false;

	// Create fullscreen geometry.
	AlignedVector< render::VertexElement > vertexElements;
	vertexElements.push_back(render::VertexElement(render::DuPosition, render::DtFloat2, offsetof(LightVertex, pos)));
	vertexElements.push_back(render::VertexElement(render::DuCustom, render::DtFloat2, offsetof(LightVertex, texCoord)));

	m_vertexBufferQuad = renderSystem->createVertexBuffer(vertexElements, 6 * sizeof(LightVertex), false);
	if (!m_vertexBufferQuad)
		return false;

	LightVertex* vertex = (LightVertex*)m_vertexBufferQuad->lock();
	T_FATAL_ASSERT(vertex);

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

void LightRendererDeferred::destroy()
{
	safeDestroy(m_vertexBufferQuad);
}

void LightRendererDeferred::renderLights(
	render::RenderContext* renderContext,
	float time,
	int32_t lightCount,
	const Matrix44& projection,
	const Matrix44& view,
	render::StructBuffer* lightSBuffer,
	render::StructBuffer* tileSBuffer,
	IrradianceGrid* irradianceGrid,
	render::ITexture* depthMap,
	render::ITexture* normalMap,
	render::ITexture* miscMap,
	render::ITexture* colorMap,
	render::ITexture* occlusionMap,
	render::ITexture* shadowMask,
	render::ITexture* shadowMapAtlas,
	render::ITexture* reflectionMap
)
{
	auto lrb = renderContext->alloc< render::LambdaRenderBlock >(L"Lights");
	lrb->lambda = [=](render::IRenderView* renderView)
	{
		Scalar p11 = projection.get(0, 0);
		Scalar p22 = projection.get(1, 1);

		m_lightShader->setCombination(s_handleShadowEnable, bool(shadowMask != nullptr && shadowMask != nullptr));
		m_lightShader->setCombination(s_handleReflectionsEnable, bool(reflectionMap != nullptr));
		m_lightShader->setCombination(s_handleIrradianceEnable, bool(irradianceGrid != nullptr));

		m_lightShader->setFloatParameter(s_handleTime, time);
		m_lightShader->setFloatParameter(s_handleLightCount, float(lightCount));
		m_lightShader->setVectorParameter(s_handleMagicCoeffs, Vector4(1.0f / p11, 1.0f / p22, 0.0f, 0.0f));
		m_lightShader->setMatrixParameter(s_handleViewInverse, view.inverse());
		m_lightShader->setTextureParameter(s_handleDepthMap, depthMap);
		m_lightShader->setTextureParameter(s_handleNormalMap, normalMap);
		m_lightShader->setTextureParameter(s_handleMiscMap, miscMap);
		m_lightShader->setTextureParameter(s_handleColorMap, colorMap);
		m_lightShader->setTextureParameter(s_handleOcclusionMap, occlusionMap);
		m_lightShader->setTextureParameter(s_handleShadowMask, shadowMask);
		m_lightShader->setTextureParameter(s_handleShadowMapAtlas, shadowMapAtlas);
		m_lightShader->setTextureParameter(s_handleReflectionMap, reflectionMap);
		m_lightShader->setStructBufferParameter(s_handleLightSBuffer, lightSBuffer);
		m_lightShader->setStructBufferParameter(s_handleTileSBuffer, tileSBuffer);

		if (irradianceGrid)
		{
			const auto size = irradianceGrid->getSize();
			m_lightShader->setVectorParameter(s_handleIrradianceGridSize, Vector4((float)size[0], (float)size[1], (float)size[2], 0.0f));
			m_lightShader->setVectorParameter(s_handleIrradianceGridBoundsMin, irradianceGrid->getBoundingBox().mn);
			m_lightShader->setVectorParameter(s_handleIrradianceGridBoundsMax, irradianceGrid->getBoundingBox().mx);
			m_lightShader->setStructBufferParameter(s_handleIrradianceGridSBuffer, irradianceGrid->getBuffer());
		}

		m_lightShader->draw(renderView, m_vertexBufferQuad, 0, m_primitivesQuad);
	};
	renderContext->enqueue(lrb);
}

void LightRendererDeferred::renderReflections(
	render::RenderContext* renderContext,
	const Matrix44& projection,
	const Matrix44& view,
	const Matrix44& lastView,
	render::ITexture* screenMap,
	render::ITexture* depthMap,
	render::ITexture* normalMap,
	render::ITexture* miscMap
)
{
	auto lrb = renderContext->alloc< render::LambdaRenderBlock >(L"Reflections");
	lrb->lambda = [=](render::IRenderView* renderView)
	{
		Scalar p11 = projection.get(0, 0);
		Scalar p22 = projection.get(1, 1);

		m_reflectionShader->setMatrixParameter(s_handleProjection, projection);
		m_reflectionShader->setMatrixParameter(s_handleViewInverse, view.inverse());
		m_reflectionShader->setVectorParameter(s_handleMagicCoeffs, Vector4(1.0f / p11, 1.0f / p22, 0.0f, 0.0f));
		m_reflectionShader->setTextureParameter(s_handleScreenMap, screenMap);
		m_reflectionShader->setTextureParameter(s_handleDepthMap, depthMap);
		m_reflectionShader->setTextureParameter(s_handleNormalMap, normalMap);
		m_reflectionShader->setTextureParameter(s_handleMiscMap, miscMap);

		m_reflectionShader->draw(renderView, m_vertexBufferQuad, 0, m_primitivesQuad);
	};
	renderContext->enqueue(lrb);
}

void LightRendererDeferred::renderFog(
	render::RenderContext* renderContext,
	const Matrix44& projection,
	const Matrix44& view,
	const Vector4& fogDistanceAndDensity,
	const Vector4& fogColor,
	render::ITexture* depthMap,
	render::ITexture* normalMap,
	render::ITexture* miscMap,
	render::ITexture* colorMap
)
{
	auto lrb = renderContext->alloc< render::LambdaRenderBlock >(L"Fog");
	lrb->lambda = [=](render::IRenderView* renderView)
	{
		Scalar p11 = projection.get(0, 0);
		Scalar p22 = projection.get(1, 1);

		m_fogShader->setMatrixParameter(s_handleProjection, projection);
		m_fogShader->setMatrixParameter(s_handleView, view);
		m_fogShader->setMatrixParameter(s_handleViewInverse, view.inverse());
		m_fogShader->setVectorParameter(s_handleMagicCoeffs, Vector4(1.0f / p11, 1.0f / p22, 0.0f, 0.0f));
		m_fogShader->setVectorParameter(s_handleFogDistanceAndDensity, fogDistanceAndDensity);
		m_fogShader->setVectorParameter(s_handleFogColor, fogColor);
		m_fogShader->setTextureParameter(s_handleDepthMap, depthMap);
		m_fogShader->setTextureParameter(s_handleNormalMap, normalMap);
		m_fogShader->setTextureParameter(s_handleMiscMap, miscMap);
		m_fogShader->setTextureParameter(s_handleColorMap, colorMap);

		m_fogShader->draw(renderView, m_vertexBufferQuad, 0, m_primitivesQuad);
	};
	renderContext->enqueue(lrb);
}

	}
}
