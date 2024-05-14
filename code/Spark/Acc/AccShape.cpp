/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <limits>
#include "Core/Log/Log.h"
#include "Core/Math/Bezier2nd.h"
#include "Core/Math/Const.h"
#include "Core/Math/Color4ub.h"
#include "Core/Math/Half.h"
#include "Render/Buffer.h"
#include "Render/IRenderSystem.h"
#include "Render/Shader.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderPass.h"
#include "Spark/ColorTransform.h"
#include "Spark/Canvas.h"
#include "Spark/Dictionary.h"
#include "Spark/Shape.h"
#include "Spark/Bitmap.h"
#include "Spark/Triangulator.h"
#include "Spark/Acc/AccBitmapRect.h"
#include "Spark/Acc/AccGradientCache.h"
#include "Spark/Acc/AccShape.h"
#include "Spark/Acc/AccShapeResources.h"
#include "Spark/Acc/AccTextureCache.h"

namespace traktor
{
	namespace spark
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

const render::Handle s_handleTransform(L"Spark_Transform");
const render::Handle s_handleClipBounds(L"Spark_ClipBounds");
const render::Handle s_handleFrameBounds(L"Spark_FrameBounds");
const render::Handle s_handleFrameTransform(L"Spark_FrameTransform");
const render::Handle s_handleCxFormMul(L"Spark_CxFormMul");
const render::Handle s_handleCxFormAdd(L"Spark_CxFormAdd");
const render::Handle s_handleTexture(L"Spark_Texture");
const render::Handle s_handleTextureClamp(L"Spark_TextureClamp");
const render::Handle s_handleLineData(L"Spark_LineData");
const render::Handle s_handleLineWidth(L"Spark_LineWidth");
const render::Handle s_handleLineColor(L"Spark_LineColor");
const render::Handle s_handleTechniques[] =
{
	render::Handle(L"Spark_Default"),
	render::Handle(L"Spark_Default"),
	render::Handle(L"Spark_Layer"),
	render::Handle(L"Spark_Multiply"),
	render::Handle(L"Spark_Screen"),
	render::Handle(L"Spark_Lighten"),
	render::Handle(L"Spark_Darken"),
	render::Handle(L"Spark_Difference"),
	render::Handle(L"Spark_Add"),
	render::Handle(L"Spark_Subtract"),
	render::Handle(L"Spark_Invert"),
	render::Handle(L"Spark_Alpha"),
	render::Handle(L"Spark_Erase"),
	render::Handle(L"Spark_Overlay"),
	render::Handle(L"Spark_Hardlight"),
	render::Handle(L"Spark_Opaque")
};

const static Matrix33 c_textureTS = translate(0.5f, 0.5f) * scale(1.0f / 32768.0f, 1.0f / 32768.0f);

#pragma pack(1)
struct LineData
{
	float start[2];
	float end[2];
};
#pragma pack()

		}

