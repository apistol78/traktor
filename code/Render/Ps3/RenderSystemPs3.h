#ifndef traktor_render_RenderSystemPs3_H
#define traktor_render_RenderSystemPs3_H

#include "Render/RenderSystem.h"

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

class T_DLLCLASS RenderSystemPs3 : public RenderSystem
{
	T_RTTI_CLASS(RenderSystemPs3)

public:
	RenderSystemPs3();

	virtual ~RenderSystemPs3();

	virtual int getDisplayModeCount() const;
	
	virtual DisplayMode* getDisplayMode(int index);
	
	virtual DisplayMode* getCurrentDisplayMode();

	virtual bool handleMessages();

	virtual RenderView* createRenderView(DisplayMode* displayMode, int depthBits, int stencilBits, int multiSample, bool waitVBlank);

	virtual RenderView* createRenderView(void* windowHandle, int depthBits, int stencilBits, int multiSample);

	virtual VertexBuffer* createVertexBuffer(const std::vector< VertexElement >& vertexElements, int bufferSize, bool dynamic);

	virtual IndexBuffer* createIndexBuffer(IndexType indexType, int bufferSize, bool dynamic);

	virtual SimpleTexture* createSimpleTexture(const SimpleTextureCreateDesc& desc);

	virtual CubeTexture* createCubeTexture(const CubeTextureCreateDesc& desc);
	
	virtual VolumeTexture* createVolumeTexture(const VolumeTextureCreateDesc& desc);

	virtual RenderTarget* createRenderTarget(const RenderTargetCreateDesc& desc);

	virtual Shader* createShader(ShaderGraph* shaderGraph);
};

	}
}

#endif	// traktor_render_RenderSystemPs3_H
