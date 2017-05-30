/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <limits>
#include "Core/Log/Log.h"
#include "Core/Math/Bezier2nd.h"
#include "Core/Math/Const.h"
#include "Core/Math/Color4ub.h"
#include "Flash/ColorTransform.h"
#include "Flash/Canvas.h"
#include "Flash/Dictionary.h"
#include "Flash/Shape.h"
#include "Flash/Bitmap.h"
#include "Flash/Triangulator.h"
#include "Flash/Acc/AccBitmapRect.h"
#include "Flash/Acc/AccGradientCache.h"
#include "Flash/Acc/AccShape.h"
#include "Flash/Acc/AccShapeResources.h"
#include "Flash/Acc/AccTextureCache.h"
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

const uint8_t c_cpNegOne = 0;
const uint8_t c_cpZero = 127;
const uint8_t c_cpHalf = 191;
const uint8_t c_cpOne = 255;

const uint8_t c_controlPoints[3][2] =
{
	{ c_cpZero, c_cpZero },
	{ c_cpHalf, c_cpZero },
	{ c_cpOne, c_cpOne }
};

const static Matrix33 c_textureTS = translate(0.5f, 0.5f) * scale(1.0f / 32768.0f, 1.0f / 32768.0f);

		}

AccShape::AccShape(AccShapeResources* shapeResources)
:	m_shapeResources(shapeResources)
,	m_vertexPool(0)
,	m_batchFlags(0)
{
}

AccShape::~AccShape()
{
	destroy();
}

bool AccShape::createFromPaths(
	AccShapeVertexPool* vertexPool,
	AccGradientCache* gradientCache,
	AccTextureCache* textureCache,
	const Dictionary& dictionary,
	const AlignedVector< FillStyle >& fillStyles,
	const AlignedVector< LineStyle >& lineStyles,
	const AlignedVector< Path >& paths,
	bool oddEven
)
{
	AlignedVector< Triangle > triangles;
	AlignedVector< Segment > segments;
	Triangulator triangulator;
	Segment s;

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
				uint32_t from = triangles.size();

				triangulator.triangulate(segments, *ii, oddEven, triangles);
				segments.resize(0);

				uint32_t to = triangles.size();

				// Transform each new triangle with path's transform.
				for (uint32_t ti = from; ti < to; ++ti)
				{
					triangles[ti].v[0] = i->getTransform() * triangles[ti].v[0];
					triangles[ti].v[1] = i->getTransform() * triangles[ti].v[1];
					triangles[ti].v[2] = i->getTransform() * triangles[ti].v[2];
				}
			}
		}
	}

	return createFromTriangles(
		vertexPool,
		gradientCache,
		textureCache,
		dictionary,
		fillStyles,
		lineStyles,
		triangles
	);
}

