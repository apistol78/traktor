#ifndef traktor_world_LightRendererDeferred_H
#define traktor_world_LightRendererDeferred_H

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

	void renderLight(
		render::IRenderView* renderView,
		float time,
		const Matrix44& projection,
		const Matrix44& view,
		const Light& light,
		render::ITexture* depthMap,
		render::ITexture* normalMap,
		render::ITexture* miscMap,
		render::ITexture* colorMap,
		float shadowMaskSize,
		render::ITexture* shadowMask
	);

	void renderReflections(
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
	resource::Proxy< render::Shader > m_lightDirectionalShader;
	resource::Proxy< render::Shader > m_lightPointShader;
	resource::Proxy< render::Shader > m_lightSpotShader;
	resource::Proxy< render::Shader > m_reflectionShader;
	resource::Proxy< render::Shader > m_fogShader;
	Ref< render::VertexBuffer > m_vertexBufferQuad;
	render::Primitives m_primitivesQuad;
};

	}
}

#endif	// traktor_world_LightRendererDeferred_H
