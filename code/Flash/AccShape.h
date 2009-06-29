#ifndef traktor_flash_AccShape_H
#define traktor_flash_AccShape_H

#include "Core/Heap/Ref.h"
#include "Core/Object.h"
#include "Core/Math/Matrix33.h"
#include "Core/Math/Matrix44.h"
#include "Core/Containers/AlignedVector.h"
#include "Resource/Proxy.h"
#include "Render/Types.h"

namespace traktor
{
	namespace resource
	{

class IResourceManager;

	}

	namespace render
	{

class RenderSystem;
class RenderView;
class Shader;
class Texture;
class VertexBuffer;

	}

	namespace flash
	{

class AccTextureCache;
class FlashMovie;
class FlashShape;
struct SwfCxTransform;

/*! \brief Accelerated shape.
 * \ingroup Flash
 */
class AccShape : public Object
{
public:
	bool create(
		resource::IResourceManager* resourceManager,
		render::RenderSystem* renderSystem,
		AccTextureCache& textureCache,
		const FlashMovie& movie,
		const FlashShape& shape
	);

	void destroy();

	void render(
		render::RenderView* renderView,
		const FlashShape& shape,
		const Vector4& frameSize,
		const Matrix33& transform,
		const SwfCxTransform& cxform,
		bool maskWrite,
		bool maskIncrement,
		uint8_t maskReference
	);

private:
	struct Batch
	{
		render::Texture* texture;
		Matrix33 textureMatrix;
		render::Primitives primitives;
	};

	resource::Proxy< render::Shader > m_shaderSolid;
	resource::Proxy< render::Shader > m_shaderTextured;
	resource::Proxy< render::Shader > m_shaderSolidMask;
	resource::Proxy< render::Shader > m_shaderTexturedMask;
	resource::Proxy< render::Shader > m_shaderIncrementMask;
	resource::Proxy< render::Shader > m_shaderDecrementMask;
	Ref< render::VertexBuffer > m_vertexBuffer;
	AlignedVector< Batch > m_batches;
};

	}
}

#endif	// traktor_flash_AccShape_H
