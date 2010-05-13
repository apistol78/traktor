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
#include "Render/VertexElement.h"
#include "Graphics/IGraphicsSystem.h"
#include "Core/Math/Const.h"
#include "Core/Math/Float.h"
#include "Core/Timer/Timer.h"
#include "Core/Log/Log.h"

#if defined(max)
#	undef max
#endif
#if defined(min)
#	undef min
#endif

namespace traktor
{
	namespace render
	{
		namespace
		{

const int32_t c_targetScale = 1;

inline float clamp(float v, float mn, float mx)
{
	return min(max(v, mn), mx);
}

inline uint16_t to565(float r, float g, float b)
{
	return
		(int32_t(r * ((1 << 5) - 1)) << 11) |
		(int32_t(g * ((1 << 6) - 1)) << 5) |
		(int32_t(b * ((1 << 5) - 1)));
}

inline uint16_t to565(const Vector4& rgb)
{
	return to565(rgb.x(), rgb.y(), rgb.z());
}

inline uint16_t toDepth(float v)
{
	return uint16_t(v * 65535.0f);
}

struct AxisX { static float get(const Vector2& v) { return v.x; } };
struct AxisY { static float get(const Vector2& v) { return v.y; } };

template < typename Axis, typename Pred >
inline int clip(
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

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderViewSw", RenderViewSw, IRenderView)

RenderViewSw::RenderViewSw(RenderSystemSw* renderSystem, graphics::IGraphicsSystem* graphicsSystem, Processor* processor)
:	m_renderSystem(renderSystem)
,	m_graphicsSystem(graphicsSystem)
,	m_processor(processor)
,	m_depthBuffer(0)
,	m_viewPort(0, 0, 0, 0, 0, 0)
{
	m_frameBufferSurface = m_graphicsSystem->getSecondarySurface();
	m_frameBufferSurface->getSurfaceDesc(m_frameBufferSurfaceDesc);

	m_depthBuffer.reset(new uint16_t [m_frameBufferSurfaceDesc.width * m_frameBufferSurfaceDesc.height]);
	m_viewPort = Viewport(0, 0, m_frameBufferSurfaceDesc.width, m_frameBufferSurfaceDesc.height, 0.0f, 1.0f);

	//m_dirty[0] = 0;
	//m_dirty[1] = 0;
	//m_dirty[2] = m_frameBufferSurfaceDesc.width;
	//m_dirty[3] = m_frameBufferSurfaceDesc.height;

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
	m_depthBuffer.release();
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

void RenderViewSw::resize(int32_t width, int32_t height)
{
	bool result = m_graphicsSystem->resize(width / c_targetScale, height / c_targetScale);
	T_ASSERT (result);

	m_frameBufferSurface = m_graphicsSystem->getSecondarySurface();
	m_frameBufferSurface->getSurfaceDesc(m_frameBufferSurfaceDesc);

	m_depthBuffer.reset(new uint16_t [m_frameBufferSurfaceDesc.width * m_frameBufferSurfaceDesc.height]);
	m_viewPort.width = width / c_targetScale;
	m_viewPort.height = height / c_targetScale;

	//m_dirty[0] = 0;
	//m_dirty[1] = 0;
	//m_dirty[2] = m_frameBufferSurfaceDesc.width;
	//m_dirty[3] = m_frameBufferSurfaceDesc.height;

	m_targetSize.set(
		float(width),
		float(height),
		0.0f,
		0.0f
	);
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

bool RenderViewSw::isFullScreen() const
{
	return false;
}

void RenderViewSw::setViewport(const Viewport& viewport)
{
	m_viewPort.left = viewport.left / c_targetScale;
	m_viewPort.top = viewport.top / c_targetScale;
	m_viewPort.width = viewport.width / c_targetScale;
	m_viewPort.height = viewport.height / c_targetScale;
	m_viewPort.nearZ = viewport.nearZ;
	m_viewPort.farZ = viewport.farZ;
}

Viewport RenderViewSw::getViewport()
{
	return m_viewPort;
}

bool RenderViewSw::begin()
{
	T_ASSERT (m_renderStateStack.empty());

	uint16_t* frameBuffer = static_cast< uint16_t* >(m_frameBufferSurface->lock(m_frameBufferSurfaceDesc));

	RenderState rs =
	{
		m_viewPort,
		m_frameBufferSurfaceDesc.width,
		m_frameBufferSurfaceDesc.height,
		frameBuffer,
		m_frameBufferSurfaceDesc.pitch,
		m_depthBuffer.ptr(),
		m_frameBufferSurfaceDesc.width * sizeof(uint16_t)
	};

	m_renderStateStack.push_back(rs);
	return true;
}

bool RenderViewSw::begin(RenderTargetSet* renderTargetSet, int renderTarget, bool keepDepthStencil)
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
		rt->getWidth() * sizeof(uint16_t),
		rts->getDepthSurface(),
		rt->getWidth() * sizeof(uint16_t)
	};

	if (keepDepthStencil)
	{
		rs.depthTarget = m_renderStateStack.back().depthTarget;
		rs.depthTargetPitch = m_renderStateStack.back().depthTargetPitch;
	}

	m_renderStateStack.push_back(rs);

	rts->setContentValid(true);
	return true;
}

void RenderViewSw::clear(uint32_t clearMask, const float color[4], float depth, int32_t stencil)
{
	RenderState& rs = m_renderStateStack.back();

	int32_t x1 = 0; //std::max< int32_t >(m_dirty[0], 0);
	int32_t y1 = 0; //std::max< int32_t >(m_dirty[1], 0);
	int32_t x2 = rs.width; //std::min< int32_t >(m_dirty[2], rs.width);
	int32_t y2 = rs.height; //std::min< int32_t >(m_dirty[3], rs.height);

	if (clearMask & CfColor)
	{
		uint16_t* colorTarget = rs.colorTarget;
		if (colorTarget)
		{
			uint16_t clearColor = to565(color[0], color[1], color[2]);

			colorTarget += y1 * rs.colorTargetPitch / sizeof(uint16_t);
			for (int32_t y = y1; y < y2; ++y)
			{
				for (int32_t x = x1; x < x2; ++x)
					colorTarget[x] = clearColor;
				colorTarget += rs.colorTargetPitch / sizeof(uint16_t);
			}
		}
	}

	if (clearMask & CfDepth)
	{
		uint16_t* depthTarget = rs.depthTarget;
		if (depthTarget)
		{
			uint16_t clearDepth = toDepth(1.0f - depth);

			depthTarget += y1 * rs.depthTargetPitch / sizeof(uint16_t);
			for (int32_t y = y1; y < y2; ++y)
			{
				for (int32_t x = x1; x < x2; ++x)
					depthTarget[x] = clearDepth;
				depthTarget += rs.depthTargetPitch / sizeof(uint16_t);
			}
		}
	}

	//m_dirty[0] = m_frameBufferSurfaceDesc.width;
	//m_dirty[1] = m_frameBufferSurfaceDesc.height;
	//m_dirty[2] = 0;
	//m_dirty[3] = 0;
}

void RenderViewSw::setVertexBuffer(VertexBuffer* vertexBuffer)
{
	m_currentVertexBuffer = checked_type_cast< VertexBufferSw* >(vertexBuffer);
}

void RenderViewSw::setIndexBuffer(IndexBuffer* indexBuffer)
{
	m_currentIndexBuffer = checked_type_cast< IndexBufferSw* >(indexBuffer);
}

void RenderViewSw::setProgram(IProgram* program)
{
	m_currentProgram = checked_type_cast< ProgramSw * >(program);
}

void RenderViewSw::draw(const Primitives& primitives)
{
	if (primitives.indexed)
		drawIndexed(primitives);
	else
		drawNonIndexed(primitives);
}

void RenderViewSw::end()
{
	T_ASSERT (!m_renderStateStack.empty());

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

void RenderViewSw::fetchVertex(uint32_t index, varying_data_t& outVertexVarying) const
{
	const std::vector< VertexElement >& vertexElements = m_currentVertexBuffer->getVertexElements();
	const VertexBufferSw::vertex_tuple_t* vertices = m_currentVertexBuffer->getData();

	uint32_t vertexElementCount = uint32_t(vertexElements.size());
	uint32_t vertexOffset = index * vertexElementCount;

	T_ASSERT (vertexElementCount + vertexOffset <= m_currentVertexBuffer->getVertexCount());

	for (uint32_t i = 0; i < vertexElementCount; ++i)
	{
		uint32_t varyingOffset = getVaryingOffset(vertexElements[i].getDataUsage(), vertexElements[i].getIndex());
		outVertexVarying[varyingOffset] = Vector4((const float *)vertices[i + vertexOffset]);
	}

	checkVaryings(outVertexVarying);
}

void RenderViewSw::executeVertexShader(const varying_data_t& vertexVarying, varying_data_t& outInterpolatorVarying) /*const*/
{
	checkVaryings(vertexVarying);

	m_processor->execute(
		m_currentProgram->getVertexProgram(),
		m_currentProgram->getParameters(),		// Uniforms
		(const Vector4*)&vertexVarying,			// Vertex varyings
		m_targetSize,							// Target size
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
		float iw = 1.0f / position.w();

#if defined(_DEBUG)
		if (isInfinite(iw))
			iw = 1.0f;
#endif

		context.screen[i].x = (position.x() * iw * 0.5f + 0.5f) * rs.viewPort.width + rs.viewPort.left;
		context.screen[i].y = (0.5f - position.y() * iw * 0.5f) * rs.viewPort.height + rs.viewPort.top;
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

				// Clip to view planes.
				clipPlanes(context, 3);
				if (context.clippedCount < 3)
					continue;

				// Project onto near view plane, aka screen.
				projectScreen(context);

				// Inverse interpolators to prepare for perspective correction.
				for (uint32_t j = 0; j < context.clippedCount; ++j)
				{
					float iw = 1.0f / context.clippedVaryings[j][0].w();
#if defined(_DEBUG)
					if (isNan(iw) || isInfinite(iw))
						iw = 1.0f;
#endif

					context.clippedVaryings[j][0] =
						context.clippedVaryings[j][0].xyz0() + Vector4(0.0f, 0.0f, 0.0f, iw);

					for (uint32_t k = 1; k < sizeof(varying_data_t) / sizeof(Vector4); ++k)
						context.clippedVaryings[j][k] *= traktor::Scalar(iw);

					// Update dirty rectangle.
					//if (m_renderStateStack.size() <= 1)
					//{
					//	m_dirty[0] = std::min< int32_t >(m_dirty[0], floorf(context.screen[j].x));
					//	m_dirty[1] = std::min< int32_t >(m_dirty[1], floorf(context.screen[j].y));
					//	m_dirty[2] = std::max< int32_t >(m_dirty[2], ceilf(context.screen[j].x));
					//	m_dirty[3] = std::max< int32_t >(m_dirty[3], ceilf(context.screen[j].y));
					//}
				}

				// Check culling, rewind vertices if needed as triangle rasterizer
				// can only handle one type of winding.
				Vector2 l = context.screen[1] - context.screen[0];
				Vector2 r = context.screen[2] - context.screen[0];
				float nz = l.x * r.y - r.x * l.y;

				bool cullDiscard = false;
				bool cullReversed = false;
				switch (m_currentProgram->getRenderState().cullMode)
				{
				case PixelOutput::CmNever:
					if (nz > 0.0f)
						cullReversed = true;
					break;

				case PixelOutput::CmClockWise:
					if (nz > 0.0f)
						cullDiscard = true;
					break;

				case PixelOutput::CmCounterClockWise:
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
							triangle(Tc, this, &RenderViewSw::triangleShade, context);
						}
					}
					else
					{
						for (int k = 0; k < count2 - 2; ++k)
						{
							Vector2 Tc[] = { T2[0], T2[2 + k], T2[1 + k] };
							triangle(Tc, this, &RenderViewSw::triangleShade, context);
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
		//{
		//	for (uint32_t i = 0; i < set.count; ++i)
		//	{
		//		fetchVertex(set.offset + i * 2 + 0, context.vertexVaryings[0]);
		//		fetchVertex(set.offset + i * 2 + 1, context.vertexVaryings[1]);

		//		executeVertexShader(context.vertexVaryings[0], context.interpolatorVaryings[0]);
		//		executeVertexShader(context.vertexVaryings[1], context.interpolatorVaryings[1]);

		//		clipPlanes(context, 2);
		//		if (context.clippedCount < 2)
		//			continue;

		//		projectScreen(context);

		//		// Clip line to view port.
		//		Vector2 T1[8], T2[8];
		//		int count1, count2;
		//		count1 = clip< AxisX >(context.screen,      2, float(rs.viewPort.left),                     T1, std::greater_equal< float >());
		//		count2 = clip< AxisX >(T1,             count1, float(rs.viewPort.left + rs.viewPort.width), T2, std::less         < float >());
		//		count1 = clip< AxisY >(T2,             count2, float(rs.viewPort.top),                      T1, std::greater_equal< float >());
		//		count2 = clip< AxisY >(T1,             count1, float(rs.viewPort.top + rs.viewPort.height), T2, std::less         < float >());

		//		if (count2 >= 2)
		//			line(T2, this, &RenderViewSw::lineShade, context);
		//	}
		//}
		break;

	case PtTriangleStrip:
		{
			T_ASSERT (primitives.count > 0);
			for (uint32_t i = 1; i < primitives.count; ++i)
			{
			}
		}
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

				// Clip to view planes.
				clipPlanes(context, 3);
				if (context.clippedCount < 3)
					continue;

				// Project onto near view plane, aka screen.
				projectScreen(context);

				// Inverse interpolators to prepare for perspective correction.
				for (uint32_t j = 0; j < context.clippedCount; ++j)
				{
					float iw = 1.0f / context.clippedVaryings[j][0].w();
#if defined(_DEBUG)
					if (isNan(iw) || isInfinite(iw))
						iw = 1.0f;
#endif

					context.clippedVaryings[j][0] =
						context.clippedVaryings[j][0].xyz0() +
						Vector4(0.0f, 0.0f, 0.0f, iw);

					for (uint32_t k = 1; k < sizeof(varying_data_t) / sizeof(Vector4); ++k)
						context.clippedVaryings[j][k] *= traktor::Scalar(iw);

					// Update dirty rectangle.
					//if (m_renderStateStack.size() <= 1)
					//{
					//	m_dirty[0] = std::min< int32_t >(m_dirty[0], floorf(context.screen[j].x));
					//	m_dirty[1] = std::min< int32_t >(m_dirty[1], floorf(context.screen[j].y));
					//	m_dirty[2] = std::max< int32_t >(m_dirty[2], ceilf(context.screen[j].x));
					//	m_dirty[3] = std::max< int32_t >(m_dirty[3], ceilf(context.screen[j].y));
					//}
				}

				// Check culling, rewind vertices if needed as triangle rasterizer
				// can only handle one type of winding.
				Vector2 l = context.screen[1] - context.screen[0];
				Vector2 r = context.screen[2] - context.screen[0];
				float nz = l.x * r.y - r.x * l.y;

				bool cullDiscard = false;
				bool cullReversed = false;
				switch (m_currentProgram->getRenderState().cullMode)
				{
				case PixelOutput::CmNever:
					if (nz > 0.0f)
						cullReversed = true;
					break;

				case PixelOutput::CmClockWise:
					if (nz > 0.0f)
						cullDiscard = true;
					break;

				case PixelOutput::CmCounterClockWise:
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
					context.baryDenom[0] = 1.0f / ((T[1].y - T[2].y) * T[0].x + (T[2].x - T[1].x) * T[0].y + T[1].x * T[2].y - T[2].x * T[1].y);

					// f20(x1,y1) = (y2 - y0) * x1 + (x0 - x2) * y1 + x2 * y0 - x0 * y2
					context.baryDenom[1] = 1.0f / ((T[2].y - T[0].y) * T[1].x + (T[0].x - T[2].x) * T[1].y + T[2].x * T[0].y - T[0].x * T[2].y);

					// f01(x2,y2) = (y0 - y1) * x2 + (x1 - x0) * y2 + x0 * y1 - x1 * y0
					context.baryDenom[2] = 1.0f / ((T[0].y - T[1].y) * T[2].x + (T[1].x - T[0].x) * T[2].y + T[0].x * T[1].y - T[1].x * T[0].y);

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
							triangle(Tc, this, &RenderViewSw::triangleShade, context);
						}
					}
					else
					{
						for (int k = 0; k < count2 - 2; ++k)
						{
							Vector2 Tc[] = { T2[0], T2[2 + k], T2[1 + k] };
							triangle(Tc, this, &RenderViewSw::triangleShade, context);
						}
					}
				}
			}
		}
		break;
	}
}

void RenderViewSw::lineShade(const FragmentContext& context, int x, int y, float d)
{
//	RenderState& rs = m_renderStateStack.back();
//
//	Processor::image_t pixelProgram = m_currentProgram->getPixelProgram();
//	const Vector4* parameters = m_currentProgram->getParameters();
//	const Ref< AbstractSampler >* samplers = m_currentProgram->getSamplers();
//
//	uint32_t offset = x + y * rs.targetPitch;
//
//	varying_data_t pixelVaryings;
//	varying_data_t fragmentVaryings;
//
//	for (size_t i = 0; i < sizeof(varying_data_t) / sizeof(Vector4); ++i)
//		pixelVaryings[i] = context.interpolatorVaryings[0][i] + (context.interpolatorVaryings[1][i] - context.interpolatorVaryings[0][i]) * traktor::Scalar(d);
//
//	m_processor->execute(
//		pixelProgram,
//		parameters,
//		pixelVaryings,
//		samplers,
//		fragmentVaryings
//	);
//
//	Vector4& color = fragmentVaryings[0];
//
//	color = max(color, Vector4(0.0f, 0.0f, 0.0f, 0.0f));
//	color = min(color, Vector4(1.0f, 1.0f, 1.0f, 1.0f));
//	color *= traktor::Scalar(255.0f);
//
//	uint32_t target = 
//		(uint32_t(color.x()) << 16) |
//		(uint32_t(color.y()) << 8) |
//		(uint32_t(color.z()));
//
//	//uint16_t iw = (uint16_t)(iz1 + (iz2 - iz1) * t);
//	//if (/*!depthEnable || */iw <= rs.depthTarget[offset])
//	{
//		rs.colorTarget[offset] = target;
//		//if (depthWriteEnable)
//		//	rs.depthTarget[offset] = iw;
//	}
}

void RenderViewSw::triangleShade(const FragmentContext& context, int x1, int x2, int y)
{
	if (!context.blendEnable)
		triangleShadeOpaque(context, x1, x2, y);
	else
		triangleShadeBlend(context, x1, x2, y);
}

void RenderViewSw::triangleShadeOpaque(const FragmentContext& context, int x1, int x2, int y)
{
	RenderState& rs = m_renderStateStack.back();

	Processor::image_t pixelProgram = m_currentProgram->getPixelProgram();
	const Vector4* parameters = m_currentProgram->getParameters();
	const Ref< AbstractSampler >* samplers = m_currentProgram->getSamplers();
	uint32_t icount = m_currentProgram->getInterpolatorCount();

	uint32_t colorTargetOffset = x1 + y * rs.colorTargetPitch / sizeof(uint16_t);
	uint32_t depthTargetOffset = x1 + y * rs.depthTargetPitch / sizeof(uint16_t);

	// Calculate barycentric coordinates.
	const Vector2* T = context.triangle;
	const uint32_t* I = context.indices;

	// f12(x,y) = (y1 - y2) * x + (x2 - x1) * y + x1 * y2 - x2 * y1
	traktor::Scalar alpha1 = traktor::Scalar(((T[1].y - T[2].y) * x1 + (T[2].x - T[1].x) * y + context.baryOffset[0]) * context.baryDenom[0]);
	traktor::Scalar alpha2 = traktor::Scalar(((T[1].y - T[2].y) * x2 + (T[2].x - T[1].x) * y + context.baryOffset[0]) * context.baryDenom[0]);

	// f20(x,y) = (y2 - y0) * x + (x0 - x2) * y + x2 * y0 - x0 * y2
	traktor::Scalar beta1 = traktor::Scalar(((T[2].y - T[0].y) * x1 + (T[0].x - T[2].x) * y + context.baryOffset[1]) * context.baryDenom[1]);
	traktor::Scalar beta2 = traktor::Scalar(((T[2].y - T[0].y) * x2 + (T[0].x - T[2].x) * y + context.baryOffset[1]) * context.baryDenom[1]);

	// f01(x,y) = (y0 - y1) * x + (x1 - x0) * y + x0 * y1 - x1 * y0
	traktor::Scalar gamma1 = traktor::Scalar(((T[0].y - T[1].y) * x1 + (T[1].x - T[0].x) * y + context.baryOffset[2]) * context.baryDenom[2]);
	traktor::Scalar gamma2 = traktor::Scalar(((T[0].y - T[1].y) * x2 + (T[1].x - T[0].x) * y + context.baryOffset[2]) * context.baryDenom[2]);

	// Calculate varyings.
	T_ALIGN16 varying_data_t surfaceInterpolators[2];
	T_ALIGN16 varying_data_t pixelVaryings;
	T_ALIGN16 varying_data_t fragmentVaryings;

	surfaceInterpolators[0][0] = context.clippedVaryings[I[0]][0] * alpha1 + context.clippedVaryings[I[1]][0] * beta1 + context.clippedVaryings[I[2]][0] * gamma1;
	surfaceInterpolators[1][0] = context.clippedVaryings[I[0]][0] * alpha2 + context.clippedVaryings[I[1]][0] * beta2 + context.clippedVaryings[I[2]][0] * gamma2;

	traktor::Scalar iw1 = surfaceInterpolators[0][0].w();
	traktor::Scalar iw2 = surfaceInterpolators[1][0].w();
	traktor::Scalar ix = traktor::Scalar(1.0f / float(x2 - x1));

	// Only interpolate DuCustom across surface.
	for (uint32_t i = 6; i < 6 + icount; ++i)
	{
		surfaceInterpolators[0][i] = context.clippedVaryings[I[0]][i] * alpha1 + context.clippedVaryings[I[1]][i] * beta1 + context.clippedVaryings[I[2]][i] * gamma1;
		surfaceInterpolators[1][i] = context.clippedVaryings[I[0]][i] * alpha2 + context.clippedVaryings[I[1]][i] * beta2 + context.clippedVaryings[I[2]][i] * gamma2;
		surfaceInterpolators[1][i] = (surfaceInterpolators[1][i] - surfaceInterpolators[0][i]) * ix;
	}

	uint16_t fragmentColor = 0;

	for (int x = 0; x < x2 - x1; ++x, ++colorTargetOffset, ++depthTargetOffset)
	{
		traktor::Scalar t = traktor::Scalar(x) * ix;
		traktor::Scalar iw = iw1 + (iw2 - iw1) * traktor::Scalar(t);
		uint16_t fragmentDepth = toDepth(iw);

		for (uint32_t i = 6; i < 6 + icount; ++i)
		{
			pixelVaryings[i] = surfaceInterpolators[0][i] / iw;
			surfaceInterpolators[0][i] += surfaceInterpolators[1][i];
		}

		if (!context.depthEnable || fragmentDepth > rs.depthTarget[depthTargetOffset])
		{
			// Execute fragment program every Nth pixel.
#if defined(WINCE)
			if ((x & 3) == 0)
#else
			if ((x & 1) == 0)
#endif
			{
				m_processor->execute(
					pixelProgram,
					parameters,
					pixelVaryings,
					m_targetSize,
					samplers,
					fragmentVaryings
				);

				// Write color to current target.
				Vector4& color = fragmentVaryings[0];
				color = max(color, Vector4(0.0f, 0.0f, 0.0f, 0.0f));
				color = min(color, Vector4(1.0f, 1.0f, 1.0f, 1.0f));
				fragmentColor = to565(color);
			}

			rs.colorTarget[colorTargetOffset] = fragmentColor;

			if (context.depthWriteEnable)
				rs.depthTarget[depthTargetOffset] = fragmentDepth;
		}
	}
}

void RenderViewSw::triangleShadeBlend(const FragmentContext& context, int x1, int x2, int y)
{
//	RenderState& rs = m_renderStateStack.back();
//
//	Processor::image_t pixelProgram = m_currentProgram->getPixelProgram();
//	const Vector4* parameters = m_currentProgram->getParameters();
//	const Ref< AbstractSampler >* samplers = m_currentProgram->getSamplers();
//	uint32_t icount = m_currentProgram->getInterpolatorCount();
//	uint32_t offset = x1 + y * rs.targetPitch;
//
//	// Calculate barycentric coordinates.
//	const Vector2* T = context.triangle;
//	const uint32_t* I = context.indices;
//
//	// f12(x,y) = (y1 - y2) * x + (x2 - x1) * y + x1 * y2 - x2 * y1
//	traktor::Scalar alpha1 = traktor::Scalar(((T[1].y - T[2].y) * x1 + (T[2].x - T[1].x) * y + context.baryOffset[0]) * context.baryDenom[0]);
//	traktor::Scalar alpha2 = traktor::Scalar(((T[1].y - T[2].y) * x2 + (T[2].x - T[1].x) * y + context.baryOffset[0]) * context.baryDenom[0]);
//
//	// f20(x,y) = (y2 - y0) * x + (x0 - x2) * y + x2 * y0 - x0 * y2
//	traktor::Scalar beta1 = traktor::Scalar(((T[2].y - T[0].y) * x1 + (T[0].x - T[2].x) * y + context.baryOffset[1]) * context.baryDenom[1]);
//	traktor::Scalar beta2 = traktor::Scalar(((T[2].y - T[0].y) * x2 + (T[0].x - T[2].x) * y + context.baryOffset[1]) * context.baryDenom[1]);
//
//	// f01(x,y) = (y0 - y1) * x + (x1 - x0) * y + x0 * y1 - x1 * y0
//	traktor::Scalar gamma1 = traktor::Scalar(((T[0].y - T[1].y) * x1 + (T[1].x - T[0].x) * y + context.baryOffset[2]) * context.baryDenom[2]);
//	traktor::Scalar gamma2 = traktor::Scalar(((T[0].y - T[1].y) * x2 + (T[1].x - T[0].x) * y + context.baryOffset[2]) * context.baryDenom[2]);
//
//	// Calculate surface interpolators.
//	varying_data_t surfaceInterpolators[2];
//	varying_data_t pixelVaryings;
//	varying_data_t fragmentVaryings;
//
//	surfaceInterpolators[0][0] = context.clippedVaryings[I[0]][0] * alpha1 + context.clippedVaryings[I[1]][0] * beta1 + context.clippedVaryings[I[2]][0] * gamma1;
//	surfaceInterpolators[1][0] = context.clippedVaryings[I[0]][0] * alpha2 + context.clippedVaryings[I[1]][0] * beta2 + context.clippedVaryings[I[2]][0] * gamma2;
//
//	traktor::Scalar iw1 = surfaceInterpolators[0][0].w();
//	traktor::Scalar iw2 = surfaceInterpolators[1][0].w();
//	traktor::Scalar ix = traktor::Scalar(1.0f / float(x2 - x1));
//
//	// Only interpolate DuCustom across surface.
//	for (uint32_t i = 6; i < 6 + icount; ++i)
//	{
//		surfaceInterpolators[0][i] = context.clippedVaryings[I[0]][i] * alpha1 + context.clippedVaryings[I[1]][i] * beta1 + context.clippedVaryings[I[2]][i] * gamma1;
//		surfaceInterpolators[1][i] = context.clippedVaryings[I[0]][i] * alpha2 + context.clippedVaryings[I[1]][i] * beta2 + context.clippedVaryings[I[2]][i] * gamma2;
//		surfaceInterpolators[1][i] = (surfaceInterpolators[1][i] - surfaceInterpolators[0][i]) * ix;
//	}
//
//	for (int x = x1; x < x2; ++x, ++offset)
//	{
//		traktor::Scalar t = traktor::Scalar(x - x1) * ix;
//		traktor::Scalar iw = iw1 + (iw2 - iw1) * t;
//
//		if (!context.depthEnable || iw > rs.depthTarget[offset])
//		{
//			// Calculate per pixel varyings.
//			for (uint32_t i = 6; i < 6 + icount; ++i)
//			{
//				pixelVaryings[i] = surfaceInterpolators[0][i] / iw;
//				surfaceInterpolators[0][i] += surfaceInterpolators[1][i];
//			}
//
//		// Execute pixel program.
//			m_processor->execute(
//				pixelProgram,
//				parameters,
//				pixelVaryings,
//				m_targetSize,
//				samplers,
//				fragmentVaryings
//			);
//
//			// Write color to current target.
//			Vector4& color = fragmentVaryings[0];
//
//			uint32_t target = rs.colorTarget[offset];
//
//			Vector4 destination(
//				((target >> 16) & 255) / 255.0f,
//				((target >> 8) & 255) / 255.0f,
//				(target & 255) / 255.0f,
//				((target >> 24) & 255) / 255.0f
//			);
//
//			switch (m_currentProgram->getRenderState().blendSource)
//			{
//			case PixelOutput::BfOne:
//				break;
//
//			case PixelOutput::BfZero:
//				color.set(0.0f, 0.0f, 0.0f, 0.0f);
//				break;
//
//			case PixelOutput::BfSourceColor:
//				color *= color;
//				break;
//
//			case PixelOutput::BfOneMinusSourceColor:
//				color *= traktor::Scalar(1.0f) - color;
//				break;
//
//			case PixelOutput::BfDestinationColor:
//				color *= destination;
//				break;
//
//			case PixelOutput::BfOneMinusDestinationColor:
//				color *= traktor::Scalar(1.0f) - destination;
//				break;
//
//			case PixelOutput::BfSourceAlpha:
//				color *= color.w();
//				break;
//
//			case PixelOutput::BfOneMinusSourceAlpha:
//				color *= traktor::Scalar(1.0f) - color.w();
//				break;
//
//			case PixelOutput::BfDestinationAlpha:
//				color *= destination.w();
//				break;
//
//			case PixelOutput::BfOneMinusDestinationAlpha:
//				color *= traktor::Scalar(1.0f) - destination.w();
//				break;
//			}
//			
//			switch (m_currentProgram->getRenderState().blendDestination)
//			{
//			case PixelOutput::BfOne:
//				break;
//
//			case PixelOutput::BfZero:
//				destination.set(0.0f, 0.0f, 0.0f, 0.0f);
//				break;
//
//			case PixelOutput::BfSourceColor:
//				destination *= color;
//				break;
//
//			case PixelOutput::BfOneMinusSourceColor:
//				destination *= traktor::Scalar(1.0f) - color;
//				break;
//
//			case PixelOutput::BfDestinationColor:
//				destination *= destination;
//				break;
//
//			case PixelOutput::BfOneMinusDestinationColor:
//				destination *= traktor::Scalar(1.0f) - destination;
//				break;
//
//			case PixelOutput::BfSourceAlpha:
//				destination *= color.w();
//				break;
//
//			case PixelOutput::BfOneMinusSourceAlpha:
//				destination *= traktor::Scalar(1.0f) - color.w();
//				break;
//
//			case PixelOutput::BfDestinationAlpha:
//				destination *= destination.w();
//				break;
//
//			case PixelOutput::BfOneMinusDestinationAlpha:
//				destination *= traktor::Scalar(1.0f) - destination.w();
//				break;
//			}
//
//			switch (m_currentProgram->getRenderState().blendOperation)
//			{
//			case PixelOutput::BoAdd:
//				color = color + destination;
//				break;
//
//			case PixelOutput::BoSubtract:
//				color = color - destination;
//				break;
//
//			case PixelOutput::BoReverseSubtract:
//				color = destination - color;
//				break;
//			}
//
//			color = max(color, Vector4(0.0f, 0.0f, 0.0f, 0.0f));
//			color = min(color, Vector4(1.0f, 1.0f, 1.0f, 1.0f));
//			color *= traktor::Scalar(255.0f);
//
//			target = 
//				(uint32_t(color.x()) << 16) |
//				(uint32_t(color.y()) << 8) |
//				(uint32_t(color.z()));
//
//			rs.colorTarget[offset] = target;
//
//			if (context.depthWriteEnable)
//				rs.depthTarget[offset] = iw;
//		}
//	}
}

	}
}
