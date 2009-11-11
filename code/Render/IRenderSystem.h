#ifndef traktor_render_IRenderSystem_H
#define traktor_render_IRenderSystem_H

#include <vector>
#include "Core/Heap/Ref.h"
#include "Core/Object.h"
#include "Render/ITexture.h"
#include "Render/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class DisplayMode;
class IRenderView;
class VertexElement;
class VertexBuffer;
class IndexBuffer;
class ISimpleTexture;
class ICubeTexture;
class IVolumeTexture;
class RenderTargetSet;
class ShaderGraph;
class ProgramResource;
class IProgram;

/*! \brief Render system interface.
 * \ingroup Render
 *
 * The render system class is an abstraction of
 * the underlying system used.
 */
class T_DLLCLASS IRenderSystem : public Object
{
	T_RTTI_CLASS(IRenderSystem)

public:
	/*! \name Render system creation. */
	//@{

	/*! \brief Create render system.
	 *
	 * \return True if successfully created.
	 */
	virtual bool create() = 0;

	/*! \brief Destroy render system. */
	virtual void destroy() = 0;

	//@}

	/*! \name Display mode enumeration. */
	//@{

	/*! \brief Get number of available display modes.
	 *
	 * Return number of available display modes,
	 * preferably display modes supported by both graphics card
	 * and monitor.
	 */
	virtual int getDisplayModeCount() const = 0;
	
	/*! \brief Get display mode.
	 *
	 * Get information about display mode from index 0 - (getDisplayMode() - 1).
	 */
	virtual Ref< DisplayMode > getDisplayMode(int index) = 0;
	
	/*! \brief Get current display mode.
	 *
	 * Get information about currently set display mode.
	 */
	virtual Ref< DisplayMode > getCurrentDisplayMode() = 0;
	
	/*! \brief Find supported display mode from criterion.
	 *
	 * Find best matching and supported display mode from a criterion.
	 * Using a simple heuristics to find best matching mode, width + height + color depth * 10.
	 */
	virtual Ref< DisplayMode > findDisplayMode(const DisplayMode* criteria);
	
	//@}

	/*! \name System message handling. */
	//@{

	/*! \brief Handle system messages.
	 *
	 * When rendering to a full screen view this must be called frequently
	 * in order for the render systems to handle OS messages.
	 * \return false if OS terminated application.
	 */
	virtual bool handleMessages() = 0;

	//@}
	
	/*! \name Render view creation. */
	//@{
	
	/*! \brief Create full screen render view. */
	virtual Ref< IRenderView > createRenderView(const DisplayMode* displayMode, const RenderViewCreateDesc& desc) = 0;

	/*! \brief Create windowed render view. */
	virtual Ref< IRenderView > createRenderView(void* windowHandle, const RenderViewCreateDesc& desc) = 0;
	
	//@}

	/*! \name Factory methods. */
	//@{

	/*! \brief Create vertex buffer.
	 *
	 * \param vertexElements Vertex element declaration.
	 * \param bufferSize Size of vertex buffer in bytes.
	 * \param dynamic If vertex buffer is frequently updated.
	 */
	virtual Ref< VertexBuffer > createVertexBuffer(const std::vector< VertexElement >& vertexElements, uint32_t bufferSize, bool dynamic) = 0;

	/*! \brief Create index buffer.
	 *
	 * \param indexType Type of index, 16 or 32 bit.
	 * \param bufferSize Size of index buffer in bytes.
	 * \param dynamic If index buffer is frequently updated.
	 */
	virtual Ref< IndexBuffer > createIndexBuffer(IndexType indexType, uint32_t bufferSize, bool dynamic) = 0;

	/*! \brief Create simple, 2d, texture. */
	virtual Ref< ISimpleTexture > createSimpleTexture(const SimpleTextureCreateDesc& desc) = 0;

	/*! \brief Create cube texture. */
	virtual Ref< ICubeTexture > createCubeTexture(const CubeTextureCreateDesc& desc) = 0;
	
	/*! \brief Create volume texture. */
	virtual Ref< IVolumeTexture > createVolumeTexture(const VolumeTextureCreateDesc& desc) = 0;

	/*! \brief Create render target set. */
	virtual Ref< RenderTargetSet > createRenderTargetSet(const RenderTargetSetCreateDesc& desc) = 0;

	/*! \brief Compile "simple" shader graph into a render program resource.
	 *
	 * \param shaderGraph Shader graph instance.
	 * \param optimize Level of optimization (0-4; 0 = No optimization; 4 = Maximum optimization).
	 * \param validate Validate generated shader to known capabilities.
	 * \return Compiled program resource.
	 */
	virtual Ref< ProgramResource > compileProgram(const ShaderGraph* shaderGraph, int optimize, bool validate) = 0;

	/*! \brief Create shader from shader resource.
	 *
	 * \param shaderResource Compiled shader resource.
	 * \return Shader suitable for rendering with this render system.
	 */
	virtual Ref< IProgram > createProgram(const ProgramResource* programResource) = 0;

	//@}
};

	}
}

#endif	// traktor_render_IRenderSystem_H
