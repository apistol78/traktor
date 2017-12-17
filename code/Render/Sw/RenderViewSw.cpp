/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <functional>
#include <algorithm>
#include "Render/Sw/RenderViewSw.h"
#include "Render/Sw/RenderSystemSw.h"
#include "Render/Sw/RenderTargetSetSw.h"
#include "Render/Sw/RenderTargetSw.h"
#include "Render/Sw/VertexBufferSw.h"
#include "Render/Sw/IndexBufferSw.h"
#include "Render/Sw/ProgramSw.h"
#include "Render/Sw/Line.h"
#include "Render/Sw/Triangle.h"
#include "Render/Sw/TypesSw.h"
#include "Render/VertexElement.h"
#include "Graphics/IGraphicsSystem.h"
#include "Core/Math/Const.h"
#include "Core/Math/Float.h"
#include "Core/Misc/String.h"
#include "Core/Timer/Timer.h"
#include "Core/Log/Log.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

typedef traktor::Scalar scalar_t;

struct AxisX { static float get(const Vector2& v) { return v.x; } };
struct AxisY { static float get(const Vector2& v) { return v.y; } };

template < typename Axis, typename Pred >
int clip(
	const Vector2* in,
	int inCount,
	float v,
	Vector2* out,
	const Pred& pred
)
{
	float a1 = Axis::get(in[inCount - 1]);
	bool c1 = pred(a1, v);
	int outCount = 0;

	for (int c = 0, p = inCount - 1; c < inCount; p = c++)
	{
		float a2 = Axis::get(in[c]);
		bool c2 = pred(a2, v);
		if (c1 ^ c2)
		{
			float d = abs((v - a1) / (a2 - a1));
			out[outCount++] = in[p] + (in[c] - in[p]) * d;
		}
		if (c2)
			out[outCount++] = in[c];
		a1 = a2;
		c1 = c2;
	}

	return outCount;
}

