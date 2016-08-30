#ifndef traktor_render_RenderViewSw_H
#define traktor_render_RenderViewSw_H

#include "Core/Math/Vector2.h"
#include "Core/Math/Vector4.h"
#include "Graphics/ISurface.h"
#include "Render/IRenderView.h"
#include "Render/Sw/VaryingUtils.h"

namespace traktor
{
	namespace graphics
	{

class IGraphicsSystem;

	}

	namespace render
	{

class RenderSystemSw;
class RenderTargetSetSw;
class VertexBufferSw;
class IndexBufferSw;
class ProgramSw;
class Processor;

/*!
 * \ingroup SW
 */
class RenderViewSw : public IRenderView
{
	T_RTTI_CLASS;

public:
	RenderViewSw(RenderSystemSw* renderSystem, graphics::IGraphicsSystem* graphicsSystem, Processor* processor);

	virtual ~RenderViewSw();

	virtual bool nextEvent(RenderEvent& outEvent) T_OVERRIDE T_FINAL;

	virtual void close() T_OVERRIDE T_FINAL;

	virtual bool reset(const RenderViewDefaultDesc& desc) T_OVERRIDE T_FINAL;

	virtual bool reset(int32_t width, int32_t height) T_OVERRIDE T_FINAL;

	virtual int getWidth() const T_OVERRIDE T_FINAL;

	virtual int getHeight() const T_OVERRIDE T_FINAL;

	virtual bool isActive() const T_OVERRIDE T_FINAL;

	virtual bool isMinimized() const T_OVERRIDE T_FINAL;

	virtual bool isFullScreen() const T_OVERRIDE T_FINAL;

	virtual void showCursor() T_OVERRIDE T_FINAL;

	virtual void hideCursor() T_OVERRIDE T_FINAL;

	virtual bool isCursorVisible() const T_OVERRIDE T_FINAL;

	virtual bool setGamma(float gamma) T_OVERRIDE T_FINAL;

	virtual void setViewport(const Viewport& viewport) T_OVERRIDE T_FINAL;

	virtual Viewport getViewport() T_OVERRIDE T_FINAL;

	virtual SystemWindow getSystemWindow() T_OVERRIDE T_FINAL;

	virtual bool begin(EyeType eye) T_OVERRIDE T_FINAL;

	virtual bool begin(RenderTargetSet* renderTargetSet) T_OVERRIDE T_FINAL;

	virtual bool begin(RenderTargetSet* renderTargetSet, int renderTarget) T_OVERRIDE T_FINAL;

	virtual void clear(uint32_t clearMask, const Color4f* colors, float depth, int32_t stencil) T_OVERRIDE T_FINAL;

	virtual void draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives) T_OVERRIDE T_FINAL;

	virtual void draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives, uint32_t instanceCount) T_OVERRIDE T_FINAL;

	virtual void end() T_OVERRIDE T_FINAL;

	virtual void present() T_OVERRIDE T_FINAL;

	virtual void pushMarker(const char* const marker) T_OVERRIDE T_FINAL;

	virtual void popMarker() T_OVERRIDE T_FINAL;

	virtual void getStatistics(RenderViewStatistics& outStatistics) const T_OVERRIDE T_FINAL;

	virtual bool getBackBufferContent(void* buffer) const T_OVERRIDE T_FINAL;

private:
	struct RenderState
	{
		Viewport viewPort;
		uint32_t width;
		uint32_t height;
		uint32_t* colorTarget;
		uint32_t colorTargetPitch;		//< Color target pitch in bytes.
		float* depthTarget;
		uint32_t depthTargetPitch;		//< Depth target pitch in bytes.
		uint8_t* stencilTarget;
		uint32_t stencilTargetPitch;	//< Stencil target pitch in bytes.
	};

	struct FragmentContext
	{
		T_ALIGN16 varying_data_t vertexVaryings[3];			//< From vertex stream for each triangle
		T_ALIGN16 varying_data_t interpolatorVaryings[3];	//< Post vertex program
		T_ALIGN16 varying_data_t clippedVaryings[8];		//< Post view plane clip
		uint32_t clippedCount;
		Vector2 screen[8];
		const Vector2* triangle;
		uint32_t indices[3];
		float baryDenom[3];
		float baryOffset[3];
		bool depthEnable;
		bool depthWriteEnable;
		bool blendEnable;

		FragmentContext()
		:	clippedCount(0)
		,	triangle(0)
		,	depthEnable(false)
		,	depthWriteEnable(false)
		,	blendEnable(false)
		{
			std::memset(vertexVaryings, 0, sizeof(vertexVaryings));
			std::memset(interpolatorVaryings, 0, sizeof(interpolatorVaryings));
			std::memset(clippedVaryings, 0, sizeof(clippedVaryings));
		}
	};

	Ref< RenderSystemSw > m_renderSystem;
	Ref< graphics::IGraphicsSystem > m_graphicsSystem;
	Ref< Processor > m_processor;

	/*! \name Primary render target surfaces. */
	//@{
	Ref< graphics::ISurface > m_frameBufferSurface;
	graphics::SurfaceDesc m_frameBufferSurfaceDesc;
	Ref< RenderTargetSetSw > m_primaryTarget;
	//@}

	std::vector< RenderState > m_renderStateStack;
	Ref< VertexBufferSw > m_currentVertexBuffer;
	Ref< IndexBufferSw > m_currentIndexBuffer;
	Ref< ProgramSw > m_currentProgram;
	Viewport m_viewPort;
	Vector4 m_targetSize;
	int32_t m_instance;
	uint32_t m_drawCalls;
	uint32_t m_primitiveCount;

	void fetchVertex(uint32_t index, varying_data_t& outVertexVarying) const;

	void executeVertexShader(const varying_data_t& vertexVarying, varying_data_t& outInterpolatorVarying) /*const*/;

	void clipPlanes(FragmentContext& context, uint32_t vertexCount) const;

	void projectScreen(FragmentContext& context) const;

	void drawIndexed(const Primitives& primitives);

	void drawNonIndexed(const Primitives& primitives);

	void triangleShadeOpaque(const FragmentContext& context, int x1, int x2, int y);

	void triangleShadeBlend(const FragmentContext& context, int x1, int x2, int y);
};

	}
}

#endif	// traktor_render_RenderViewSw_H
