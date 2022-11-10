/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Render/IRenderSystem.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_VRFY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

struct RENDERDOC_API_1_4_1;

namespace traktor
{

class Library;

	namespace render
	{

class ResourceTracker;

/*! Verification render system.
 * \ingroup Render
 *
 * This render system is only a wrapper around
 * a "real" render system implementation.
 * Main purpose of this render system is to
 * verify usage, help track bad usage
 * patterns and maintain consistent use
 * from all systems.
 */
class T_DLLCLASS RenderSystemVrfy : public IRenderSystem
{
	T_RTTI_CLASS;

public:
	RenderSystemVrfy() = default;

	explicit RenderSystemVrfy(bool useRenderDoc);

	virtual bool create(const RenderSystemDesc& desc) override final;

	virtual void destroy() override final;

	virtual bool reset(const RenderSystemDesc& desc) override final;

	virtual void getInformation(RenderSystemInformation& outInfo) const override final;

	virtual uint32_t getDisplayModeCount() const override final;

	virtual DisplayMode getDisplayMode(uint32_t index) const override final;

	virtual DisplayMode getCurrentDisplayMode() const override final;

	virtual float getDisplayAspectRatio() const override final;

	virtual Ref< IRenderView > createRenderView(const RenderViewDefaultDesc& desc) override final;

	virtual Ref< IRenderView > createRenderView(const RenderViewEmbeddedDesc& desc) override final;

	virtual Ref< Buffer > createBuffer(uint32_t usage, uint32_t elementCount, uint32_t elementSize, bool dynamic) override final;

	virtual Ref< const IVertexLayout > createVertexLayout(const AlignedVector< VertexElement >& vertexElements) override final;

	virtual Ref< ISimpleTexture > createSimpleTexture(const SimpleTextureCreateDesc& desc, const wchar_t* const tag) override final;

	virtual Ref< ICubeTexture > createCubeTexture(const CubeTextureCreateDesc& desc, const wchar_t* const tag) override final;

	virtual Ref< IVolumeTexture > createVolumeTexture(const VolumeTextureCreateDesc& desc, const wchar_t* const tag) override final;

	virtual Ref< IRenderTargetSet > createRenderTargetSet(const RenderTargetSetCreateDesc& desc, IRenderTargetSet* sharedDepthStencil, const wchar_t* const tag) override final;

	virtual Ref< IProgram > createProgram(const ProgramResource* programResource, const wchar_t* const tag) override final;

	virtual void purge() override final;

	virtual void getStatistics(RenderSystemStatistics& outStatistics) const override final;

private:
	Ref< IRenderSystem > m_renderSystem;
	bool m_useRenderDoc = false;
	Ref< Library > m_libRenderDoc;
	RENDERDOC_API_1_4_1* m_apiRenderDoc = nullptr;
	Ref< ResourceTracker > m_resourceTracker;
};

	}
}

