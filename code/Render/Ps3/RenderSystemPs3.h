#ifndef traktor_render_RenderSystemPs3_H
#define traktor_render_RenderSystemPs3_H

#include "Core/Thread/Semaphore.h"
#include "Render/IRenderSystem.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_PS3_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class MemoryHeap;

class T_DLLCLASS RenderSystemPs3 : public IRenderSystem
{
	T_RTTI_CLASS;

public:
	RenderSystemPs3();

	virtual ~RenderSystemPs3();

	virtual bool create(const RenderSystemCreateDesc& desc);

	virtual void destroy();

	virtual uint32_t getDisplayModeCount() const;
	
	virtual DisplayMode getDisplayMode(uint32_t index) const;
	
	virtual DisplayMode getCurrentDisplayMode() const;

	virtual bool handleMessages();

	virtual Ref< IRenderView > createRenderView(const RenderViewCreateDefaultDesc& desc);

	virtual Ref< IRenderView > createRenderView(const RenderViewCreateEmbeddedDesc& desc);

	virtual Ref< VertexBuffer > createVertexBuffer(const std::vector< VertexElement >& vertexElements, uint32_t bufferSize, bool dynamic);

	virtual Ref< IndexBuffer > createIndexBuffer(IndexType indexType, uint32_t bufferSize, bool dynamic);

	virtual Ref< ISimpleTexture > createSimpleTexture(const SimpleTextureCreateDesc& desc);

	virtual Ref< ICubeTexture > createCubeTexture(const CubeTextureCreateDesc& desc);
	
	virtual Ref< IVolumeTexture > createVolumeTexture(const VolumeTextureCreateDesc& desc);

	virtual Ref< RenderTargetSet > createRenderTargetSet(const RenderTargetSetCreateDesc& desc);

	virtual Ref< IProgram > createProgram(const ProgramResource* programResource);

	virtual Ref< IProgramCompiler > createProgramCompiler() const;

	void compactHeaps();

	void acquireLock() { m_lock.wait(); }

	void releaseLock() { m_lock.release(); }

private:
	Semaphore m_lock;
	Ref< MemoryHeap > m_memoryHeapLocal;
	Ref< MemoryHeap > m_memoryHeapMain;
};

	}
}

#endif	// traktor_render_RenderSystemPs3_H
