#ifndef traktor_flash_AccShape_H
#define traktor_flash_AccShape_H

#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"
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

class AccShapeResources;
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
	AccShape(AccShapeResources* shapeResources, AccShapeVertexPool* vertexPool);

	bool createTesselation(const FlashShape& shape);

	bool createRenderable(
		AccTextureCache& textureCache,
		const FlashMovie& movie,
		const FlashShape& shape
	);

	void destroy();

	void render(
		render::RenderContext* renderContext,
		const FlashShape& shape,
		const Matrix33& transform,
		const Vector4& frameSize,
		const Vector4& viewSize,
		const Vector4& viewOffset,
		float screenOffsetScale,
		const SwfCxTransform& cxform,
		bool maskWrite,
		bool maskIncrement,
		uint8_t maskReference
	);

	const SwfRect& getBounds() const { return m_bounds; }

private:
	struct TesselationBatch
	{
		AlignedVector< Triangle > triangles;
		AlignedVector< Line > lines;
	};

	struct RenderBatch
	{
		render::ITexture* texture;
		Matrix33 textureMatrix;
		render::Primitives primitives;
	};

	Ref< AccShapeResources > m_shapeResources;
	Ref< AccShapeVertexPool > m_vertexPool;
	AlignedVector< TesselationBatch > m_tesselationBatches;
	uint32_t m_tesselationTriangleCount;
	AccShapeVertexPool::Range m_vertexRange;
	AlignedVector< RenderBatch > m_renderBatches;
	SwfRect m_bounds;
	uint8_t m_batchFlags;
};

	}
}

#endif	// traktor_flash_AccShape_H