void normalizePosition(varying_data_t& postVS)
{
	Vector4& cs = postVS[0];
	if (cs.w() > FUZZY_EPSILON)
		cs /= cs.w();
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderViewSw", RenderViewSw, IRenderView)

RenderViewSw::RenderViewSw(RenderSystemSw* renderSystem, graphics::IGraphicsSystem* graphicsSystem, Processor* processor)
:	m_renderSystem(renderSystem)
,	m_graphicsSystem(graphicsSystem)
,	m_processor(processor)
,	m_viewPort(0, 0, 0, 0, 0, 0)
,	m_instance(0)
,	m_drawCalls(0)
,	m_primitiveCount(0)
{
	m_frameBufferSurface = m_graphicsSystem->getSecondarySurface();
	m_frameBufferSurface->getSurfaceDesc(m_frameBufferSurfaceDesc);

	RenderTargetSetCreateDesc desc;
	desc.count = 0;
	desc.width = m_frameBufferSurfaceDesc.width;
	desc.height = m_frameBufferSurfaceDesc.height;
	desc.multiSample = 0;
	desc.createDepthStencil = true;
	desc.usingPrimaryDepthStencil = false;
	desc.preferTiled = false;
	desc.ignoreStencil = false;
	desc.generateMips = false;

	m_primaryTarget = new RenderTargetSetSw();
	m_primaryTarget->create(desc);

	m_viewPort = Viewport(0, 0, m_frameBufferSurfaceDesc.width, m_frameBufferSurfaceDesc.height, 0.0f, 1.0f);

	m_targetSize.set(
		float(m_frameBufferSurfaceDesc.width),
		float(m_frameBufferSurfaceDesc.height),
		0.0f,
		0.0f
	);
}

RenderViewSw::~RenderViewSw()
{
	T_ASSERT (!m_graphicsSystem);
}

bool RenderViewSw::nextEvent(RenderEvent& outEvent)
{
#if defined(_WIN32)

	bool going = true;
	MSG msg;

	while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
	{
		int ret = GetMessage(&msg, NULL, 0, 0);
		if (ret <= 0 || msg.message == WM_QUIT)
			going = false;

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return false;

#else
	return false;
#endif
}

void RenderViewSw::close()
{
	if (m_graphicsSystem)
	{
		m_graphicsSystem->destroy();
		m_graphicsSystem = 0;
	}
}

bool RenderViewSw::reset(const RenderViewDefaultDesc& desc)
{
	return false;
}

bool RenderViewSw::reset(int32_t width, int32_t height)
{
	bool result = m_graphicsSystem->resize(width, height);
	T_ASSERT (result);

	m_frameBufferSurface = m_graphicsSystem->getSecondarySurface();
	m_frameBufferSurface->getSurfaceDesc(m_frameBufferSurfaceDesc);

	RenderTargetSetCreateDesc desc;
	desc.count = 0;
	desc.width = m_frameBufferSurfaceDesc.width;
	desc.height = m_frameBufferSurfaceDesc.height;
	desc.multiSample = 0;
	desc.createDepthStencil = true;
	desc.usingPrimaryDepthStencil = false;
	desc.preferTiled = false;
	desc.ignoreStencil = false;
	desc.generateMips = false;

	m_primaryTarget = new RenderTargetSetSw();
	m_primaryTarget->create(desc);

	m_viewPort.width = width;
	m_viewPort.height = height;

	m_targetSize.set(
		float(width),
		float(height),
		0.0f,
		0.0f
	);

	return true;
}

int RenderViewSw::getWidth() const
{
	return m_frameBufferSurfaceDesc.width;
}

int RenderViewSw::getHeight() const
{
	return m_frameBufferSurfaceDesc.height;
}

bool RenderViewSw::isActive() const
{
	return true;
}

bool RenderViewSw::isMinimized() const
{
	return false;
}

bool RenderViewSw::isFullScreen() const
{
	return false;
}

void RenderViewSw::showCursor()
{
}

void RenderViewSw::hideCursor()
{
}

bool RenderViewSw::isCursorVisible() const
{
	return true;
}

bool RenderViewSw::setGamma(float gamma)
{
	return false;
}

void RenderViewSw::setViewport(const Viewport& viewport)
{
	m_viewPort.left = viewport.left;
	m_viewPort.top = viewport.top;
	m_viewPort.width = viewport.width;
	m_viewPort.height = viewport.height;
	m_viewPort.nearZ = viewport.nearZ;
	m_viewPort.farZ = viewport.farZ;
}

Viewport RenderViewSw::getViewport()
{
	return m_viewPort;
}

SystemWindow RenderViewSw::getSystemWindow()
{
	return SystemWindow();
}

bool RenderViewSw::begin(EyeType eye)
{
	T_ASSERT (m_renderStateStack.empty());

	uint32_t* frameBuffer = static_cast< uint32_t* >(m_frameBufferSurface->lock(m_frameBufferSurfaceDesc));

	RenderState rs =
	{
		m_viewPort,
		m_frameBufferSurfaceDesc.width,
		m_frameBufferSurfaceDesc.height,
		frameBuffer,
		m_frameBufferSurfaceDesc.pitch,
		m_primaryTarget->getDepthSurface(),
		m_frameBufferSurfaceDesc.width * sizeof(float),
		m_primaryTarget->getStencilSurface(),
		m_frameBufferSurfaceDesc.width * sizeof(uint8_t),
	};

	m_renderStateStack.push_back(rs);

	m_drawCalls = 0;
	m_primitiveCount = 0;
	return true;
}

bool RenderViewSw::begin(RenderTargetSet* renderTargetSet)
{
	return false;
}

bool RenderViewSw::begin(RenderTargetSet* renderTargetSet, int renderTarget)
{
	T_ASSERT (!m_renderStateStack.empty());

	RenderTargetSetSw* rts = checked_type_cast< RenderTargetSetSw* >(renderTargetSet);
	RenderTargetSw* rt = checked_type_cast< RenderTargetSw* >(rts->getColorTexture(renderTarget));

	RenderState rs =
	{
		Viewport(0, 0, rts->getWidth(), rts->getHeight(), 0.0f, 1.0f),
		rts->getWidth(),
		rts->getHeight(),
		rt->getColorSurface(),
		rt->getWidth() * sizeof(uint32_t),
		rts->getDepthSurface(),
		rt->getWidth() * sizeof(float),
		rts->getStencilSurface(),
		rt->getWidth() * sizeof(uint8_t)
	};

	if (rts->usingPrimaryDepth())
	{
		rs.depthTarget = m_primaryTarget->getDepthSurface();
		rs.depthTargetPitch = m_frameBufferSurfaceDesc.width * sizeof(float);
		rs.stencilTarget = m_primaryTarget->getStencilSurface();
		rs.stencilTargetPitch = m_frameBufferSurfaceDesc.width * sizeof(uint8_t);
	}

	m_renderStateStack.push_back(rs);
	return true;
}

void RenderViewSw::clear(uint32_t clearMask, const Color4f* colors, float depth, int32_t stencil)
{
	RenderState& rs = m_renderStateStack.back();
	if (clearMask & CfColor)
	{
		uint32_t* colorTarget = rs.colorTarget;
		if (colorTarget)
		{
			uint32_t clearColor = toARGB(colors[0]);
			for (int32_t y = 0; y < rs.height; ++y)
			{
				for (int32_t x = 0; x < rs.width; ++x)
					colorTarget[x] = clearColor;
				colorTarget += rs.colorTargetPitch / sizeof(uint32_t);
			}
		}
	}
	if (clearMask & CfDepth)
	{
		float* depthTarget = rs.depthTarget;
		if (depthTarget)
		{
			Vector4 depth4f(depth, depth, depth, depth);
			for (int32_t y = 0; y < rs.height; ++y)
			{
				for (int32_t x = 0; x < rs.width; x += 4)
					depth4f.storeAligned(&depthTarget[x]);
				depthTarget += rs.depthTargetPitch / sizeof(uint32_t);
			}
		}
	}
}

void RenderViewSw::draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives)
{
	m_currentVertexBuffer = checked_type_cast< VertexBufferSw* >(vertexBuffer);
	m_currentIndexBuffer = checked_type_cast< IndexBufferSw* >(indexBuffer);
	m_currentProgram = checked_type_cast< ProgramSw * >(program);
	m_instance = 0;

	if (primitives.indexed)
		drawIndexed(primitives);
	else
		drawNonIndexed(primitives);

	m_drawCalls++;
	m_primitiveCount += primitives.count;
}

void RenderViewSw::draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives, uint32_t instanceCount)
{
	m_currentVertexBuffer = checked_type_cast< VertexBufferSw* >(vertexBuffer);
	m_currentIndexBuffer = checked_type_cast< IndexBufferSw* >(indexBuffer);
	m_currentProgram = checked_type_cast< ProgramSw * >(program);

	if (primitives.indexed)
	{
		for (m_instance = 0; m_instance < instanceCount; ++m_instance)
			drawIndexed(primitives);
	}
	else
	{
		for (m_instance = 0; m_instance < instanceCount; ++m_instance)
			drawNonIndexed(primitives);
	}

	m_drawCalls++;
	m_primitiveCount += primitives.count * instanceCount;
}

