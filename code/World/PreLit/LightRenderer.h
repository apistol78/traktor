#ifndef traktor_world_LightRenderer_H
#define traktor_world_LightRenderer_H

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
 * Render light primitives for pre-lit rendering
 * technique.
 */
class LightRenderer : public Object
{
	T_RTTI_CLASS;

public:
	LightRenderer();

	bool create(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem
	);

	void destroy();

	void render(
		render::IRenderView* renderView,
		const Matrix44& projection,
		const Matrix44& view,
		const Light& light,
		render::ITexture* depthMap,
		render::ITexture* normalMap,
		float shadowMaskSize,
		render::ITexture* shadowMask
	);

private:
	resource::Proxy< render::Shader > m_lightDirectionalShader;
	resource::Proxy< render::Shader > m_lightPointShader;
	resource::Proxy< render::Shader > m_lightSpotShader;
	Ref< render::VertexBuffer > m_vertexBufferQuad;
	render::Primitives m_primitivesQuad;
};

	}
}

#endif	// traktor_world_LightRenderer_H
