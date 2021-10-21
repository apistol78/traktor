#pragma once

#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Containers/StaticVector.h"
#include "Core/Math/Aabb2.h"
#include "Core/Math/Matrix33.h"
#include "Core/Math/Matrix44.h"
#include "Resource/Proxy.h"
#include "Render/Types.h"
#include "Spark/Polygon.h"
#include "Spark/Acc/AccShapeVertexPool.h"

namespace traktor
{
	namespace render
	{

class Buffer;
class IRenderSystem;
class ITexture;
class RenderPass;
class Shader;

	}

	namespace spark
	{

class AccBitmapRect;
class AccGradientCache;
class AccShapeResources;
class AccTextureCache;
class ColorTransform;
class Canvas;
class Dictionary;
class FillStyle;
class LineStyle;
class Shape;

/*! Accelerated shape.
 * \ingroup Spark
 */
class AccShape : public Object
{
public:
#pragma pack(1)
	struct FillVertex
	{
		float pos[2];
		uint8_t curvature[4];
		float texCoord[2];
		float texRect[4];
		uint8_t color[4];
	};
#pragma pack()

#pragma pack(1)
	struct LineVertex
	{
		float pos[2];
		float lineOffset;
	};
#pragma pack()

	struct FillRenderBatch
	{
		Ref< AccBitmapRect > texture;
		render::Primitives primitives;
		bool textureClamp;
	};

	struct LineRenderBatch
	{
		AccShapeVertexPool::Range vertexRange;
		Ref< render::ITexture > lineTexture;
		render::Primitives primitives;
		Color4f color;
		float width;
	};

	explicit AccShape(
		render::IRenderSystem* renderSystem,
		const AccShapeResources* shapeResources,
		AccShapeVertexPool* fillVertexPool,
		AccShapeVertexPool* lineVertexPool
	);

	virtual ~AccShape();

	bool createFromTriangles(
		AccGradientCache* gradientCache,
		AccTextureCache* textureCache,
		const Dictionary& dictionary,
		const AlignedVector< FillStyle >& fillStyles,
		const AlignedVector< LineStyle >& lineStyles,
		const AlignedVector< Triangle >& triangles,
		const AlignedVector< Line >& lines
	);

	bool createFromShape(
		AccGradientCache* gradientCache,
		AccTextureCache* textureCache,
		const Dictionary& dictionary,
		const Shape& shape
	);

	bool createFromGlyph(
		AccGradientCache* gradientCache,
		AccTextureCache* textureCache,
		const Dictionary& dictionary,
		const Shape& shape
	);

	bool createFromCanvas(
		AccGradientCache* gradientCache,
		AccTextureCache* textureCache,
		const Canvas& canvas
	);

	void destroy();

	void render(
		render::RenderPass* renderPass,
		const Matrix33& transform,
		const Vector4& clipBounds,
		const Vector4& frameBounds,
		const Vector4& frameTransform,
		const ColorTransform& cxform,
		bool maskWrite,
		bool maskIncrement,
		uint8_t maskReference,
		uint8_t blendMode
	);

	const Aabb2& getBounds() const { return m_bounds; }

private:
	render::IRenderSystem* m_renderSystem;
	const AccShapeResources* m_shapeResources;
	AccShapeVertexPool* m_fillVertexPool;
	AccShapeVertexPool* m_lineVertexPool;
	AccShapeVertexPool::Range m_fillVertexRange;
	AlignedVector< FillRenderBatch > m_fillRenderBatches;
	AlignedVector< LineRenderBatch > m_lineRenderBatches;
	Aabb2 m_bounds;
	uint8_t m_batchFlags = 0;
};

	}
}