void RenderViewSw::end()
{
	T_ASSERT (!m_renderStateStack.empty());

#if 0
	{
		RenderState& rs = m_renderStateStack.back();

		Ref< drawing::Image > image = new drawing::Image(
			drawing::PixelFormat::getR8G8B8A8(),
			rs.width,
			rs.height
		);

		for (int32_t y = 0; y < rs.height; ++y)
		{
			for (int32_t x = 0; x < rs.width; ++x)
			{
				uint32_t c = *(rs.colorTarget + y * (rs.colorTargetPitch / 4) + x);
				Vector4 v = fromARGB(c);
				image->setPixelUnsafe(x, y, Color4f(v));
			}
		}

		static int32_t s_counter = 0;
		image->save(std::wstring(L"RS_") + toString(s_counter++) + L".png");
	}
#endif

	m_renderStateStack.pop_back();
	if (m_renderStateStack.empty())
		m_frameBufferSurface->unlock();
	else
		m_viewPort = m_renderStateStack.back().viewPort;
}

void RenderViewSw::present()
{
	m_graphicsSystem->flip(false);
}

void RenderViewSw::pushMarker(const char* const marker)
{
}

void RenderViewSw::popMarker()
{
}

void RenderViewSw::getStatistics(RenderViewStatistics& outStatistics) const
{
	outStatistics.drawCalls = m_drawCalls;
	outStatistics.primitiveCount = m_primitiveCount;
}

bool RenderViewSw::getBackBufferContent(void* buffer) const
{
	return false;
}

void RenderViewSw::fetchVertex(uint32_t index, varying_data_t& outVertexVarying) const
{
	const std::vector< VertexElement >& vertexElements = m_currentVertexBuffer->getVertexElements();
	const VertexBufferSw::vertex_tuple_t* vertices = m_currentVertexBuffer->getData();

	uint32_t vertexElementCount = uint32_t(vertexElements.size());
	uint32_t vertexOffset = index * vertexElementCount;

	T_ASSERT (index <= m_currentVertexBuffer->getVertexCount());

	for (uint32_t i = 0; i < vertexElementCount; ++i)
	{
		uint32_t varyingOffset = getVaryingOffset(vertexElements[i].getDataUsage(), vertexElements[i].getIndex());
		outVertexVarying[varyingOffset] = Vector4::loadUnaligned((const float *)vertices[i + vertexOffset]);
	}

	checkVaryings(outVertexVarying);
}

void RenderViewSw::executeVertexShader(const varying_data_t& vertexVarying, varying_data_t& outInterpolatorVarying) /*const*/
{
	checkVaryings(vertexVarying);

	m_processor->execute(
		m_currentProgram->getVertexProgram(),
		m_instance,
		m_currentProgram->getParameters(),		// Uniforms
		(const Vector4*)&vertexVarying,			// Vertex varyings
		m_targetSize,							// Target size
		Vector4::zero(),						// Fragment position
		0,										// Samplers
		(Vector4*)&outInterpolatorVarying		// Output, interpolates, varyings
	);

	checkVaryings(outInterpolatorVarying);
}

void RenderViewSw::clipPlanes(FragmentContext& context, uint32_t vertexCount) const
{
	const RenderState& rs = m_renderStateStack.back();
	float nz = rs.viewPort.nearZ;

	uint32_t& cc = context.clippedCount; cc = 0;
	for (uint32_t i = vertexCount - 1, j = 0; j < vertexCount; i = j++)
	{
		const varying_data_t& v1 = context.interpolatorVaryings[i];
		const varying_data_t& v2 = context.interpolatorVaryings[j];
		if ((v1[0].z() >= nz) ^ (v2[0].z() >= nz))
		{
			float d = (nz - v1[0].z()) / (v2[0].z() - v1[0].z());
			interpolateVaryings(v1, v2, d, context.clippedVaryings[cc]);
			checkVaryings(context.clippedVaryings[cc]);
			++cc;
		}
		if (v2[0].z() >= nz)
		{
			copyVaryings(context.clippedVaryings[cc], v2);
			checkVaryings(context.clippedVaryings[cc]);
			++cc;
		}
	}
}

