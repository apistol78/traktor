/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Misc/ComRef.h"
#include "Render/IRenderSystem.h"
#include "Render/Dx11/Platform.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_DX11_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class ContextDx11;
class ResourceCache;
class Window;

/*! DirectX 10 render system.
 * \ingroup DX11
 *
 * DX11 render system implementation.
 */
class T_DLLCLASS RenderSystemDx11 : public IRenderSystem
{
	T_RTTI_CLASS;

public:
	RenderSystemDx11();

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

	virtual Ref< ITexture > createSimpleTexture(const SimpleTextureCreateDesc& desc, const wchar_t* const tag) override final;

	virtual Ref< ITexture > createCubeTexture(const CubeTextureCreateDesc& desc, const wchar_t* const tag) override final;

	virtual Ref< ITexture > createVolumeTexture(const VolumeTextureCreateDesc& desc, const wchar_t* const tag) override final;

	virtual Ref< IRenderTargetSet > createRenderTargetSet(const RenderTargetSetCreateDesc& desc, IRenderTargetSet* sharedDepthStencil, const wchar_t* const tag) override final;

	virtual Ref< IProgram > createProgram(const ProgramResource* programResource, const wchar_t* const tag) override final;

	virtual void purge() override final;

	virtual void getStatistics(RenderSystemStatistics& outStatistics) const override final;

private:
	Ref< ContextDx11 > m_context;
	Ref< Window > m_window;
	Ref< ResourceCache > m_resourceCache;
	bool m_resourceCreateLock;
	float m_displayAspect;
};

	}
}

