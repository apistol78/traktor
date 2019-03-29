#pragma once

#include "Core/Object.h"
#include "Render/Types.h"
#include "Resource/Proxy.h"
#include "World/WorldTypes.h"

namespace traktor
{
	namespace render
	{

class IRenderSystem;
class IRenderView;
class ITexture;
class Shader;
class VertexBuffer;

	}

	namespace resource
	{

class IResourceManager;

	}

	namespace world
	{

/*! \brief Light renderer.
 * \ingroup World
 *
 * Render light primitives for deferred rendering
 * technique.
 */
class LightRendererDeferred : public Object
{
	T_RTTI_CLASS;

public:
	LightRendererDeferred();

	bool create(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem
	);

	void destroy();

	/*! \brief Render all lights. */
	void renderLights(
		render::IRenderView* renderView,
		float time,
		int32_t lightCount,
		const Matrix44& projection,
		const Matrix44& view,
		render::StructBuffer* lightSBuffer,
		render::StructBuffer* tileSBuffer,
		render::ITexture* depthMap,
		render::ITexture* normalMap,
		render::ITexture* miscMap,
		render::ITexture* colorMap,
		render::ITexture* shadowMapCascade,
		render::ITexture* shadowMapAtlas
	);

	/*! \brief Render screenspace reflections. */
	void renderReflections(
		render::IRenderView* renderView,
		const Matrix44& projection,
		const Matrix44& view,
		const Vector4& fogDistanceAndDensity,
		const Vector4& fogColor,
		bool traceReflections,
		render::ITexture* screenMap,
		render::ITexture* reflectionMap,
		render::ITexture* depthMap,
		render::ITexture* normalMap,
		render::ITexture* miscMap,
		render::ITexture* colorMap
	);

	/*! \brief Render fog. */
	void renderFog(
		render::IRenderView* renderView,
		const Matrix44& projection,
		const Matrix44& view,
		const Vector4& fogDistanceAndDensity,
		const Vector4& fogColor,
		render::ITexture* reflectionMap,
		render::ITexture* depthMap,
		render::ITexture* normalMap,
		render::ITexture* miscMap,
		render::ITexture* colorMap
	);

private:
	resource::Proxy< render::Shader > m_lightShader;
	resource::Proxy< render::Shader > m_reflectionShader;
	resource::Proxy< render::Shader > m_fogShader;
	Ref< render::VertexBuffer > m_vertexBufferQuad;
	render::Primitives m_primitivesQuad;
};

	}
}