void RenderViewSw::projectScreen(FragmentContext& context) const
{
	const RenderState& rs = m_renderStateStack.back();
	for (uint32_t i = 0; i < context.clippedCount; ++i)
	{
		const Vector4& position = context.clippedVaryings[i][0];
		context.screen[i].x = (position.x() * 0.5f + 0.5f) * rs.viewPort.width + rs.viewPort.left;
		context.screen[i].y = (0.5f - position.y() * 0.5f) * rs.viewPort.height + rs.viewPort.top;
	}
}

void RenderViewSw::drawIndexed(const Primitives& primitives)
{
	Timer primitiveTimer;
	primitiveTimer.start();

	const RenderState& rs = m_renderStateStack.back();

	static FragmentContext context;
	context.depthEnable = m_currentProgram->getRenderState().depthEnable && bool(rs.depthTarget != 0);
	context.depthWriteEnable = m_currentProgram->getRenderState().depthWriteEnable && bool(rs.depthTarget != 0);
	context.blendEnable = m_currentProgram->getRenderState().blendEnable;

	T_ASSERT (m_currentIndexBuffer);
	const uint32_t* indices = &m_currentIndexBuffer->getIndices().front();

	switch (primitives.type)
	{
	case PtPoints:
		break;

	case PtLineStrip:
		break;

	case PtLines:
		break;

	case PtTriangleStrip:
		break;

	case PtTriangles:
		{
			indices += primitives.offset;
			for (uint32_t i = 0; i < primitives.count; ++i)
			{
				fetchVertex(*indices++, context.vertexVaryings[0]);
				fetchVertex(*indices++, context.vertexVaryings[1]);
				fetchVertex(*indices++, context.vertexVaryings[2]);

				executeVertexShader(context.vertexVaryings[0], context.interpolatorVaryings[0]);
				executeVertexShader(context.vertexVaryings[1], context.interpolatorVaryings[1]);
				executeVertexShader(context.vertexVaryings[2], context.interpolatorVaryings[2]);

				// Perform homogenization.
				normalizePosition(context.interpolatorVaryings[0]);
				normalizePosition(context.interpolatorVaryings[1]);
				normalizePosition(context.interpolatorVaryings[2]);

				// Clip to view planes.
				clipPlanes(context, 3);
				if (context.clippedCount < 3)
					continue;

				// Project onto near view plane, aka screen.
				projectScreen(context);

//				// Inverse interpolators to prepare for perspective correction.
//				for (uint32_t j = 0; j < context.clippedCount; ++j)
//				{
//					float iw = 1.0f / context.clippedVaryings[j][0].w();
//#if defined(_DEBUG)
//					if (isNan(iw) || isInfinite(iw))
//						iw = 1.0f;
//#endif
//
//					context.clippedVaryings[j][0] =
//						context.clippedVaryings[j][0].xyz0() + Vector4(0.0f, 0.0f, 0.0f, iw);
//
//					for (uint32_t k = 1; k < sizeof(varying_data_t) / sizeof(Vector4); ++k)
//						context.clippedVaryings[j][k] *= scalar_t(iw);
//				}

				// Check culling, rewind vertices if needed as triangle rasterizer
				// can only handle one type of winding.
				Vector2 l = context.screen[1] - context.screen[0];
				Vector2 r = context.screen[2] - context.screen[0];
				float nz = l.x * r.y - r.x * l.y;

				bool cullDiscard = false;
				bool cullReversed = false;
				switch (m_currentProgram->getRenderState().cullMode)
				{
				case CmNever:
					if (nz > 0.0f)
						cullReversed = true;
					break;

				case CmClockWise:
					if (nz > 0.0f)
						cullDiscard = true;
					break;

				case CmCounterClockWise:
					if (nz < 0.0f)
						cullDiscard = true;
					cullReversed = true;
					break;

				default:
					cullDiscard = true;
				}
				if (cullDiscard)
					continue;

				for (uint32_t j = 0; j < context.clippedCount - 2; ++j)
				{
					Vector2 T[] = { context.screen[0], context.screen[1 + j], context.screen[2 + j] };

#if defined(_DEBUG)
					if (isNan(T[0].x) || isNan(T[0].y) || isInfinite(T[0].x) || isInfinite(T[0].y))
						continue;
					if (isNan(T[1].x) || isNan(T[1].y) || isInfinite(T[1].x) || isInfinite(T[1].y))
						continue;
					if (isNan(T[2].x) || isNan(T[2].y) || isInfinite(T[2].x) || isInfinite(T[2].y))
						continue;
#endif

					context.triangle = T;
					context.indices[0] = 0;
					context.indices[1] = 1 + j;
					context.indices[2] = 2 + j;

					// Calculate barycentric denominators.
					// fab(x,y) = (ya - yb) * x + (xb - xa) * y + xa * yb - xb * ya

					// f12(x0,y0) = (y1 - y2) * x0 + (x2 - x1) * y0 + x1 * y2 - x2 * y1
					context.baryOffset[0] = T[1].x * T[2].y - T[2].x * T[1].y;
					context.baryDenom[0] = 1.0f / ((T[1].y - T[2].y) * T[0].x + (T[2].x - T[1].x) * T[0].y + context.baryOffset[0]);

					// f20(x1,y1) = (y2 - y0) * x1 + (x0 - x2) * y1 + x2 * y0 - x0 * y2
					context.baryOffset[1] = T[2].x * T[0].y - T[0].x * T[2].y;
					context.baryDenom[1] = 1.0f / ((T[2].y - T[0].y) * T[1].x + (T[0].x - T[2].x) * T[1].y + context.baryOffset[1]);

					// f01(x2,y2) = (y0 - y1) * x2 + (x1 - x0) * y2 + x0 * y1 - x1 * y0
					context.baryOffset[2] = T[0].x * T[1].y - T[1].x * T[0].y;
					context.baryDenom[2] = 1.0f / ((T[0].y - T[1].y) * T[2].x + (T[1].x - T[0].x) * T[2].y + context.baryOffset[2]);

					// Clip triangle to view port.
					Vector2 T1[8], T2[8];
					int count1, count2;
					count1 = clip< AxisX >(T,       3, float(rs.viewPort.left),                     T1, std::greater_equal< float >());
					count2 = clip< AxisX >(T1, count1, float(rs.viewPort.left + rs.viewPort.width), T2, std::less         < float >());
					count1 = clip< AxisY >(T2, count2, float(rs.viewPort.top),                      T1, std::greater_equal< float >());
					count2 = clip< AxisY >(T1, count1, float(rs.viewPort.top + rs.viewPort.height), T2, std::less         < float >());

					// Rasterize clipped triangles.
					if (!cullReversed)
					{
						for (int k = 0; k < count2 - 2; ++k)
						{
							Vector2 Tc[] = { T2[0], T2[1 + k], T2[2 + k] };
							if (!context.blendEnable)
								triangle(Tc, this, &RenderViewSw::triangleShadeOpaque, context);
							else
								triangle(Tc, this, &RenderViewSw::triangleShadeBlend, context);
						}
					}
					else
					{
						for (int k = 0; k < count2 - 2; ++k)
						{
							Vector2 Tc[] = { T2[0], T2[2 + k], T2[1 + k] };
							if (!context.blendEnable)
								triangle(Tc, this, &RenderViewSw::triangleShadeOpaque, context);
							else
								triangle(Tc, this, &RenderViewSw::triangleShadeBlend, context);
						}
					}
				}
			}
		}
		break;
	}
}

