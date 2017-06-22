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
#include "Core/Math/Half.h"
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
#include "Render/IRenderSystem.h"
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
	BfHaveTextured = 2,
	BfHaveLines = 4
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

AccShape::AccShape(render::IRenderSystem* renderSystem, const AccShapeResources* shapeResources, AccShapeVertexPool* fillVertexPool, AccShapeVertexPool* lineVertexPool)
:	m_renderSystem(renderSystem)
,	m_shapeResources(shapeResources)
,	m_fillVertexPool(fillVertexPool)
,	m_lineVertexPool(lineVertexPool)
,	m_batchFlags(0)
{
}

AccShape::~AccShape()
{
	destroy();
}

bool AccShape::createFromTriangles(
	AccGradientCache* gradientCache,
	AccTextureCache* textureCache,
	const Dictionary& dictionary,
	const AlignedVector< FillStyle >& fillStyles,
	const AlignedVector< LineStyle >& lineStyles,
	const AlignedVector< Triangle >& triangles,
	const AlignedVector< Line >& lines
)
{
	Aabb2 triangleBounds;
	Aabb2 lineBounds;

	// Update shape's bounds from all triangles and lines.
	for (AlignedVector< Triangle >::const_iterator j = triangles.begin(); j != triangles.end(); ++j)
	{
		for (int32_t k = 0; k < 3; ++k)
			triangleBounds.contain(j->v[k]);
	}

	for (AlignedVector< Line >::const_iterator j = lines.begin(); j != lines.end(); ++j)
	{
		const LineStyle& lineStyle = lineStyles[j->lineStyle - 1];
		float width = lineStyle.getLineWidth() / 2.0f;

		for (int32_t k = 0; k < 2; ++k)
		{
			const Vector2& pt = j->v[k];
			lineBounds.contain(pt - width);
			lineBounds.contain(pt + width);
		}
	}

	m_bounds = triangleBounds;
	m_bounds.contain(lineBounds);
	m_fillRenderBatches.resize(0);
	m_lineRenderBatches.resize(0);
	m_batchFlags = 0;

	if (!lines.empty())
	{
		struct LineCluster
		{
			Aabb2 bounds;
			AlignedVector< int32_t > lines;
		};

		const int32_t c_maxSubDivide = 4;
		const int32_t c_maxLinesPerCluster = 4;

		// Collect unique line styles.
		std::set< int32_t > uniqueLineStyles;
		for (auto line : lines)
			uniqueLineStyles.insert(line.lineStyle);

		// Generate cluster and batches for each line style.
		for (auto lineStyleIndex : uniqueLineStyles)
		{
			const LineStyle& lineStyle = lineStyles[lineStyleIndex - 1];
			float width = lineStyle.getLineWidth() / 2.0f;

			// Create root clusters.
			AlignedVector< LineCluster > clusters;
			clusters.push_back();
			clusters.back().bounds = lineBounds;
			for (int32_t i = 0; i < lines.size(); ++i)
			{
				if (lines[i].lineStyle == lineStyleIndex)
					clusters.back().lines.push_back(i);
			}

			// \TBD Check if a single line occupy entire cluster... possibly when subdividing deep...

			// Sub-divide clusters with too many lines.
			for (int32_t it = 0; it < c_maxSubDivide; ++it)
			{
				int32_t nsub = 0;
				int32_t size = clusters.size();
				for (int32_t i = 0; i < size; ++i)
				{
					if (clusters[i].lines.size() > c_maxLinesPerCluster)
					{
						const int32_t c_subSize = 2;
						for (int32_t iy = 0; iy < c_subSize; ++iy)
						{
							for (int32_t ix = 0; ix < c_subSize; ++ix)
							{
								Vector2 dxy(1.0f / float(c_subSize), 1.0f / float(c_subSize));
								Vector2 fxy = Vector2(ix, iy) * dxy;

								Aabb2 cell;
								cell.mn = clusters[i].bounds.mn + (clusters[i].bounds.mx - clusters[i].bounds.mn) * fxy;
								cell.mx = clusters[i].bounds.mn + (clusters[i].bounds.mx - clusters[i].bounds.mn) * (fxy + dxy);

								bool foundLine = false;
								for (int32_t j = 0; j < clusters[i].lines.size(); ++j)
								{
									const Line& line = lines[clusters[i].lines[j]];

									Aabb2 cellWithMargin = cell;
									cellWithMargin.mn -= Vector2(width, width);
									cellWithMargin.mx += Vector2(width, width);

									float d;
									if (!cellWithMargin.intersectSegment(line.v[0], line.v[1], d))
										continue;

									if (!foundLine)
									{
										clusters.push_back();
										foundLine = true;
									}

									LineCluster& cs = clusters.back();
									cs.bounds.contain(cell);
									cs.lines.push_back(clusters[i].lines[j]);
								}
							}
						}

						clusters[i].lines.clear();
						++nsub;
					}
				}

				AlignedVector< LineCluster >::iterator i = std::remove_if(clusters.begin(), clusters.end(), [](LineCluster& c) {
					return c.lines.empty();
				});
				clusters.erase(i, clusters.end());

				if (nsub <= 0)
					break;
			}

			// Shrink bounds of clusters.
			for (int32_t i = 0; i < clusters.size(); ++i)
			{
				LineCluster& c = clusters[i];
				Aabb2 b;

				for (int32_t j = 0; j < c.lines.size(); ++j)
				{
					const Line& line = lines[c.lines[j]];

					//Aabb2 clip = c.bounds;
					//clip.mn -= Vector2(width, width);
					//clip.mx += Vector2(width, width);

					//float d;
					//if (clip.intersectSegment(line.v[0], line.v[1], d))
					//{
					//}

					b.contain(line.v[0], width);
					b.contain(line.v[1], width);
				}

				if (b.mn.x > c.bounds.mn.x)
					c.bounds.mn.x = b.mn.x;
				if (b.mn.y > c.bounds.mn.y)
					c.bounds.mn.y = b.mn.y;
				if (b.mx.x < c.bounds.mx.x)
					c.bounds.mx.x = b.mx.x;
				if (b.mx.y < c.bounds.mx.y)
					c.bounds.mx.y = b.mx.y;
			}

			// Generate line batches from clusters.
			AccShapeVertexPool::Range vertexRange;
			if (!m_lineVertexPool->acquireRange(clusters.size() * 2 * 3, vertexRange))
				return false;

			LineVertex* vertex = static_cast< LineVertex* >(vertexRange.vertexBuffer->lock());
			if (!vertex)
				return false;

			int32_t lineDataSize = 0;
			for (auto c : clusters)
				lineDataSize += min< int32_t >(c.lines.size(), c_maxLinesPerCluster);

			render::SimpleTextureCreateDesc stcd;
			stcd.width = lineDataSize;
			stcd.height = 1;
			stcd.mipCount = 1;
			stcd.format = render::TfR16G16B16A16F;
			stcd.sRGB = false;
			stcd.immutable = false;

			Ref< render::ISimpleTexture > lineTexture = m_renderSystem->createSimpleTexture(stcd);
			if (!lineTexture)
				return false;

			render::ITexture::Lock lock;
			if (!lineTexture->lock(0, lock))
				return false;

			half_t* lineData = static_cast< half_t* >(lock.bits);
			int32_t lineDataOffset = 0;

			m_lineRenderBatches.push_back();
			m_lineRenderBatches.back().vertexRange = vertexRange;
			m_lineRenderBatches.back().lineTexture = lineTexture;
			m_lineRenderBatches.back().primitives.setNonIndexed(render::PtTriangles, 0, clusters.size() * 2);
			m_lineRenderBatches.back().color = lineStyle.getLineColor();
			m_lineRenderBatches.back().width = width;

			for (int32_t i = 0; i < clusters.size(); ++i)
			{
				const LineCluster& c = clusters[i];

				Vector2 q[4];
				c.bounds.getExtents(q);

				const int t[] = { 0, 1, 3, 2, 3, 1 };
				for (int j = 0; j < sizeof_array(t); ++j)
				{
					vertex->pos[0] = q[t[j]].x;
					vertex->pos[1] = q[t[j]].y;
					vertex->lineOffset = float(lineDataOffset);
					vertex++;
				}

				for (int32_t j = 0; j < min< int32_t >(c.lines.size(), c_maxLinesPerCluster); ++j)
				{
					const Line& line = lines[c.lines[j]];
					half_t* p = &lineData[lineDataOffset * 4];

					*p++ = floatToHalf(line.v[0].x);
					*p++ = floatToHalf(line.v[0].y);
					*p++ = floatToHalf(line.v[1].x);
					*p++ = floatToHalf(line.v[1].y);

					lineDataOffset++;
				}
			}

			T_ASSERT (lineDataOffset == lineDataSize);

			lineTexture->unlock(0);
			vertexRange.vertexBuffer->unlock();
		}

		m_batchFlags |= BfHaveLines;
	}

	if (!triangles.empty())
	{
		uint32_t vertexOffset = 0;
		Matrix33 textureMatrix;
		uint16_t lastFillStyle = 0;
		Color4ub color(255, 255, 255, 255);
		Ref< AccBitmapRect > texture;
		bool textureClamp = false;

		if (!m_fillVertexPool->acquireRange(triangles.size() * 3, m_fillVertexRange))
			return false;

		FillVertex* vertex = static_cast< FillVertex* >(m_fillVertexRange.vertexBuffer->lock());
		if (!vertex)
			return false;

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

			if (m_fillRenderBatches.empty() || m_fillRenderBatches.back().texture != texture || m_fillRenderBatches.back().textureClamp != textureClamp)
			{
				m_fillRenderBatches.push_back(FillRenderBatch());
				m_fillRenderBatches.back().primitives.setNonIndexed(render::PtTriangles, vertexOffset, 0);
				m_fillRenderBatches.back().texture = texture;
				m_fillRenderBatches.back().textureClamp = textureClamp;
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

			m_fillRenderBatches.back().primitives.count++;
			vertexOffset += 3;
		}

		m_fillVertexRange.vertexBuffer->unlock();
	}

	// Some shapes doesn't expose styles, such as glyphs, but are
	// assumed to be solids.
	if (!m_batchFlags)
		m_batchFlags |= BfHaveSolid;

	return true;
}

