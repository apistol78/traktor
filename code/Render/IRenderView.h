#ifndef traktor_render_IRenderView_H
#define traktor_render_IRenderView_H

#include "Core/Object.h"
#include "Core/Platform.h"
#include "Core/Math/Color4f.h"
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

#define T_USE_RENDER_MARKERS

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

	/*! \brief Get next system event.
	 *
	 * When using non-embedded render view this must
	 * be called frequently in order to handle OS
	 * events. Significant events are translated into
	 * platform agnostic events, such as "toggle fullscreen
	 * requested", "window resized" etc.
	 *
	 * \return True if event returned; false if no event, ie. idle.
	 */
	virtual bool nextEvent(RenderEvent& outEvent) = 0;

	virtual void close() = 0;

	virtual bool reset(const RenderViewDefaultDesc& desc) = 0;

	virtual bool reset(int32_t width, int32_t height) = 0;

	virtual int getWidth() const = 0;

	virtual int getHeight() const = 0;

	virtual bool isActive() const = 0;

	virtual bool isFullScreen() const = 0;

	virtual void showCursor() = 0;

	virtual void hideCursor() = 0;

	virtual bool setGamma(float gamma) = 0;

	virtual void setViewport(const Viewport& viewport) = 0;

	virtual Viewport getViewport() = 0;

	virtual SystemWindow getSystemWindow() = 0;

	//@}

	/*! \name Rendering methods. */
	//@{

	/*! \brief Begin rendering to back buffer.
	 *
	 * \param eye Left or right eye.
	 * \return True if successful.
	 */
	virtual bool begin(EyeType eye) = 0;

	/*! \brief Begin rendering to all render targets in set.
	 *
	 * \param renderTargetSet Set of render targets.
	 * \return True if successful.
	 */
	virtual bool begin(RenderTargetSet* renderTargetSet) = 0;

	/*! \brief Begin rendering to a render target set.
	 *
	 * \param renderTargetSet Set of render targets.
	 * \param renderTarget Index of render target in set.
	 * \return True if successful.
	 */
	virtual bool begin(RenderTargetSet* renderTargetSet, int renderTarget) = 0;

	/*! \brief Clear current target.
	 *
	 * \param clearMask Combination of ClearFlags.
	 * \param colors Clear color values; must be one color for each bound target.
	 * \param depth Clear depth value.
	 * \param stencil Clear stencil value.
	 */
	virtual void clear(uint32_t clearMask, const Color4f* colors, float depth, int32_t stencil) = 0;

	/*! \brief Draw primitives.
	 *
	 * \param vertexBuffer Vertex buffer containing geometry.
	 * \param indexBuffer Index buffer to be used; null if no indices should be used.
	 * \param program Program to be used.
	 * \param primitives Set of primitives to render.
	 */
	virtual void draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives) = 0;

	/*! \brief Draw primitives using instancing.
	 *
	 * \param vertexBuffer Vertex buffer containing geometry.
	 * \param indexBuffer Index buffer to be used; null if no indices should be used.
	 * \param program Program to be used.
	 * \param primitives Set of primitives to render.
	 * \param instanceCount Number of instances.
	 */
	virtual void draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives, uint32_t instanceCount) = 0;

	/*! \brief End rendering. */
	virtual void end() = 0;

	/*! \brief Swap back and front buffers. */
	virtual void present() = 0;

	/*! \name Statistics. */
	//@{

	/*! \brief Write push debug marker to command buffer. */
	virtual void pushMarker(const char* const marker) = 0;

	/*! \brief Write pop debug marker to command buffer. */
	virtual void popMarker() = 0;

	/*! \brief Get render view statistics. */
	virtual void getStatistics(RenderViewStatistics& outStatistics) const = 0;

	/*! \brief Get backbuffer content. */
	virtual bool getBackBufferContent(void* buffer) const = 0;

	//@}
};

	}
}

#endif	// traktor_render_IRenderView_H
