#ifndef traktor_render_RenderViewPs3_H
#define traktor_render_RenderViewPs3_H

#include <list>
#include "Core/Heap/Ref.h"
#include "Render/RenderView.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_PS3_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class RenderSystemPs3;
class VertexBufferPs3;
class IndexBufferPs3;
class ShaderPs3;

class T_DLLCLASS RenderViewPs3 : public RenderView
{
	T_RTTI_CLASS(RenderViewPs3)

public:
	RenderViewPs3(RenderSystemPs3* renderSystem);

	virtual ~RenderViewPs3();

	bool create(int resolutionId, int width, int height);

	virtual void close();

	virtual void resize(int width, int height);

	virtual void setViewport(const Viewport& viewport);

	virtual Viewport getViewport();

	virtual bool begin(RenderTarget* renderTarget, RenderMode renderMode);

	virtual void clear(unsigned int clearMask, unsigned int color, float depth, int stencil);

	virtual void setVertexBuffer(VertexBuffer* vertexBuffer);
	
	virtual void setIndexBuffer(IndexBuffer* indexBuffer);

	virtual void setShader(Shader* shader);
	
	virtual void draw(const Primitives& primitives);

	virtual void end();

	virtual void present();

private:
	struct RenderState
	{
		Viewport viewport;
		uint32_t width;
		uint32_t height;
		uint32_t colorFormat;
		uint32_t colorOffset;
		uint32_t colorPitch;
		uint32_t depthOffset;
		uint32_t depthPitch;
		RenderMode mode;
	};

	Ref< RenderSystemPs3 > m_renderSystem;
	Ref< VertexBufferPs3 > m_currentVertexBuffer;
	Ref< IndexBufferPs3 > m_currentIndexBuffer;
	Ref< ShaderPs3 > m_currentShader;

	int m_width;
	int m_height;

	Viewport m_viewport;

	void* m_colorAddr[2];
	uint32_t m_colorOffset[2];
	uint32_t m_colorPitch;

	void* m_depthAddr;
	uint32_t m_depthOffset;
	uint32_t m_depthPitch;

	volatile uint32_t* m_frameSyncLabelData;
	uint32_t m_frameCounter;

	volatile uint32_t* m_targetSyncLabelData;
	uint32_t m_targetCounter;

	std::list< RenderState > m_renderStateStack;

	void setCurrentRenderState();
};

	}
}

#endif	// traktor_render_RenderViewPs3_H