bool AccShape::createFromTriangles(
	AccShapeVertexPool* vertexPool,
	AccGradientCache* gradientCache,
	AccTextureCache* textureCache,
	const Dictionary& dictionary,
	const AlignedVector< FillStyle >& fillStyles,
	const AlignedVector< LineStyle >& lineStyles,
	const AlignedVector< Triangle >& triangles
)
{
	// Update shape's bounds from all triangles.
	m_bounds.mn.x = m_bounds.mn.y =  std::numeric_limits< float >::max();
	m_bounds.mx.x = m_bounds.mx.y = -std::numeric_limits< float >::max();
	for (AlignedVector< Triangle >::const_iterator j = triangles.begin(); j != triangles.end(); ++j)
	{
		for (int k = 0; k < 3; ++k)
		{
			const Vector2& pt = j->v[k];
			m_bounds.mn.x = min< float >(m_bounds.mn.x, pt.x);
			m_bounds.mn.y = min< float >(m_bounds.mn.y, pt.y);
			m_bounds.mx.x = max< float >(m_bounds.mx.x, pt.x);
			m_bounds.mx.y = max< float >(m_bounds.mx.y, pt.y);
		}
	}

	m_renderBatches.resize(0);
	m_batchFlags = 0;

	if (!triangles.empty())
	{
		// Allocate vertex range.
		T_ASSERT (!m_vertexRange.vertexBuffer)
		if (!vertexPool->acquireRange(triangles.size() * 3, m_vertexRange))
			return false;
		T_ASSERT (m_vertexRange.vertexBuffer);
		m_vertexPool = vertexPool;

		AccShapeVertexPool::Vertex* vertex = static_cast< AccShapeVertexPool::Vertex* >(m_vertexRange.vertexBuffer->lock());
		if (!vertex)
			return false;

		uint32_t vertexOffset = 0;
		Matrix33 textureMatrix;
		uint16_t lastFillStyle = 0;
		Color4ub color(255, 255, 255, 255);
		Ref< AccBitmapRect > texture;
		bool textureClamp = false;

		for (AlignedVector< Triangle >::const_iterator j = triangles.begin(); j != triangles.end(); ++j)
		{
			T_ASSERT (j->fillStyle);

			uint8_t curveSign = c_cpZero;
			if (j->type == TcIn)
				curveSign = c_cpOne;
			else if (j->type == TcOut)
				curveSign = c_cpNegOne;

			if (
				!fillStyles.empty() &&
				j->fillStyle != lastFillStyle
			)
			{
				color = Color4ub(255, 255, 255, 255);
				texture = 0;

				const FillStyle& style = fillStyles[j->fillStyle - 1];

				// Convert colors, solid or gradients.
				const AlignedVector< FillStyle::ColorRecord >& colorRecords = style.getColorRecords();
				if (colorRecords.size() > 1)
				{
					texture = gradientCache->getGradientTexture(style);
					textureMatrix = c_textureTS * style.getGradientMatrix().inverse();
					textureClamp = true;
					m_batchFlags |= BfHaveTextured;
				}
				else if (colorRecords.size() == 1)
				{
					color = colorRecords.front().color.toColor4ub();
					m_batchFlags |= BfHaveSolid;
				}

				// Convert bitmaps.
				const Bitmap* bitmap = dictionary.getBitmap(style.getFillBitmap());
				if (bitmap)
				{
					texture = textureCache->getBitmapTexture(*bitmap);
					textureMatrix = scale(1.0f / bitmap->getWidth(), 1.0f / bitmap->getHeight()) * style.getFillBitmapMatrix().inverse();
					textureClamp = !style.getFillBitmapRepeat();
					m_batchFlags |= BfHaveTextured;
				}

				lastFillStyle = j->fillStyle;
			}

			if (m_renderBatches.empty() || m_renderBatches.back().texture != texture || m_renderBatches.back().textureClamp != textureClamp)
			{
				m_renderBatches.push_back(RenderBatch());
				m_renderBatches.back().primitives.setNonIndexed(render::PtTriangles, vertexOffset, 0);
				m_renderBatches.back().texture = texture;
				m_renderBatches.back().textureClamp = textureClamp;
			}

			if (texture)
			{
				for (int k = 0; k < 3; ++k)
				{
					const Vector2& P = j->v[k];
					Vector2 tc = textureMatrix * P;

					vertex->pos[0] = P.x;
					vertex->pos[1] = P.y;
					vertex->curvature[0] = c_controlPoints[k][0];
					vertex->curvature[1] = c_controlPoints[k][1];
					vertex->curvature[2] = curveSign;
					vertex->texCoord[0] = tc.x;
					vertex->texCoord[1] = tc.y;
					vertex->texRect[0] = texture->rect[0];
					vertex->texRect[1] = texture->rect[1];
					vertex->texRect[2] = texture->rect[2];
					vertex->texRect[3] = texture->rect[3];
					vertex->color[0] = color.r;
					vertex->color[1] = color.g;
					vertex->color[2] = color.b;
					vertex->color[3] = color.a;

					vertex++;
				}
			}
			else
			{
				for (int k = 0; k < 3; ++k)
				{
					const Vector2& P = j->v[k];

					vertex->pos[0] = P.x;
					vertex->pos[1] = P.y;
					vertex->curvature[0] = c_controlPoints[k][0];
					vertex->curvature[1] = c_controlPoints[k][1];
					vertex->curvature[2] = curveSign;
					vertex->color[0] = color.r;
					vertex->color[1] = color.g;
					vertex->color[2] = color.b;
					vertex->color[3] = color.a;

					vertex++;
				}
			}

			m_renderBatches.back().primitives.count++;
			vertexOffset += 3;
		}

		m_vertexRange.vertexBuffer->unlock();

		// Some shapes doesn't expose styles, such as glyphs, but are
		// assumed to be solids.
		if (!m_batchFlags)
			m_batchFlags |= BfHaveSolid;
	}

	return true;
}

bool AccShape::createFromShape(
	AccShapeVertexPool* vertexPool,
	AccGradientCache* gradientCache,
	AccTextureCache* textureCache,
	const Dictionary& dictionary,
	const Shape& shape
)
{
	if (!shape.getTriangles().empty())
		return createFromTriangles(
			vertexPool,
			gradientCache,
			textureCache,
			dictionary,
			shape.getFillStyles(),
			shape.getLineStyles(),
			shape.getTriangles()
		);
	else if (!shape.getPaths().empty())
		return createFromPaths(
			vertexPool,
			gradientCache,
			textureCache,
			dictionary,
			shape.getFillStyles(),
			shape.getLineStyles(),
			shape.getPaths(),
			false
		);
	else
		return false;
}