bool AccShape::createFromShape(
	AccGradientCache* gradientCache,
	AccTextureCache* textureCache,
	const Dictionary& dictionary,
	const Shape& shape
)
{
	if (!shape.getTriangles().empty())
		return createFromTriangles(
			gradientCache,
			textureCache,
			dictionary,
			shape.getFillStyles(),
			shape.getLineStyles(),
			shape.getTriangles(),
			shape.getLines()
		);
	else if (!shape.getPaths().empty())
	{
		AlignedVector< Triangle > triangles;
		AlignedVector< Line > lines;
		shape.triangulate(false, triangles, lines);
		return createFromTriangles(
			gradientCache,
			textureCache,
			dictionary,
			shape.getFillStyles(),
			shape.getLineStyles(),
			triangles,
			lines
		);
	}
	else
		return false;
}

bool AccShape::createFromGlyph(
	AccGradientCache* gradientCache,
	AccTextureCache* textureCache,
	const Dictionary& dictionary,
	const Shape& shape
)
{
	if (!shape.getTriangles().empty())
		return createFromTriangles(
			gradientCache,
			textureCache,
			dictionary,
			shape.getFillStyles(),
			shape.getLineStyles(),
			shape.getTriangles(),
			shape.getLines()
		);
	else if (!shape.getPaths().empty())
	{
		AlignedVector< Triangle > triangles;
		AlignedVector< Line > lines;
		shape.triangulate(false, triangles, lines);
		return createFromTriangles(
			gradientCache,
			textureCache,
			dictionary,
			shape.getFillStyles(),
			shape.getLineStyles(),
			triangles,
			lines
		);
	}
	else
		return false;
}

