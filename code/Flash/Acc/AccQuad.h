#ifndef traktor_flash_AccQuad_H
#define traktor_flash_AccQuad_H

#include "Core/Object.h"
#include "Core/Math/Aabb2.h"
#include "Core/Math/Matrix33.h"
#include "Resource/Proxy.h"

namespace traktor
{
	namespace resource
	{

class IResourceManager;

	}

	namespace render
	{

class IRenderSystem;
class ITexture;
class RenderContext;
class Shader;
class VertexBuffer;

	}

	namespace flash
	{

struct SwfCxTransform;

/*! \brief Simple unit-quad shape.
 * \ingroup Flash
 */
class AccQuad : public Object
{
public:
	bool create(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem
	);

	void destroy();

	void render(
		render::RenderContext* renderContext,
		const Aabb2& bounds,
		const Matrix33& transform,
		const Vector4& frameSize,
		const Vector4& frameTransform,
		const SwfCxTransform& cxform,
		render::ITexture* texture,
		const Vector4& textureOffset,
		bool maskWrite,
		bool maskIncrement,
		uint8_t maskReference
	);

private:
	resource::Proxy< render::Shader > m_shaderSolid;
	resource::Proxy< render::Shader > m_shaderTextured;
	resource::Proxy< render::Shader > m_shaderIncrementMask;
	resource::Proxy< render::Shader > m_shaderDecrementMask;
	Ref< render::VertexBuffer > m_vertexBuffer;
};

	}
}

#endif	// traktor_flash_AccQuad_H