AccShape::AccShape(
	render::IRenderSystem* renderSystem,
	const AccShapeResources* shapeResources,
	AccShapeVertexPool* fillVertexPool,
	AccShapeVertexPool* lineVertexPool
)
:	m_renderSystem(renderSystem)
,	m_shapeResources(shapeResources)
,	m_fillVertexPool(fillVertexPool)
,	m_lineVertexPool(lineVertexPool)
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
	for (const auto& triangle : triangles)
	{
		for (int32_t k = 0; k < 3; ++k)
			triangleBounds.contain(triangle.v[k]);
	}

	for (const auto& line : lines)
	{
		const LineStyle& lineStyle = lineStyles[line.lineStyle - 1];
		const float width = lineStyle.getLineWidth() / 2.0f;

		for (int32_t k = 0; k < 2; ++k)
		{
			const Vector2& pt = line.v[k];
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
		SmallSet< int32_t > uniqueLineStyles;
		for (const auto& line : lines)
			uniqueLineStyles.insert(line.lineStyle);

		// Generate cluster and batches for each line style.
		for (const auto uniqueLineStyle : uniqueLineStyles)
		{
			const LineStyle& lineStyle = lineStyles[uniqueLineStyle - 1];
			const float width = lineStyle.getLineWidth() / 2.0f + 0.5f;

			// Create root clusters.
			AlignedVector< LineCluster > clusters;
			clusters.push_back();
			clusters.back().bounds = lineBounds;
			for (int32_t i = 0; i < lines.size(); ++i)
			{
				if (lines[i].lineStyle == uniqueLineStyle)
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
								const Vector2 dxy(1.0f / float(c_subSize), 1.0f / float(c_subSize));
								const Vector2 fxy = Vector2(ix, iy) * dxy;

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

				auto itr = std::remove_if(clusters.begin(), clusters.end(), [](LineCluster& c) {
					return c.lines.empty();
				});
				clusters.erase(itr, clusters.end());

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
			Ref< render::Buffer > vertexRange;
			if (!m_lineVertexPool->acquire(clusters.size() * 2 * 3, vertexRange))
				return false;

			LineVertex* vertex = static_cast< LineVertex* >(vertexRange->lock());
			if (!vertex)
				return false;

			// Calculate size of buffer.
			int32_t lineDataSize = 0;
			for (auto c : clusters)
				lineDataSize += c.lines.size();

			Ref< render::Buffer > lineBuffer = m_renderSystem->createBuffer(render::BufferUsage::BuStructured, lineDataSize * sizeof(LineData), false);
			if (!lineBuffer)
				return false;

			LineData* lineData = static_cast< LineData* >(lineBuffer->lock());
			int32_t lineDataOffset = 0;

			m_lineRenderBatches.push_back();
			m_lineRenderBatches.back().vertexRange = vertexRange;
			m_lineRenderBatches.back().lineBuffer = lineBuffer;
			m_lineRenderBatches.back().primitives.setNonIndexed(render::PrimitiveType::Triangles, 0, clusters.size() * 2);
			m_lineRenderBatches.back().color = lineStyle.getLineColor();
			m_lineRenderBatches.back().width = width - 0.5f;

			for (int32_t i = 0; i < clusters.size(); ++i)
			{
				const LineCluster& c = clusters[i];
				if (c.lines.empty())
					continue;

				Vector2 q[4];
				c.bounds.getExtents(q);

				const int t[] = { 0, 1, 3, 2, 3, 1 };
				for (int j = 0; j < sizeof_array(t); ++j)
				{
					vertex->pos[0] = q[t[j]].x;
					vertex->pos[1] = q[t[j]].y;
					vertex->lineOffset = (int32_t)lineDataOffset;
					vertex->lineCount = (int32_t)c.lines.size();
					vertex++;
				}

				for (int32_t j = 0; j < c.lines.size(); ++j)
				{
					const Line& line = lines[c.lines[j]];
					LineData* p = &lineData[lineDataOffset];
					p->start[0] = line.v[0].x;
					p->start[1] = line.v[0].y;
					p->end[0] = line.v[1].x;
					p->end[1] = line.v[1].y;
					++lineDataOffset;
				}
			}

			T_ASSERT(lineDataOffset == lineDataSize);

			lineBuffer->unlock();
			vertexRange->unlock();

			m_lineVertexRanges.insert(vertexRange);
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

		if (!m_fillVertexPool->acquire(triangles.size() * 3, m_fillVertexRange))
			return false;

		FillVertex* vertex = static_cast< FillVertex* >(m_fillVertexRange->lock());
		if (!vertex)
			return false;

		for (const auto& triangle : triangles)
		{
			T_ASSERT(triangle.fillStyle);

			uint8_t curveSign = c_cpZero;
			if (triangle.type == TriangleType::In)
				curveSign = c_cpOne;
			else if (triangle.type == TriangleType::Out)
				curveSign = c_cpNegOne;

			if (
				!fillStyles.empty() &&
				triangle.fillStyle != lastFillStyle
			)
			{
				color = Color4ub(255, 255, 255, 255);
				texture = 0;

				const FillStyle& style = fillStyles[triangle.fillStyle - 1];

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

				lastFillStyle = triangle.fillStyle;
			}

			if (m_fillRenderBatches.empty() || m_fillRenderBatches.back().texture != texture || m_fillRenderBatches.back().textureClamp != textureClamp)
			{
				m_fillRenderBatches.push_back(FillRenderBatch());
				m_fillRenderBatches.back().primitives.setNonIndexed(render::PrimitiveType::Triangles, vertexOffset, 0);
				m_fillRenderBatches.back().texture = texture;
				m_fillRenderBatches.back().textureClamp = textureClamp;
			}

			if (texture)
			{
				for (int k = 0; k < 3; ++k)
				{
					const Vector2& P = triangle.v[k];
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
					const Vector2& P = triangle.v[k];

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

		m_fillVertexRange->unlock();
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
	m_shapeResources = nullptr;

	if (m_fillVertexPool)
	{
		m_fillVertexPool->release(m_fillVertexRange);
		m_fillVertexPool = nullptr;
	}

	if (m_lineVertexPool)
	{
		for (auto lineVertexRange : m_lineVertexRanges)
			m_lineVertexPool->release(lineVertexRange);
		m_lineVertexRanges.clear();
	}

	m_fillRenderBatches.clear();
}

void AccShape::render(
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
) const
{
	const Matrix44 m(
		transform.e11, transform.e12, transform.e13, 0.0f,
		transform.e21, transform.e22, transform.e23, 0.0f,
		transform.e31, transform.e32, transform.e33, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	render::IProgram* programSolid = nullptr;
	render::IProgram* programTextured = nullptr;
	render::IProgram* programLine = nullptr;

	if ((m_batchFlags & BfHaveSolid) != 0)
	{
		if (!maskWrite)
		{
			const render::Shader::Permutation perm(s_handleTechniques[blendMode]);
			programSolid = m_shapeResources->m_shaderSolid->getProgram(perm).program;
		}
		else if (maskIncrement)
			programSolid = m_shapeResources->m_shaderIncrementMask->getProgram().program;
		else
			programSolid = m_shapeResources->m_shaderDecrementMask->getProgram().program;
	}

	if ((m_batchFlags & BfHaveTextured) != 0)
	{
		if (!maskWrite)
		{
			const render::Shader::Permutation perm(s_handleTechniques[blendMode]);
			programTextured = m_shapeResources->m_shaderTextured->getProgram(perm).program;
		}
	}

	if ((m_batchFlags & BfHaveLines) != 0)
	{
		if (!maskWrite)
		{
			const render::Shader::Permutation perm(s_handleTechniques[blendMode]);
			programLine = m_shapeResources->m_shaderLine->getProgram(perm).program;
		}
	}

	renderPass->addBuild([=, this](const render::RenderGraph&, render::RenderContext* renderContext) {

		render::ProgramParameters* sharedParams = renderContext->alloc < render::ProgramParameters >();
		sharedParams->beginParameters(renderContext);
		sharedParams->setMatrixParameter(s_handleTransform, m);
		sharedParams->setVectorParameter(s_handleClipBounds, clipBounds);
		sharedParams->setVectorParameter(s_handleFrameBounds, frameBounds);
		sharedParams->setVectorParameter(s_handleFrameTransform, frameTransform);
		sharedParams->setVectorParameter(s_handleCxFormMul, cxform.mul);
		sharedParams->setVectorParameter(s_handleCxFormAdd, cxform.add);
		sharedParams->setStencilReference(maskReference);
		sharedParams->endParameters(renderContext);

		for (const auto& batch : m_fillRenderBatches)
		{
			if (!batch.texture)
			{
				if (programSolid)
				{
					auto rb = renderContext->allocNamed< render::NonIndexedRenderBlock >(L"Flash AccShape; draw solid batch");
					rb->program = programSolid;
					rb->programParams = sharedParams;
					rb->vertexBuffer = m_fillVertexRange->getBufferView();
					rb->vertexLayout = m_fillVertexPool->getVertexLayout();
					rb->primitive = batch.primitives.type;
					rb->offset = batch.primitives.offset;
					rb->count = batch.primitives.count;
					renderContext->draw(rb);
				}
			}
			else
			{
				if (programTextured)
				{
					auto rb = renderContext->allocNamed< render::NonIndexedRenderBlock >(L"Flash AccShape; draw textured batch");
					rb->program = programTextured;
					rb->vertexBuffer = m_fillVertexRange->getBufferView();
					rb->vertexLayout = m_fillVertexPool->getVertexLayout();
					rb->primitive = batch.primitives.type;
					rb->offset = batch.primitives.offset;
					rb->count = batch.primitives.count;
					rb->programParams = renderContext->alloc< render::ProgramParameters >();
					rb->programParams->beginParameters(renderContext);
					rb->programParams->attachParameters(sharedParams);
					rb->programParams->setTextureParameter(s_handleTexture, batch.texture->texture);
					rb->programParams->setFloatParameter(s_handleTextureClamp, batch.textureClamp ? 1.0f : 0.0f);
					rb->programParams->endParameters(renderContext);
					renderContext->draw(rb);
				}
			}
		}

		for (const auto& batch : m_lineRenderBatches)
		{
			if (programLine)
			{
				auto rb = renderContext->allocNamed< render::NonIndexedRenderBlock >(L"Flash AccShape; draw line batch");
				rb->program = programLine;
				rb->vertexBuffer = batch.vertexRange->getBufferView();
				rb->vertexLayout = m_lineVertexPool->getVertexLayout();
				rb->primitive = batch.primitives.type;
				rb->offset = batch.primitives.offset;
				rb->count = batch.primitives.count;
				rb->programParams = renderContext->alloc< render::ProgramParameters >();
				rb->programParams->beginParameters(renderContext);
				rb->programParams->setBufferViewParameter(s_handleLineData, batch.lineBuffer->getBufferView());
				rb->programParams->setVectorParameter(s_handleLineColor, batch.color);
				rb->programParams->setFloatParameter(s_handleLineWidth, batch.width);
				rb->programParams->endParameters(renderContext);
				renderContext->draw(rb);
			}
		}
	});
}

	}
}