bool AccShape::createFromCanvas(
	AccGradientCache* gradientCache,
	AccTextureCache* textureCache,
	const Canvas& canvas
)
{
	AlignedVector< Triangle > triangles;
	AlignedVector< Line > lines;
	canvas.triangulate(false, triangles, lines);
	return createFromTriangles(
		gradientCache,
		textureCache,
		canvas.getDictionary(),
		canvas.getFillStyles(),
		canvas.getLineStyles(),
		triangles,
		lines
	);
}

void AccShape::destroy()
{
	m_shapeResources = 0;

	if (m_fillVertexPool)
	{
		m_fillVertexPool->releaseRange(m_fillVertexRange);
		m_fillVertexPool = 0;
	}

	m_fillRenderBatches.clear();
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
	const Matrix44 m(
		transform.e11, transform.e12, transform.e13, 0.0f,
		transform.e21, transform.e22, transform.e23, 0.0f,
		transform.e31, transform.e32, transform.e33, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	render::Shader* shaderSolid = 0;
	render::Shader* shaderTextured = 0;
	render::Shader* shaderLine = 0;

	if (!maskWrite)
	{
		shaderSolid = m_shapeResources->m_shaderSolid;
		shaderSolid->setTechnique(m_shapeResources->m_handleTechniques[blendMode]);

		shaderTextured = m_shapeResources->m_shaderTextured;
		shaderTextured->setTechnique(m_shapeResources->m_handleTechniques[blendMode]);

		shaderLine = m_shapeResources->m_shaderLine;
		shaderLine->setTechnique(m_shapeResources->m_handleTechniques[blendMode]);
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

	if (shaderLine && (m_batchFlags & BfHaveLines) != 0)
	{
		render::IProgram* program = shaderLine->getCurrentProgram();
		if (program)
		{
			render::NullRenderBlock* renderBlockLine = renderContext->alloc< render::NullRenderBlock >("Flash AccShape; set line parameters");
			renderBlockLine->program = program;
			renderBlockLine->programParams = renderContext->alloc< render::ProgramParameters >();
			renderBlockLine->programParams->beginParameters(renderContext);
			renderBlockLine->programParams->setMatrixParameter(m_shapeResources->m_handleTransform, m);
			renderBlockLine->programParams->setVectorParameter(m_shapeResources->m_handleFrameBounds, frameBounds);
			renderBlockLine->programParams->setVectorParameter(m_shapeResources->m_handleFrameTransform, frameTransform);
			renderBlockLine->programParams->setVectorParameter(m_shapeResources->m_handleCxFormMul, cxform.mul);
			renderBlockLine->programParams->setVectorParameter(m_shapeResources->m_handleCxFormAdd, cxform.add);
			renderBlockLine->programParams->setStencilReference(maskReference);
			renderBlockLine->programParams->endParameters(renderContext);
			renderContext->draw(render::RpOverlay, renderBlockLine);
		}
	}

	for (AlignedVector< FillRenderBatch >::iterator j = m_fillRenderBatches.begin(); j != m_fillRenderBatches.end(); ++j)
	{
		if (!j->texture)
		{
			if (shaderSolid && shaderSolid->getCurrentProgram())
			{
				render::NonIndexedRenderBlock* renderBlock = renderContext->alloc< render::NonIndexedRenderBlock >("Flash AccShape; draw solid batch");
				renderBlock->program = shaderSolid->getCurrentProgram();
				renderBlock->vertexBuffer = m_fillVertexRange.vertexBuffer;
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
				renderBlock->vertexBuffer = m_fillVertexRange.vertexBuffer;
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

	for (AlignedVector< LineRenderBatch >::const_iterator i = m_lineRenderBatches.begin(); i != m_lineRenderBatches.end(); ++i)
	{
		if (shaderLine && shaderLine->getCurrentProgram())
		{
			render::NonIndexedRenderBlock* renderBlock = renderContext->alloc< render::NonIndexedRenderBlock >("Flash AccShape; draw line batch");
			renderBlock->program = shaderLine->getCurrentProgram();
			renderBlock->vertexBuffer = i->vertexRange.vertexBuffer;
			renderBlock->primitive = i->primitives.type;
			renderBlock->offset = i->primitives.offset;
			renderBlock->count = i->primitives.count;
			renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
			renderBlock->programParams->beginParameters(renderContext);
			renderBlock->programParams->setTextureParameter(m_shapeResources->m_handleLineData, i->lineTexture);
			renderBlock->programParams->setVectorParameter(m_shapeResources->m_handleLineColor, i->color);
			renderBlock->programParams->setFloatParameter(m_shapeResources->m_handleLineWidth, i->width);
			renderBlock->programParams->endParameters(renderContext);
			renderContext->draw(render::RpOverlay, renderBlock);
		}
	}
}

	}
}
