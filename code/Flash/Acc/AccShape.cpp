#include <limits>
#include "Core/Log/Log.h"
#include "Core/Math/Bezier2nd.h"
#include "Core/Math/Const.h"
#include "Core/Math/Color4ub.h"
#include "Flash/FlashCanvas.h"
#include "Flash/FlashDictionary.h"
#include "Flash/FlashShape.h"
#include "Flash/FlashBitmap.h"
#include "Flash/Acc/AccShape.h"
#include "Flash/Acc/AccShapeResources.h"
#include "Flash/Acc/Triangulator.h"
#include "Render/ISimpleTexture.h"
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

const static Matrix33 c_textureTS = translate(0.5f, 0.5f) * scale(1.0f / 32768.0f, 1.0f / 32768.0f);

		}

AccShape::AccShape(AccShapeResources* shapeResources)
:	m_shapeResources(shapeResources)
,	m_vertexPool(0)
,	m_batchFlags(0)
,	m_needUpdate(true)
{
}

AccShape::~AccShape()
{
	destroy();
}

bool AccShape::createTesselation(const AlignedVector< Path >& paths)
{
	AlignedVector< Segment > segments;
	Triangulator triangulator;
	Segment s;

	// Create triangles through tessellation.
	m_triangles.resize(0);
	for (AlignedVector< Path >::const_iterator i = paths.begin(); i != paths.end(); ++i)
	{
		const AlignedVector< Vector2 >& points = i->getPoints();
		const AlignedVector< SubPath >& subPaths = i->getSubPaths();

		std::set< uint16_t > fillStyles;
		for (uint32_t j = 0; j < subPaths.size(); ++j)
		{
			const SubPath& sp = subPaths[j];
			if (sp.fillStyle0)
				fillStyles.insert(sp.fillStyle0);
			if (sp.fillStyle1)
				fillStyles.insert(sp.fillStyle1);
		}

		for (std::set< uint16_t >::const_iterator ii = fillStyles.begin(); ii != fillStyles.end(); ++ii)
		{
			for (uint32_t j = 0; j < subPaths.size(); ++j)
			{
				const SubPath& sp = subPaths[j];
				if (sp.fillStyle0 != *ii && sp.fillStyle1 != *ii)
					continue;

				for (AlignedVector< SubPathSegment >::const_iterator k = sp.segments.begin(); k != sp.segments.end(); ++k)
				{
					switch (k->type)
					{
					case SpgtLinear:
						{
							s.v[0] = points[k->pointsOffset];
							s.v[1] = points[k->pointsOffset + 1];
							s.curve = false;
							s.fillStyle0 = sp.fillStyle0;
							s.fillStyle1 = sp.fillStyle1;
							s.lineStyle = sp.lineStyle;
							segments.push_back(s);
						}
						break;

					case SpgtQuadratic:
						{
							s.v[0] = points[k->pointsOffset];
							s.v[1] = points[k->pointsOffset + 2];
							s.c = points[k->pointsOffset + 1];
							s.curve = true;
							s.fillStyle0 = sp.fillStyle0;
							s.fillStyle1 = sp.fillStyle1;
							s.lineStyle = sp.lineStyle;
							segments.push_back(s);
						}
						break;

					default:
						break;
					}
				}
			}

			if (!segments.empty())
			{
				triangulator.triangulate(segments, *ii, m_triangles);
				segments.resize(0);
			}
		}
	}

	// Update shape's bounds from all triangles.
	m_bounds.mn.x = m_bounds.mn.y =  std::numeric_limits< float >::max();
	m_bounds.mx.x = m_bounds.mx.y = -std::numeric_limits< float >::max();
	for (AlignedVector< Triangle >::const_iterator j = m_triangles.begin(); j != m_triangles.end(); ++j)
	{
		for (int k = 0; k < 3; ++k)
		{
			Vector2 pt = j->v[k];
			m_bounds.mn.x = min< float >(m_bounds.mn.x, pt.x);
			m_bounds.mn.y = min< float >(m_bounds.mn.y, pt.y);
			m_bounds.mx.x = max< float >(m_bounds.mx.x, pt.x);
			m_bounds.mx.y = max< float >(m_bounds.mx.y, pt.y);
		}
	}

	m_needUpdate = true;
	return true;
}

