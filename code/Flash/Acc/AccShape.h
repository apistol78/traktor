#ifndef traktor_flash_AccShape_H
#define traktor_flash_AccShape_H

#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Aabb2.h"
#include "Core/Math/Matrix33.h"
#include "Core/Math/Matrix44.h"
#include "Flash/Polygon.h"
#include "Flash/Acc/AccShapeVertexPool.h"
#include "Resource/Proxy.h"
#include "Render/Types.h"

namespace traktor
{
	namespace render
	{

class ITexture;
class RenderContext;
class Shader;
class VertexBuffer;

	}

	namespace flash
	{

class AccBitmapRect;
class AccGradientCache;
class AccShapeResources;
class AccTextureCache;
class FlashCanvas;
class FlashDictionary;
class FlashFillStyle;
class FlashLineStyle;
class FlashShape;
struct SwfCxTransform;

/*! \brief Accelerated shape.
 * \ingroup Flash
 */
class AccShape : public Object
{
public:
	struct RenderBatch
	{
		Ref< AccBitmapRect > texture;
		render::Primitives primitives;
	};

	AccShape(AccShapeResources* shapeResources);

	virtual ~AccShape();

	bool create(
		AccShapeVertexPool* vertexPool,
		AccGradientCache* gradientCache,
		AccTextureCache* textureCache,
		const FlashDictionary& dictionary,
		const AlignedVector< FlashFillStyle >& fillStyles,
		const AlignedVector< FlashLineStyle >& lineStyles,
		const AlignedVector< Path >& paths,
		bool oddEven
	);

	bool create(
		AccShapeVertexPool* vertexPool,
		AccGradientCache* gradientCache,
		AccTextureCache* textureCache,
		const FlashDictionary& dictionary,
		const AlignedVector< FlashFillStyle >& fillStyles,
		const AlignedVector< FlashLineStyle >& lineStyles,
		const FlashShape& shape,
		bool oddEven
	);

	bool create(
		AccShapeVertexPool* vertexPool,
		AccGradientCache* gradientCache,
		AccTextureCache* textureCache,
		const FlashDictionary& dictionary,
		const AlignedVector< FlashFillStyle >& fillStyles,
		const AlignedVector< FlashLineStyle >& lineStyles,
		const FlashCanvas& canvas
	);

	void destroy();

	void render(
		render::RenderContext* renderContext,
		const Matrix33& transform,
		const Vector4& frameBounds,
		const Vector4& frameTransform,
		const SwfCxTransform& cxform,
		bool maskWrite,
		bool maskIncrement,
		uint8_t maskReference,
		uint8_t blendMode
	);

	const Aabb2& getBounds() const { return m_bounds; }

	const AlignedVector< RenderBatch >& getRenderBatches() const { return m_renderBatches; }

private:
	AccShapeResources* m_shapeResources;
	AccShapeVertexPool* m_vertexPool;
	AccShapeVertexPool::Range m_vertexRange;
	AlignedVector< RenderBatch > m_renderBatches;
	Aabb2 m_bounds;
	uint8_t m_batchFlags;
};

	}
}

#endif	// traktor_flash_AccShape_H
