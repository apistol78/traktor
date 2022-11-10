/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
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

namespace traktor::render
{

class Buffer;
class ICubeTexture;
class IProgram;
class IRenderTargetSet;
class IRenderView;
class ISimpleTexture;
class IVertexLayout;
class IVolumeTexture;
class ProgramResource;
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

	/*! Create buffer.
	 *
	 * \param usage Buffer usage flags.
	 * \param elementCount Number of elements.
	 * \param elementSize Size of a single element.
	 * \param dynamic If index buffer is frequently updated.
	 */
	virtual Ref< Buffer > createBuffer(uint32_t usage, uint32_t elementCount, uint32_t elementSize, bool dynamic) = 0;

	/*! Create vertex layout. */
	virtual Ref< const IVertexLayout > createVertexLayout(const AlignedVector< VertexElement >& vertexElements) = 0;

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