void RenderViewSw::drawNonIndexed(const Primitives& primitives)
{
	Timer primitiveTimer;
	primitiveTimer.start();

	const RenderState& rs = m_renderStateStack.back();

	static FragmentContext context;
	context.depthEnable = m_currentProgram->getRenderState().depthEnable && bool(rs.depthTarget != 0);
	context.depthWriteEnable = m_currentProgram->getRenderState().depthWriteEnable && bool(rs.depthTarget != 0);
	context.blendEnable = m_currentProgram->getRenderState().blendEnable;

	switch (primitives.type)
	{
	case PtPoints:
		break;

	case PtLineStrip:
		break;

	case PtLines:
		break;

	case PtTriangleStrip:
		break;

	case PtTriangles:
		{
			for (uint32_t i = 0; i < primitives.count; ++i)
			{
				fetchVertex(primitives.offset + i * 3 + 0, context.vertexVaryings[0]);
				fetchVertex(primitives.offset + i * 3 + 1, context.vertexVaryings[1]);
				fetchVertex(primitives.offset + i * 3 + 2, context.vertexVaryings[2]);

				executeVertexShader(context.vertexVaryings[0], context.interpolatorVaryings[0]);
				executeVertexShader(context.vertexVaryings[1], context.interpolatorVaryings[1]);
				executeVertexShader(context.vertexVaryings[2], context.interpolatorVaryings[2]);

				// Perform homogenization.
				normalizePosition(context.interpolatorVaryings[0]);
				normalizePosition(context.interpolatorVaryings[1]);
				normalizePosition(context.interpolatorVaryings[2]);

				// Clip to view planes.
				clipPlanes(context, 3);
				if (context.clippedCount < 3)
					continue;

				// Project onto near view plane, aka screen.
				projectScreen(context);

//				// Inverse interpolators to prepare for perspective correction.
//				for (uint32_t j = 0; j < context.clippedCount; ++j)
//				{
//					float iw = 1.0f / context.clippedVaryings[j][0].w();
//#if defined(_DEBUG)
//					if (isNan(iw) || isInfinite(iw))
//						iw = 1.0f;
//#endif
//
//					context.clippedVaryings[j][0] =
//						context.clippedVaryings[j][0].xyz0() +
//						Vector4(0.0f, 0.0f, 0.0f, iw);
//
//					for (uint32_t k = 1; k < sizeof(varying_data_t) / sizeof(Vector4); ++k)
//						context.clippedVaryings[j][k] *= scalar_t(iw);
//				}

				// Check culling, rewind vertices if needed as triangle rasterizer
				// can only handle one type of winding.
				Vector2 l = context.screen[1] - context.screen[0];
				Vector2 r = context.screen[2] - context.screen[0];
				float nz = l.x * r.y - r.x * l.y;

				bool cullDiscard = false;
				bool cullReversed = false;
				switch (m_currentProgram->getRenderState().cullMode)
				{
				case CmNever:
					if (nz > 0.0f)
						cullReversed = true;
					break;

				case CmClockWise:
					if (nz > 0.0f)
						cullDiscard = true;
					break;

				case CmCounterClockWise:
					if (nz < 0.0f)
						cullDiscard = true;
					cullReversed = true;
					break;

				default:
					cullDiscard = true;
				}
				if (cullDiscard)
					continue;

				for (uint32_t j = 0; j < context.clippedCount - 2; ++j)
				{
					Vector2 T[] = { context.screen[0], context.screen[1 + j], context.screen[2 + j] };

#if defined(_DEBUG)
					if (isNan(T[0].x) || isNan(T[0].y))
						continue;
					if (isNan(T[1].x) || isNan(T[1].y))
						continue;
					if (isNan(T[2].x) || isNan(T[2].y))
						continue;
#endif

					context.triangle = T;
					context.indices[0] = 0;
					context.indices[1] = 1 + j;
					context.indices[2] = 2 + j;

					// Calculate barycentric denominators.
					// fab(x,y) = (ya - yb) * x + (xb - xa) * y + xa * yb - xb * ya

					// f12(x0,y0) = (y1 - y2) * x0 + (x2 - x1) * y0 + x1 * y2 - x2 * y1
					context.baryOffset[0] = T[1].x * T[2].y - T[2].x * T[1].y;
					context.baryDenom[0] = 1.0f / ((T[1].y - T[2].y) * T[0].x + (T[2].x - T[1].x) * T[0].y + context.baryOffset[0]);

					// f20(x1,y1) = (y2 - y0) * x1 + (x0 - x2) * y1 + x2 * y0 - x0 * y2
					context.baryOffset[1] = T[2].x * T[0].y - T[0].x * T[2].y;
					context.baryDenom[1] = 1.0f / ((T[2].y - T[0].y) * T[1].x + (T[0].x - T[2].x) * T[1].y + context.baryOffset[1]);

					// f01(x2,y2) = (y0 - y1) * x2 + (x1 - x0) * y2 + x0 * y1 - x1 * y0
					context.baryOffset[2] = T[0].x * T[1].y - T[1].x * T[0].y;
					context.baryDenom[2] = 1.0f / ((T[0].y - T[1].y) * T[2].x + (T[1].x - T[0].x) * T[2].y + context.baryOffset[2]);

					// Clip triangle to view port.
					Vector2 T1[8], T2[8];
					int count1, count2;
					count1 = clip< AxisX >(T,       3, float(rs.viewPort.left),                     T1, std::greater_equal< float >());
					count2 = clip< AxisX >(T1, count1, float(rs.viewPort.left + rs.viewPort.width), T2, std::less         < float >());
					count1 = clip< AxisY >(T2, count2, float(rs.viewPort.top),                      T1, std::greater_equal< float >());
					count2 = clip< AxisY >(T1, count1, float(rs.viewPort.top + rs.viewPort.height), T2, std::less         < float >());

					// Rasterize clipped triangles.
					if (!cullReversed)
					{
						for (int k = 0; k < count2 - 2; ++k)
						{
							Vector2 Tc[] = { T2[0], T2[1 + k], T2[2 + k] };
							if (!context.blendEnable)
								triangle(Tc, this, &RenderViewSw::triangleShadeOpaque, context);
							else
								triangle(Tc, this, &RenderViewSw::triangleShadeBlend, context);
						}
					}
					else
					{
						for (int k = 0; k < count2 - 2; ++k)
						{
							Vector2 Tc[] = { T2[0], T2[2 + k], T2[1 + k] };
							if (!context.blendEnable)
								triangle(Tc, this, &RenderViewSw::triangleShadeOpaque, context);
							else
								triangle(Tc, this, &RenderViewSw::triangleShadeBlend, context);
						}
					}
				}
			}
		}
		break;
	}
}

