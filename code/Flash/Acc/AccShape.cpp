#include <limits>
#include "Core/Log/Log.h"
#include "Core/Math/Color4ub.h"
#include "Flash/FlashCanvas.h"
#include "Flash/FlashDictionary.h"
#include "Flash/FlashShape.h"
#include "Flash/FlashBitmap.h"
#include "Flash/Acc/AccShape.h"
#include "Flash/Acc/AccShapeResources.h"
#include "Flash/Acc/AccTextureCache.h"
#include "Flash/Acc/Triangulator.h"
#include "Render/Shader.h"
#include "Render/VertexBuffer.h"
#include "Render/Context/RenderContext.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

enum BatchFlags
{
	BfHaveSolid = 1,
	BfHaveTextured = 2
};

const float c_controlPoints[3][2] =
{
	{ 0.0f, 0.0f },
	{ 0.5f, 0.0f },
	{ 1.0f, 1.0f }
};

const float c_curveSign[3] =
{
	0.0f,
	1.0f,
	-1.0f
};

		}

AccShape::AccShape(AccShapeResources* shapeResources)
:	m_shapeResources(shapeResources)
,	m_vertexPool(0)
,	m_tesselationTriangleCount(0)
,	m_batchFlags(0)
,	m_needUpdate(true)
{
}

AccShape::~AccShape()
{
	destroy();
}

bool AccShape::createTesselation(const std::list< Path >& paths)
{
	AlignedVector< Segment > segments;
	Triangulator triangulator;

	m_tesselationBatches.resize(0);
	m_tesselationBatches.reserve(paths.size());

	m_tesselationTriangleCount = 0;
	m_bounds.mn.x = m_bounds.mn.y =  std::numeric_limits< float >::max();
	m_bounds.mx.x = m_bounds.mx.y = -std::numeric_limits< float >::max();

	// Create triangles through tessellation.
	for (std::list< Path >::const_iterator i = paths.begin(); i != paths.end(); ++i)
	{
		segments.resize(0);

		m_tesselationBatches.push_back(TesselationBatch());
		TesselationBatch& batch = m_tesselationBatches.back();

		const std::list< SubPath >& subPaths = i->getSubPaths();
		for (std::list< SubPath >::const_iterator j = subPaths.begin(); j != subPaths.end(); ++j)
		{
			for (std::vector< SubPathSegment >::const_iterator k = j->segments.begin(); k != j->segments.end(); ++k)
			{
				switch (k->type)
				{
				case SpgtLinear:
					{
						Segment s;
						s.v[0] = i->getPoints().at(k->pointsOffset);
						s.v[1] = i->getPoints().at(k->pointsOffset + 1);
						s.curve = false;
						s.fillStyle0 = j->fillStyle0;
						s.fillStyle1 = j->fillStyle1;
						s.lineStyle = j->lineStyle;
						segments.push_back(s);
					}
					break;

				case SpgtQuadratic:
					{
						const Vector2i& cp0 = i->getPoints().at(k->pointsOffset);
						const Vector2i& cp1 = i->getPoints().at(k->pointsOffset + 1);
						const Vector2i& cp2 = i->getPoints().at(k->pointsOffset + 2);

						Segment s;
						s.v[0] = cp0;
						s.v[1] = cp2;
						s.c = cp1;
						s.curve = true;
						s.fillStyle0 = j->fillStyle0;
						s.fillStyle1 = j->fillStyle1;
						s.lineStyle = j->lineStyle;
						segments.push_back(s);
					}
					break;

				default:
					break;
				}
			}
		}

		triangulator.triangulate(segments, batch.triangles);
		m_tesselationTriangleCount += uint32_t(batch.triangles.size());

		for (AlignedVector< Triangle >::const_iterator j = batch.triangles.begin(); j != batch.triangles.end(); ++j)
		{
			for (int k = 0; k < 3; ++k)
			{
				Vector2 pt = j->v[k].toVector2();
				m_bounds.mn.x = min< float >(m_bounds.mn.x, pt.x);
				m_bounds.mn.y = min< float >(m_bounds.mn.y, pt.y);
				m_bounds.mx.x = max< float >(m_bounds.mx.x, pt.x);
				m_bounds.mx.y = max< float >(m_bounds.mx.y, pt.y);
			}
		}
	}

	m_needUpdate = true;
	return true;
}

bool AccShape::createTesselation(const FlashShape& shape)
{
	const std::list< Path >& paths = shape.getPaths();
	return createTesselation(paths);
}

bool AccShape::createTesselation(const FlashCanvas& canvas)
{
	const std::list< Path >& paths = canvas.getPaths();
	return createTesselation(paths);
}