bool AccShape::createTesselation(const FlashShape& shape)
{
	const AlignedVector< Path >& paths = shape.getPaths();
	return createTesselation(paths);
}

bool AccShape::createTesselation(const FlashCanvas& canvas)
{
	const AlignedVector< Path >& paths = canvas.getPaths();
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
	// Do we need to update?
	if (m_triangles.empty())
		return false;
	if (!m_needUpdate)
		return true;

	// Free previous vertex range if any.
	if (m_vertexRange.vertexBuffer)
	{
		m_vertexPool->releaseRange(m_vertexRange);
		m_vertexPool = 0;
		m_vertexRange.vertexBuffer = 0;
	}

	m_renderBatches.resize(0);
	m_batchFlags = 0;

	// Allocate vertex range.
	if (!vertexPool->acquireRange(m_triangles.size() * 3, m_vertexRange))
		return false;

	T_ASSERT (m_vertexRange.vertexBuffer);
	m_vertexPool = vertexPool;

	AccShapeVertexPool::Vertex* vertex = static_cast< AccShapeVertexPool::Vertex* >(m_vertexRange.vertexBuffer->lock());
	if (!vertex)
		return false;

	uint32_t vertexOffset = 0;
	Matrix33 textureMatrix;

	for (AlignedVector< Triangle >::const_iterator j = m_triangles.begin(); j != m_triangles.end(); ++j)
	{
		Color4ub color(255, 255, 255, 255);
		AccTextureCache::BitmapRect texture;
		float curveSign = 0.0f;

		if (j->type == TcIn)
			curveSign = 1.0f;
		else if (j->type == TcOut)
			curveSign = -1.0f;

		if (j->fillStyle && j->fillStyle - 1 < uint16_t(fillStyles.size()))
		{
			const FlashFillStyle& style = fillStyles[j->fillStyle - 1];

			const AlignedVector< FlashFillStyle::ColorRecord >& colorRecords = style.getColorRecords();
			if (colorRecords.size() > 1)
			{
				T_ASSERT (textureCache);
				texture = textureCache->getGradientTexture(style);
				textureMatrix = c_textureTS * style.getGradientMatrix().inverse();
				m_batchFlags |= BfHaveTextured;
			}
			else if (colorRecords.size() == 1)
			{
				color.r = colorRecords.front().color.red;
				color.g = colorRecords.front().color.green;
				color.b = colorRecords.front().color.blue;
				color.a = colorRecords.front().color.alpha;
				m_batchFlags |= BfHaveSolid;
			}

			const FlashBitmap* bitmap = dictionary.getBitmap(style.getFillBitmap());
			if (bitmap)
			{
				T_ASSERT (textureCache);
				texture = textureCache->getBitmapTexture(*bitmap);
				textureMatrix = scale(1.0f / bitmap->getWidth(), 1.0f / bitmap->getHeight()) * style.getFillBitmapMatrix().inverse();
				m_batchFlags |= BfHaveTextured;
			}
		}

		if (m_renderBatches.empty() || m_renderBatches.back().texture != texture)
		{
			m_renderBatches.push_back(RenderBatch());
			m_renderBatches.back().primitives.setNonIndexed(render::PtTriangles, vertexOffset, 0);
			m_renderBatches.back().texture = texture;
			m_renderBatches.back().textureMatrix = textureMatrix;
		}

		for (int k = 0; k < 3; ++k)
		{
			Vector2 P = j->v[k];
			vertex->pos[0] = P.x;
			vertex->pos[1] = P.y;
			vertex->uv[0] = c_controlPoints[k][0];
			vertex->uv[1] = c_controlPoints[k][1];
			vertex->uv[2] = curveSign;
			vertex->color[0] = color.r;
			vertex->color[1] = color.g;
			vertex->color[2] = color.b;
			vertex->color[3] = color.a;
			vertex++;
		}

		m_renderBatches.back().primitives.count++;
		vertexOffset += 3;
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

	m_triangles.clear();
	m_renderBatches.clear();
}

void AccShape::render(
	render::RenderContext* renderContext,
	const Matrix33& transform,
	const Vector4& frameSize,
	const Vector4& viewOffset,
	float screenOffsetScale,
	const SwfCxTransform& cxform,
	bool maskWrite,
	bool maskIncrement,
	uint8_t maskReference,
	uint8_t blendMode
)
{
	if (m_renderBatches.empty() || !m_vertexRange.vertexBuffer)
		return;

	Matrix44 m(
		transform.e11, transform.e12, transform.e13, 0.0f,
		transform.e21, transform.e22, transform.e23, 0.0f,
		transform.e31, transform.e32, transform.e33, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	render::Shader* shaderSolid = 0;
	render::Shader* shaderTextured = 0;
	if (!maskWrite)
	{
		if (maskReference == 0)
		{
			shaderSolid = m_shapeResources->m_shaderSolid;
			shaderTextured = m_shapeResources->m_shaderTextured;
		}
		else
		{
			shaderSolid = m_shapeResources->m_shaderSolidMask;
			shaderTextured = m_shapeResources->m_shaderTexturedMask;
		}

		shaderSolid->setTechnique(m_shapeResources->m_handleTechniques[blendMode]);
		shaderTextured->setTechnique(m_shapeResources->m_handleTechniques[blendMode]);
	}
	else
	{
		if (maskIncrement)
			shaderSolid = m_shapeResources->m_shaderIncrementMask;
		else
			shaderSolid = m_shapeResources->m_shaderDecrementMask;
	}

	if (shaderSolid && (m_batchFlags & BfHaveSolid) != 0)
	{
		render::IProgram* program = shaderSolid->getCurrentProgram();
		if (program)
		{
			render::NullRenderBlock* renderBlockSolid = renderContext->alloc< render::NullRenderBlock >("Flash AccShape; set solid parameters");
			renderBlockSolid->program = program;
			renderBlockSolid->programParams = renderContext->alloc< render::ProgramParameters >();
			renderBlockSolid->programParams->beginParameters(renderContext);
			renderBlockSolid->programParams->setMatrixParameter(m_shapeResources->m_handleTransform, m);
			renderBlockSolid->programParams->setVectorParameter(m_shapeResources->m_handleFrameSize, frameSize);
			renderBlockSolid->programParams->setVectorParameter(m_shapeResources->m_handleViewOffset, viewOffset);
			renderBlockSolid->programParams->setFloatParameter(m_shapeResources->m_handleScreenOffsetScale, screenOffsetScale);
			renderBlockSolid->programParams->setVectorParameter(m_shapeResources->m_handleCxFormMul, Vector4(cxform.red[0], cxform.green[0], cxform.blue[0], cxform.alpha[0]));
			renderBlockSolid->programParams->setVectorParameter(m_shapeResources->m_handleCxFormAdd, Vector4(cxform.red[1], cxform.green[1], cxform.blue[1], cxform.alpha[1]));
			renderBlockSolid->programParams->setStencilReference(maskReference);
			renderBlockSolid->programParams->endParameters(renderContext);
			renderContext->draw(render::RpOverlay, renderBlockSolid);
		}
	}

	if (shaderTextured && (m_batchFlags & BfHaveTextured) != 0)
	{
		render::IProgram* program = shaderTextured->getCurrentProgram();
		if (program)
		{
			render::NullRenderBlock* renderBlockTextured = renderContext->alloc< render::NullRenderBlock >("Flash AccShape; set textured parameters");
			renderBlockTextured->program = program;
			renderBlockTextured->programParams = renderContext->alloc< render::ProgramParameters >();
			renderBlockTextured->programParams->beginParameters(renderContext);
			renderBlockTextured->programParams->setMatrixParameter(m_shapeResources->m_handleTransform, m);
			renderBlockTextured->programParams->setVectorParameter(m_shapeResources->m_handleFrameSize, frameSize);
			renderBlockTextured->programParams->setVectorParameter(m_shapeResources->m_handleViewOffset, viewOffset);
			renderBlockTextured->programParams->setFloatParameter(m_shapeResources->m_handleScreenOffsetScale, screenOffsetScale);
			renderBlockTextured->programParams->setVectorParameter(m_shapeResources->m_handleCxFormMul, Vector4(cxform.red[0], cxform.green[0], cxform.blue[0], cxform.alpha[0]));
			renderBlockTextured->programParams->setVectorParameter(m_shapeResources->m_handleCxFormAdd, Vector4(cxform.red[1], cxform.green[1], cxform.blue[1], cxform.alpha[1]));
			renderBlockTextured->programParams->setStencilReference(maskReference);
			renderBlockTextured->programParams->endParameters(renderContext);
			renderContext->draw(render::RpOverlay, renderBlockTextured);
		}
	}

	for (AlignedVector< RenderBatch >::iterator j = m_renderBatches.begin(); j != m_renderBatches.end(); ++j)
	{
		if (!j->texture.texture)
		{
			if (shaderSolid && shaderSolid->getCurrentProgram())
			{
				render::NonIndexedRenderBlock* renderBlock = renderContext->alloc< render::NonIndexedRenderBlock >("Flash AccShape; draw solid batch");
				renderBlock->program = shaderSolid->getCurrentProgram();
				renderBlock->vertexBuffer = m_vertexRange.vertexBuffer;
				renderBlock->primitive = j->primitives.type;
				renderBlock->offset = j->primitives.offset;
				renderBlock->count = j->primitives.count;
				renderContext->draw(render::RpOverlay, renderBlock);
			}
		}
		else
		{
			if (shaderTextured && shaderTextured->getCurrentProgram())
			{
				Vector4 textureMatrix0(
					j->textureMatrix.e11, j->textureMatrix.e12, j->textureMatrix.e13, j->textureMatrix.e23
				);
				Vector4 textureMatrix1(
					j->textureMatrix.e21, j->textureMatrix.e22, 0.0f, 0.0f
				);

				render::NonIndexedRenderBlock* renderBlock = renderContext->alloc< render::NonIndexedRenderBlock >("Flash AccShape; draw textured batch");
				renderBlock->program = shaderTextured->getCurrentProgram();
				renderBlock->vertexBuffer = m_vertexRange.vertexBuffer;
				renderBlock->primitive = j->primitives.type;
				renderBlock->offset = j->primitives.offset;
				renderBlock->count = j->primitives.count;
				renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
				renderBlock->programParams->beginParameters(renderContext);
				renderBlock->programParams->setTextureParameter(m_shapeResources->m_handleTexture, j->texture.texture);
				renderBlock->programParams->setFloatParameter(m_shapeResources->m_handleTextureClamp, j->texture.clamp ? 1.0f : 0.0f);
				renderBlock->programParams->setVectorParameter(m_shapeResources->m_handleTextureRect, Vector4::loadUnaligned(j->texture.rect));
				renderBlock->programParams->setVectorParameter(m_shapeResources->m_handleTextureMatrix0, textureMatrix0);
				renderBlock->programParams->setVectorParameter(m_shapeResources->m_handleTextureMatrix1, textureMatrix1);
				renderBlock->programParams->endParameters(renderContext);
				renderContext->draw(render::RpOverlay, renderBlock);
			}
		}
	}
}

	}
}