void RenderViewSw::triangleShadeOpaque(const FragmentContext& context, int x1, int x2, int y)
{
	RenderState& rs = m_renderStateStack.back();

	Processor::image_t pixelProgram = m_currentProgram->getPixelProgram();
	const Vector4* parameters = m_currentProgram->getParameters();
	const Ref< AbstractSampler >* samplers = m_currentProgram->getSamplers();
	uint32_t icount = m_currentProgram->getInterpolatorCount();

	uint32_t colorTargetOffset = x1 + y * rs.colorTargetPitch / sizeof(uint32_t);
	uint32_t depthTargetOffset = x1 + y * rs.depthTargetPitch / sizeof(float);

	// Calculate barycentric coordinates.
	const Vector2* T = context.triangle;
	const uint32_t* I = context.indices;

	// f12(x,y) = (y1 - y2) * x + (x2 - x1) * y + x1 * y2 - x2 * y1
	scalar_t alpha1 = scalar_t(((T[1].y - T[2].y) * x1 + (T[2].x - T[1].x) * y + context.baryOffset[0]) * context.baryDenom[0]);
	scalar_t alpha2 = scalar_t(((T[1].y - T[2].y) * x2 + (T[2].x - T[1].x) * y + context.baryOffset[0]) * context.baryDenom[0]);

	// f20(x,y) = (y2 - y0) * x + (x0 - x2) * y + x2 * y0 - x0 * y2
	scalar_t beta1 = scalar_t(((T[2].y - T[0].y) * x1 + (T[0].x - T[2].x) * y + context.baryOffset[1]) * context.baryDenom[1]);
	scalar_t beta2 = scalar_t(((T[2].y - T[0].y) * x2 + (T[0].x - T[2].x) * y + context.baryOffset[1]) * context.baryDenom[1]);

	// f01(x,y) = (y0 - y1) * x + (x1 - x0) * y + x0 * y1 - x1 * y0
	scalar_t gamma1 = scalar_t(((T[0].y - T[1].y) * x1 + (T[1].x - T[0].x) * y + context.baryOffset[2]) * context.baryDenom[2]);
	scalar_t gamma2 = scalar_t(((T[0].y - T[1].y) * x2 + (T[1].x - T[0].x) * y + context.baryOffset[2]) * context.baryDenom[2]);

	// Calculate varyings.
	T_ALIGN16 varying_data_t surfaceInterpolators[2];
	T_ALIGN16 varying_data_t pixelVaryings;
	T_ALIGN16 varying_data_t fragmentVaryings;

	scalar_t z1 = surfaceInterpolators[0][0].z();
	scalar_t z2 = surfaceInterpolators[1][0].z();
	scalar_t ix = scalar_t(1.0f / float(x2 - x1));

	for (uint32_t i = 0; i < 6 + icount; ++i)
	{
		surfaceInterpolators[0][i] = context.clippedVaryings[I[0]][i] * alpha1 + context.clippedVaryings[I[1]][i] * beta1 + context.clippedVaryings[I[2]][i] * gamma1;
		surfaceInterpolators[1][i] = context.clippedVaryings[I[0]][i] * alpha2 + context.clippedVaryings[I[1]][i] * beta2 + context.clippedVaryings[I[2]][i] * gamma2;
	}

	for (int x = 0; x < x2 - x1; ++x, ++colorTargetOffset, ++depthTargetOffset)
	{
		scalar_t t = scalar_t(x * ix);
		scalar_t z = lerp(z1, z2, t);

		if (!context.depthEnable || z <= rs.depthTarget[depthTargetOffset])
		{
			// Calculate per pixel varyings.
			for (uint32_t i = 0; i < 6 + icount; ++i)
			{
				//pixelVaryings[i] = surfaceInterpolators[0][i] / iw;
				pixelVaryings[i] = lerp(surfaceInterpolators[0][i], surfaceInterpolators[1][i], t);
			}

			// Execute pixel program.
			bool kept = m_processor->execute(
				pixelProgram,
				m_instance,
				parameters,
				pixelVaryings,
				m_targetSize,
				Vector4(float(x), float(y), 0.0f, 0.0f),
				samplers,
				fragmentVaryings
			);
			if (kept)
			{
				// Write color to current target.
				Vector4& color = fragmentVaryings[0];

				color = max(color, Vector4::zero());
				color = min(color, Vector4::one());

				rs.colorTarget[colorTargetOffset] = toARGB(color);

				if (context.depthWriteEnable)
					rs.depthTarget[depthTargetOffset] = z;
			}
		}
	}
}