bool AccShape::updateRenderable(
	AccShapeVertexPool* vertexPool,
	AccTextureCache* textureCache,
	const FlashDictionary& dictionary,
	const AlignedVector< FlashFillStyle >& fillStyles,
	const AlignedVector< FlashLineStyle >& lineStyles
)
{
	if (!m_tesselationTriangleCount)
		return false;

	// Do we need to update?
	if (!m_needUpdate)
		return true;

	// Free previous vertex range if any.
	if (m_vertexRange.vertexBuffer)
	{
		m_vertexPool->releaseRange(m_vertexRange);
		m_vertexPool = 0;
		m_vertexRange.vertexBuffer = 0;
	}

	// Allocate vertex range.
	if (!vertexPool->acquireRange(m_tesselationTriangleCount * 3, m_vertexRange))
		return false;

	T_ASSERT (m_vertexRange.vertexBuffer);
	m_vertexPool = vertexPool;

	const static Matrix33 textureTS = translate(0.5f, 0.5f) * scale(1.0f / 32768.0f, 1.0f / 32768.0f);

	uint32_t vertexOffset = 0;
	Matrix33 textureMatrix;

	m_renderBatches[0].reserve(m_tesselationBatches.size());
	m_renderBatches[1].reserve(m_tesselationBatches.size());
	m_renderBatches[2].reserve(m_tesselationBatches.size());
	m_batchFlags = 0;

	AccShapeVertexPool::Vertex* vertex = static_cast< AccShapeVertexPool::Vertex* >(m_vertexRange.vertexBuffer->lock());
	if (!vertex)
		return false;

	for (uint32_t type = 0; type < 3; ++type)
	{
		AlignedVector< RenderBatch >& batch = m_renderBatches[type];

		for (AlignedVector< TesselationBatch >::const_iterator i = m_tesselationBatches.begin(); i != m_tesselationBatches.end(); ++i)
		{
			for (AlignedVector< Triangle >::const_iterator j = i->triangles.begin(); j != i->triangles.end(); ++j)
			{
				if (j->type != type)
					continue;

				Color4ub color(255, 255, 255, 255);
				resource::Proxy< render::ITexture > texture;
				bool textureClamp = false;

				if (j->fillStyle && j->fillStyle - 1 < uint16_t(fillStyles.size()))
				{
					const FlashFillStyle& style = fillStyles[j->fillStyle - 1];

					const AlignedVector< FlashFillStyle::ColorRecord >& colorRecords = style.getColorRecords();
					if (colorRecords.size() > 1)
					{
						// Create gradient texture.
						T_ASSERT (textureCache);
						texture = textureCache->getGradientTexture(style);
						textureMatrix = textureTS * style.getGradientMatrix().inverse();
						textureClamp = true;
						m_batchFlags |= BfHaveTextured;
					}
					else if (colorRecords.size() == 1)
					{
						// Solid color.
						color.r = colorRecords.front().color.red;
						color.g = colorRecords.front().color.green;
						color.b = colorRecords.front().color.blue;
						color.a = colorRecords.front().color.alpha;
						m_batchFlags |= BfHaveSolid;
					}

					const FlashBitmap* bitmap = dictionary.getBitmap(style.getFillBitmap());
					if (bitmap)
					{
						T_ASSERT_M (!texture, L"Cannot combine gradients and bitmaps");
						T_ASSERT (textureCache);
						texture = textureCache->getBitmapTexture(*bitmap);
						textureMatrix =
							scale(
								1.0f / bitmap->getWidth(),
								1.0f / bitmap->getHeight()
							) *
							style.getFillBitmapMatrix().inverse();
						textureClamp = false;
						m_batchFlags |= BfHaveTextured;
					}
				}

				if (batch.empty() || batch.back().texture != texture || batch.back().textureClamp != textureClamp)
				{
					batch.push_back(RenderBatch());
					batch.back().primitives.setNonIndexed(render::PtTriangles, vertexOffset, 0);
					batch.back().texture = texture;
					batch.back().textureMatrix = textureMatrix;
					batch.back().textureClamp = textureClamp;
				}

				for (int k = 0; k < 3; ++k)
				{
					vertex->pos[0] = float(j->v[k].x);
					vertex->pos[1] = float(j->v[k].y);
					vertex->uv[0] = c_controlPoints[k][0];
					vertex->uv[1] = c_controlPoints[k][1];
					vertex->color[0] = color.r;
					vertex->color[1] = color.g;
					vertex->color[2] = color.b;
					vertex->color[3] = color.a;
					vertex++;
				}

				batch.back().primitives.count++;

				vertexOffset += 3;
			}
		}
	}

	m_vertexRange.vertexBuffer->unlock();

	// Some shapes doesn't expose styles, such as glyphs, but are
	// assumed to be solids.
	if (!m_batchFlags)
		m_batchFlags |= BfHaveSolid;

	m_needUpdate = false;
	return true;
}

