#pragma once

#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"
#include "Render/ITexture.h"
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

class IndexBuffer;
class ICubeTexture;
class IProgram;
class IRenderTargetSet;
class IRenderView;
class ISimpleTexture;
class IVolumeTexture;
class ProgramResource;
class StructBuffer;
class StructElement;
class VertexBuffer;
class VertexElement;

/*! Render system interface.
 * \ingroup Render
 *
 * The render system class is an abstraction of
 * the underlying system used.
 */
class T_DLLCLASS IRenderSystem : public Object
{
	T_RTTI_CLASS;

public:
	/*! \name Render system creation. */
	//@{

	/*! Create render system.
	 *
	 * \param desc Create description.
	 * \return True if successfully created.
	 */
	virtual bool create(const RenderSystemDesc& desc) = 0;

	/*! Destroy render system. */
	virtual void destroy() = 0;

	/*! Reset render system. */
	virtual bool reset(const RenderSystemDesc& desc) = 0;

	/*! Get render system information. */
	virtual void getInformation(RenderSystemInformation& outInfo) const = 0;

	//@}

	/*! \name Display mode enumeration. */
	//@{

	/*! Get number of available display modes.
	 *
	 * Return number of available display modes,
	 * preferably display modes supported by both graphics card
	 * and monitor.
	 */
	virtual uint32_t getDisplayModeCount() const = 0;

	/*! Get display mode.
	 *
	 * Get information about display mode from index 0 - (getDisplayMode() - 1).
	 */
	virtual DisplayMode getDisplayMode(uint32_t index) const = 0;

	/*! Get current display mode.
	 *
	 * Get information about currently set display mode.
	 */
	virtual DisplayMode getCurrentDisplayMode() const = 0;

	/*! Get display aspect ratio. */
	virtual float getDisplayAspectRatio() const = 0;

	//@}

	/*! \name Render view creation. */
	//@{

	/*! Create default render view. */
	virtual Ref< IRenderView > createRenderView(const RenderViewDefaultDesc& desc) = 0;

	/*! Create embedded render view. */
	virtual Ref< IRenderView > createRenderView(const RenderViewEmbeddedDesc& desc) = 0;

	//@}

	/*! \name Factory methods. */
	//@{

	/*! Create vertex buffer.
	 *
	 * \param vertexElements Vertex element declaration.
	 * \param bufferSize Size of vertex buffer in bytes.
	 * \param dynamic If vertex buffer is frequently updated.
	 */
	virtual Ref< VertexBuffer > createVertexBuffer(const AlignedVector< VertexElement >& vertexElements, uint32_t bufferSize, bool dynamic) = 0;

	/*! Create index buffer.
	 *
	 * \param indexType Type of index, 16 or 32 bit.
	 * \param bufferSize Size of index buffer in bytes.
	 * \param dynamic If index buffer is frequently updated.
	 */
	virtual Ref< IndexBuffer > createIndexBuffer(IndexType indexType, uint32_t bufferSize, bool dynamic) = 0;

	/*! Create structure buffer.
	 *
	 * \param vertexElements Structure element declaration.
	 * \param bufferSize Size of vertex buffer in bytes.
	 * \param dynamic If structure buffer is frequently updated.
	 */
	virtual Ref< StructBuffer > createStructBuffer(const AlignedVector< StructElement >& structElements, uint32_t bufferSize, bool dynamic) = 0;

	/*! Create simple, 2d, texture. */
	virtual Ref< ISimpleTexture > createSimpleTexture(const SimpleTextureCreateDesc& desc, const wchar_t* const tag) = 0;

	/*! Create cube texture. */
	virtual Ref< ICubeTexture > createCubeTexture(const CubeTextureCreateDesc& desc, const wchar_t* const tag) = 0;

	/*! Create volume texture. */
	virtual Ref< IVolumeTexture > createVolumeTexture(const VolumeTextureCreateDesc& desc, const wchar_t* const tag) = 0;

	/*! Create render target set.
	 *
	 * \note
	 * If "usingPrimaryDepthStencil" is set in description
	 * then sharedDepthStencil is ignored and primary depth buffer
	 * is used for sharing.
	 *
	 * \param desc Render target set description.
	 * \param sharedDepthStencil Share depth/stencil with this render target set.
	 * \param tag Debug tag.
	 * \return Render target set.
	 */
	virtual Ref< IRenderTargetSet > createRenderTargetSet(const RenderTargetSetCreateDesc& desc, IRenderTargetSet* sharedDepthStencil, const wchar_t* const tag) = 0;

	/*! Create program from program resource.
	 *
	 * \param programResource Compiled program resource.
	 * \return Program suitable for rendering with this render system.
	 */
	virtual Ref< IProgram > createProgram(const ProgramResource* programResource, const wchar_t* const tag) = 0;

	/*! Purge any resource which might be pending destruction.
	 */
	virtual void purge() = 0;

	//@}

	/*! \name Statistics. */
	//@{

	virtual void getStatistics(RenderSystemStatistics& outStatistics) const = 0;

	//@}
};

	}
}

