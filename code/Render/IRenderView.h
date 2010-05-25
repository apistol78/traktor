#ifndef traktor_render_IRenderView_H
#define traktor_render_IRenderView_H

#include "Core/Object.h"
#include "Render/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class IRenderSystem;
class VertexBuffer;
class IndexBuffer;
class ITexture;
class ISimpleTexture;
class ICubeTexture;
class IVolumeTexture;
class RenderTargetSet;
class IProgram;

#if defined(T_USE_RENDER_MARKERS)
#	define T_RENDER_PUSH_MARKER(renderView, marker) \
	(renderView)->pushMarker(marker)
#	define T_RENDER_POP_MARKER(renderView) \
	(renderView)->popMarker()
#else
#	define T_RENDER_PUSH_MARKER(renderView, marker)
#	define T_RENDER_POP_MARKER(renderView)
#endif

/*! \brief Render view.
 * \ingroup Render
 */
class T_DLLCLASS IRenderView : public Object
{
	T_RTTI_CLASS;

public:
	/*! \name View management. */
	//@{

	virtual void close() = 0;

	virtual bool reset(const RenderViewDefaultDesc& desc) = 0;

	virtual void resize(int32_t width, int32_t height) = 0;

	virtual int getWidth() const = 0;

	virtual int getHeight() const = 0;

	virtual bool isActive() const = 0;

	virtual bool isFullScreen() const = 0;

	virtual void setViewport(const Viewport& viewport) = 0;

	virtual Viewport getViewport() = 0;

	//@}

	/*! \name Rendering methods. */
	//@{

	/*! \brief Begin rendering to back buffer.
	 *
	 * \return True if successful.
	 */
	virtual bool begin() = 0;

	/*! \brief Begin rendering to a render target set.
	 *
	 * \param renderTargetSet Set of render targets.
	 * \param renderTarget Index of render target in set.
	 * \param keepDepthStencil Keep bound depth/stencil surface.
	 * \return True if successful.
	 */
	virtual bool begin(RenderTargetSet* renderTargetSet, int renderTarget, bool keepDepthStencil) = 0;

	/*! \brief Clear current target.
	 *
	 * \param clearMask Combination of ClearFlags.
	 * \param color Clear color value.
	 * \param depth Clear depth value.
	 * \param stencil Clear stencil value.
	 */
	virtual void clear(uint32_t clearMask, const float color[4], float depth, int32_t stencil) = 0;

	/*! \brief Set active vertex buffer.
	 *
	 * \param vertexBuffer Vertex buffer to set as active.
	 */
	virtual void setVertexBuffer(VertexBuffer* vertexBuffer) = 0;
	
	/*! \brief Set active index buffer. 
	 *
	 * \param indexBuffer Index buffer to set as active.
	 */
	virtual void setIndexBuffer(IndexBuffer* indexBuffer) = 0;
	
	/*! \brief Set active program.
	 *
	 * \param program Program to set as active.
	 */
	virtual void setProgram(IProgram* program) = 0;

	/*! \brief Draw primitives.
	 *
	 * \param primitives Set of primitives to render.
	 */
	virtual void draw(const Primitives& primitives) = 0;
	
	/*! \brief End rendering. */
	virtual void end() = 0;

	/*! \brief Swap back and front buffers. */
	virtual void present() = 0;

	/*! \brief Write push debug marker to command buffer. */
	virtual void pushMarker(const char* const marker) = 0;

	/*! \brief Write pop debug marker to command buffer. */
	virtual void popMarker() = 0;
};

	}
}

#endif	// traktor_render_IRenderView_H