void RenderViewSw::triangleShadeBlend(const FragmentContext& context, int x1, int x2, int y)
{
	RenderState& rs = m_renderStateStack.back();

	Processor::image_t pixelProgram = m_currentProgram->getPixelProgram();
	const Vector4* parameters = m_currentProgram->getParameters();
	const Ref< AbstractSampler >* samplers = m_currentProgram->getSamplers();
	uint32_t icount = m_currentProgram->getInterpolatorCount();

	uint32_t colorTargetOffset = x1 + y * rs.colorTargetPitch / sizeof(uint32_t);
	uint32_t depthTargetOffset = x1 + y * rs.depthTargetPitch / sizeof(float);

	// Calculate barycentric coordinates.
	const Vector2* T = context.triangle;
	const uint32_t* I = context.indices;

	// f12(x,y) = (y1 - y2) * x + (x2 - x1) * y + x1 * y2 - x2 * y1
	scalar_t alpha1 = scalar_t(((T[1].y - T[2].y) * x1 + (T[2].x - T[1].x) * y + context.baryOffset[0]) * context.baryDenom[0]);
	scalar_t alpha2 = scalar_t(((T[1].y - T[2].y) * x2 + (T[2].x - T[1].x) * y + context.baryOffset[0]) * context.baryDenom[0]);

	// f20(x,y) = (y2 - y0) * x + (x0 - x2) * y + x2 * y0 - x0 * y2
	scalar_t beta1 = scalar_t(((T[2].y - T[0].y) * x1 + (T[0].x - T[2].x) * y + context.baryOffset[1]) * context.baryDenom[1]);
	scalar_t beta2 = scalar_t(((T[2].y - T[0].y) * x2 + (T[0].x - T[2].x) * y + context.baryOffset[1]) * context.baryDenom[1]);

	// f01(x,y) = (y0 - y1) * x + (x1 - x0) * y + x0 * y1 - x1 * y0
	scalar_t gamma1 = scalar_t(((T[0].y - T[1].y) * x1 + (T[1].x - T[0].x) * y + context.baryOffset[2]) * context.baryDenom[2]);
	scalar_t gamma2 = scalar_t(((T[0].y - T[1].y) * x2 + (T[1].x - T[0].x) * y + context.baryOffset[2]) * context.baryDenom[2]);

	// Calculate surface interpolators.
	varying_data_t surfaceInterpolators[2];
	varying_data_t pixelVaryings;
	varying_data_t fragmentVaryings;

	scalar_t z1 = surfaceInterpolators[0][0].z();
	scalar_t z2 = surfaceInterpolators[1][0].z();
	scalar_t ix = scalar_t(1.0f / float(x2 - x1));

	// Only interpolate DuCustom across surface.
	for (uint32_t i = 0; i < 6 + icount; ++i)
	{
		surfaceInterpolators[0][i] = context.clippedVaryings[I[0]][i] * alpha1 + context.clippedVaryings[I[1]][i] * beta1 + context.clippedVaryings[I[2]][i] * gamma1;
		surfaceInterpolators[1][i] = context.clippedVaryings[I[0]][i] * alpha2 + context.clippedVaryings[I[1]][i] * beta2 + context.clippedVaryings[I[2]][i] * gamma2;
	}

	for (int x = 0; x < x2 - x1; ++x, ++colorTargetOffset, ++depthTargetOffset)
	{
		scalar_t t = scalar_t(x * ix);
		scalar_t z = lerp(z1, z2, t);

		if (!context.depthEnable || z <= rs.depthTarget[depthTargetOffset])
		{
			// Calculate per pixel varyings.
			for (uint32_t i = 0; i < 6 + icount; ++i)
			{
				//pixelVaryings[i] = surfaceInterpolators[0][i] / iw;
				pixelVaryings[i] = lerp(surfaceInterpolators[0][i], surfaceInterpolators[1][i], t);
			}

			// Execute pixel program.
			bool kept = m_processor->execute(
				pixelProgram,
				m_instance,
				parameters,
				pixelVaryings,
				m_targetSize,
				Vector4(float(x), float(y), 0.0f, 0.0f),
				samplers,
				fragmentVaryings
			);
			if (kept)
			{
				// Write color to current target.
				Vector4& color = fragmentVaryings[0];
				Vector4 destination = fromARGB(rs.colorTarget[colorTargetOffset]);

				switch (m_currentProgram->getRenderState().blendSource)
				{
				case BfOne:
					break;

				case BfZero:
					color.set(0.0f, 0.0f, 0.0f, 0.0f);
					break;

				case BfSourceColor:
					color *= color;
					break;

				case BfOneMinusSourceColor:
					color *= scalar_t(1.0f) - color;
					break;

				case BfDestinationColor:
					color *= destination;
					break;

				case BfOneMinusDestinationColor:
					color *= scalar_t(1.0f) - destination;
					break;

				case BfSourceAlpha:
					color *= color.w();
					break;

				case BfOneMinusSourceAlpha:
					color *= scalar_t(1.0f) - color.w();
					break;

				case BfDestinationAlpha:
					color *= destination.w();
					break;

				case BfOneMinusDestinationAlpha:
					color *= scalar_t(1.0f) - destination.w();
					break;
				}
				
				switch (m_currentProgram->getRenderState().blendDestination)
				{
				case BfOne:
					break;

				case BfZero:
					destination.set(0.0f, 0.0f, 0.0f, 0.0f);
					break;

				case BfSourceColor:
					destination *= color;
					break;

				case BfOneMinusSourceColor:
					destination *= scalar_t(1.0f) - color;
					break;

				case BfDestinationColor:
					destination *= destination;
					break;

				case BfOneMinusDestinationColor:
					destination *= scalar_t(1.0f) - destination;
					break;

				case BfSourceAlpha:
					destination *= color.w();
					break;

				case BfOneMinusSourceAlpha:
					destination *= scalar_t(1.0f) - color.w();
					break;

				case BfDestinationAlpha:
					destination *= destination.w();
					break;

				case BfOneMinusDestinationAlpha:
					destination *= scalar_t(1.0f) - destination.w();
					break;
				}

				switch (m_currentProgram->getRenderState().blendOperation)
				{
				case BoAdd:
					color = color + destination;
					break;

				case BoSubtract:
					color = color - destination;
					break;

				case BoReverseSubtract:
					color = destination - color;
					break;
				}

				color = max(color, Vector4::zero());
				color = min(color, Vector4::one());

				rs.colorTarget[colorTargetOffset] = toARGB(color);

				if (context.depthWriteEnable)
					rs.depthTarget[depthTargetOffset] = z;
			}
		}
	}
}

	}
}