void AccShape::destroy()
{
	m_shapeResources = 0;

	if (m_vertexPool)
	{
		m_vertexPool->releaseRange(m_vertexRange);
		m_vertexPool = 0;
		m_vertexRange = AccShapeVertexPool::Range();
	}

	m_tesselationBatches.clear();
	m_renderBatches[0].clear();
	m_renderBatches[1].clear();
	m_renderBatches[2].clear();
}

void AccShape::render(
	render::RenderContext* renderContext,
	const Matrix33& transform,
	const Vector4& frameSize,
	const Vector4& viewSize,
	const Vector4& viewOffset,
	float screenOffsetScale,
	const SwfCxTransform& cxform,
	bool maskWrite,
	bool maskIncrement,
	uint8_t maskReference
)
{
	if (!m_vertexRange.vertexBuffer)
		return;

	Matrix44 m(
		transform.e11, transform.e12, transform.e13, 0.0f,
		transform.e21, transform.e22, transform.e23, 0.0f,
		transform.e31, transform.e32, transform.e33, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	render::Shader* shaderSolid[3] = { 0, 0, 0 };
	render::Shader* shaderTextured[3] = { 0, 0, 0 };
	if (!maskWrite)
	{
		if (maskReference == 0)
		{
			shaderSolid[0] = m_shapeResources->m_shaderSolid;
			shaderTextured[0] = m_shapeResources->m_shaderTextured;
			shaderSolid[1] = m_shapeResources->m_shaderSolidCurve;
			shaderTextured[1] = m_shapeResources->m_shaderTexturedCurve;
		}
		else
		{
			shaderSolid[0] = m_shapeResources->m_shaderSolidMask;
			shaderTextured[0] = m_shapeResources->m_shaderTexturedMask;
			shaderSolid[1] = m_shapeResources->m_shaderSolidMaskCurve;
			shaderTextured[1] = m_shapeResources->m_shaderTexturedMaskCurve;
		}
	}
	else
	{
		if (maskIncrement)
		{
			shaderSolid[0] = m_shapeResources->m_shaderIncrementMask;
			shaderSolid[1] = m_shapeResources->m_shaderIncrementMask;
		}
		else
		{
			shaderSolid[0] = m_shapeResources->m_shaderDecrementMask;
			shaderSolid[1] = m_shapeResources->m_shaderIncrementMask;
		}
	}

	shaderSolid[2] = shaderSolid[1];
	shaderTextured[2] = shaderTextured[1];

	for (uint32_t i = 0; i < 3; ++i)
	{
		if (m_renderBatches[i].empty())
			continue;

		if (shaderSolid[i] && (m_batchFlags & BfHaveSolid) != 0)
		{
			render::IProgram* program = shaderSolid[i]->getCurrentProgram();
			if (program)
			{
				render::NullRenderBlock* renderBlockSolid = renderContext->alloc< render::NullRenderBlock >("Flash AccShape; set solid parameters");
				renderBlockSolid->program = program;
				renderBlockSolid->programParams = renderContext->alloc< render::ProgramParameters >();
				renderBlockSolid->programParams->beginParameters(renderContext);
				renderBlockSolid->programParams->setMatrixParameter(m_shapeResources->m_handleTransform, m);
				renderBlockSolid->programParams->setVectorParameter(m_shapeResources->m_handleFrameSize, frameSize);
				renderBlockSolid->programParams->setVectorParameter(m_shapeResources->m_handleViewSize, viewSize);
				renderBlockSolid->programParams->setVectorParameter(m_shapeResources->m_handleViewOffset, viewOffset);
				renderBlockSolid->programParams->setFloatParameter(m_shapeResources->m_handleScreenOffsetScale, screenOffsetScale);
				renderBlockSolid->programParams->setFloatParameter(m_shapeResources->m_handleCurveSign, c_curveSign[i]);
				renderBlockSolid->programParams->setVectorParameter(m_shapeResources->m_handleCxFormMul, Vector4(cxform.red[0], cxform.green[0], cxform.blue[0], cxform.alpha[0]));
				renderBlockSolid->programParams->setVectorParameter(m_shapeResources->m_handleCxFormAdd, Vector4(cxform.red[1], cxform.green[1], cxform.blue[1], cxform.alpha[1]));
				renderBlockSolid->programParams->setStencilReference(maskReference);
				renderBlockSolid->programParams->endParameters(renderContext);
				renderContext->draw(render::RpOverlay, renderBlockSolid);
			}
		}

		if (shaderTextured[i] && (m_batchFlags & BfHaveTextured) != 0)
		{
			render::IProgram* program = shaderTextured[i]->getCurrentProgram();
			if (program)
			{
				render::NullRenderBlock* renderBlockTextured = renderContext->alloc< render::NullRenderBlock >("Flash AccShape; set textured parameters");
				renderBlockTextured->program = program;
				renderBlockTextured->programParams = renderContext->alloc< render::ProgramParameters >();
				renderBlockTextured->programParams->beginParameters(renderContext);
				renderBlockTextured->programParams->setMatrixParameter(m_shapeResources->m_handleTransform, m);
				renderBlockTextured->programParams->setVectorParameter(m_shapeResources->m_handleFrameSize, frameSize);
				renderBlockTextured->programParams->setVectorParameter(m_shapeResources->m_handleViewSize, viewSize);
				renderBlockTextured->programParams->setVectorParameter(m_shapeResources->m_handleViewOffset, viewOffset);
				renderBlockTextured->programParams->setFloatParameter(m_shapeResources->m_handleScreenOffsetScale, screenOffsetScale);
				renderBlockTextured->programParams->setFloatParameter(m_shapeResources->m_handleCurveSign, c_curveSign[i]);
				renderBlockTextured->programParams->setVectorParameter(m_shapeResources->m_handleCxFormMul, Vector4(cxform.red[0], cxform.green[0], cxform.blue[0], cxform.alpha[0]));
				renderBlockTextured->programParams->setVectorParameter(m_shapeResources->m_handleCxFormAdd, Vector4(cxform.red[1], cxform.green[1], cxform.blue[1], cxform.alpha[1]));
				renderBlockTextured->programParams->setStencilReference(maskReference);
				renderBlockTextured->programParams->endParameters(renderContext);
				renderContext->draw(render::RpOverlay, renderBlockTextured);
			}
		}

		for (AlignedVector< RenderBatch >::iterator j = m_renderBatches[i].begin(); j != m_renderBatches[i].end(); ++j)
		{
			if (!j->texture)
			{
				if (shaderSolid[i] && shaderSolid[i]->getCurrentProgram())
				{
					render::NonIndexedRenderBlock* renderBlock = renderContext->alloc< render::NonIndexedRenderBlock >("Flash AccShape; draw solid batch");
					renderBlock->program = shaderSolid[i]->getCurrentProgram();
					renderBlock->vertexBuffer = m_vertexRange.vertexBuffer;
					renderBlock->primitive = j->primitives.type;
					renderBlock->offset = j->primitives.offset;
					renderBlock->count = j->primitives.count;
					renderContext->draw(render::RpOverlay, renderBlock);
				}
			}
			else
			{
				if (shaderTextured[i] && shaderTextured[i]->getCurrentProgram())
				{
					Vector4 textureMatrix0(
						j->textureMatrix.e11, j->textureMatrix.e12, j->textureMatrix.e13, j->textureMatrix.e23
					);
					Vector4 textureMatrix1(
						j->textureMatrix.e21, j->textureMatrix.e22, 0.0f, 0.0f
					);

					render::NonIndexedRenderBlock* renderBlock = renderContext->alloc< render::NonIndexedRenderBlock >("Flash AccShape; draw textured batch");
					renderBlock->program = shaderTextured[i]->getCurrentProgram();
					renderBlock->vertexBuffer = m_vertexRange.vertexBuffer;
					renderBlock->primitive = j->primitives.type;
					renderBlock->offset = j->primitives.offset;
					renderBlock->count = j->primitives.count;
					renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
					renderBlock->programParams->beginParameters(renderContext);
					renderBlock->programParams->setTextureParameter(m_shapeResources->m_handleTexture, j->texture);
					renderBlock->programParams->setVectorParameter(m_shapeResources->m_handleTextureMatrix0, textureMatrix0);
					renderBlock->programParams->setVectorParameter(m_shapeResources->m_handleTextureMatrix1, textureMatrix1);
					renderBlock->programParams->setFloatParameter(m_shapeResources->m_handleTextureClamp, j->textureClamp ? 1.0f : 0.0f);
					renderBlock->programParams->endParameters(renderContext);
					renderContext->draw(render::RpOverlay, renderBlock);
				}
			}
		}
	}
}

void AccShape::preBuild()
{
	if (m_vertexRange.vertexBuffer)
		m_needUpdate = !m_vertexRange.vertexBuffer->isContentValid();
	else
		m_needUpdate = true;
}

	}
}