bool AccShape::createFromGlyph(
	AccShapeVertexPool* vertexPool,
	AccGradientCache* gradientCache,
	AccTextureCache* textureCache,
	const Dictionary& dictionary,
	const Shape& shape
)
{
	if (!shape.getTriangles().empty())
		return createFromTriangles(
			vertexPool,
			gradientCache,
			textureCache,
			dictionary,
			shape.getFillStyles(),
			shape.getLineStyles(),
			shape.getTriangles()
		);
	else if (!shape.getPaths().empty())
		return createFromPaths(
			vertexPool,
			gradientCache,
			textureCache,
			dictionary,
			shape.getFillStyles(),
			shape.getLineStyles(),
			shape.getPaths(),
			true
		);
	else
		return false;
}

bool AccShape::createFromCanvas(
	AccShapeVertexPool* vertexPool,
	AccGradientCache* gradientCache,
	AccTextureCache* textureCache,
	const Canvas& canvas
)
{
	return createFromPaths(
		vertexPool,
		gradientCache,
		textureCache,
		canvas.getDictionary(),
		canvas.getFillStyles(),
		canvas.getLineStyles(),
		canvas.getPaths(),
		false
	);
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

	m_renderBatches.clear();
}

void AccShape::render(
	render::RenderContext* renderContext,
	const Matrix33& transform,
	const Vector4& frameBounds,
	const Vector4& frameTransform,
	const ColorTransform& cxform,
	bool maskWrite,
	bool maskIncrement,
	uint8_t maskReference,
	uint8_t blendMode
)
{
	if (m_renderBatches.empty() || !m_vertexRange.vertexBuffer)
		return;

	const Matrix44 m(
		transform.e11, transform.e12, transform.e13, 0.0f,
		transform.e21, transform.e22, transform.e23, 0.0f,
		transform.e31, transform.e32, transform.e33, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	render::Shader* shaderSolid = 0;
	render::Shader* shaderTextured = 0;
	if (!maskWrite)
	{
		shaderSolid = m_shapeResources->m_shaderSolid;
		shaderSolid->setTechnique(m_shapeResources->m_handleTechniques[blendMode]);

		shaderTextured = m_shapeResources->m_shaderTextured;
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
			renderBlockSolid->programParams->setVectorParameter(m_shapeResources->m_handleFrameBounds, frameBounds);
			renderBlockSolid->programParams->setVectorParameter(m_shapeResources->m_handleFrameTransform, frameTransform);
			renderBlockSolid->programParams->setVectorParameter(m_shapeResources->m_handleCxFormMul, cxform.mul);
			renderBlockSolid->programParams->setVectorParameter(m_shapeResources->m_handleCxFormAdd, cxform.add);
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
			renderBlockTextured->programParams->setVectorParameter(m_shapeResources->m_handleFrameBounds, frameBounds);
			renderBlockTextured->programParams->setVectorParameter(m_shapeResources->m_handleFrameTransform, frameTransform);
			renderBlockTextured->programParams->setVectorParameter(m_shapeResources->m_handleCxFormMul, cxform.mul);
			renderBlockTextured->programParams->setVectorParameter(m_shapeResources->m_handleCxFormAdd, cxform.add);
			renderBlockTextured->programParams->setStencilReference(maskReference);
			renderBlockTextured->programParams->endParameters(renderContext);
			renderContext->draw(render::RpOverlay, renderBlockTextured);
		}
	}

	for (AlignedVector< RenderBatch >::iterator j = m_renderBatches.begin(); j != m_renderBatches.end(); ++j)
	{
		if (!j->texture)
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
				render::NonIndexedRenderBlock* renderBlock = renderContext->alloc< render::NonIndexedRenderBlock >("Flash AccShape; draw textured batch");
				renderBlock->program = shaderTextured->getCurrentProgram();
				renderBlock->vertexBuffer = m_vertexRange.vertexBuffer;
				renderBlock->primitive = j->primitives.type;
				renderBlock->offset = j->primitives.offset;
				renderBlock->count = j->primitives.count;
				renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
				renderBlock->programParams->beginParameters(renderContext);
				renderBlock->programParams->setTextureParameter(m_shapeResources->m_handleTexture, j->texture->texture);
				renderBlock->programParams->setFloatParameter(m_shapeResources->m_handleTextureClamp, j->textureClamp ? 1.0f : 0.0f);
				renderBlock->programParams->endParameters(renderContext);
				renderContext->draw(render::RpOverlay, renderBlock);
			}
		}
	}
}

	}
}
