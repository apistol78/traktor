/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/SmallSet.h"
#include "Render/IRenderView.h"

namespace traktor
{

class Thread;

}

namespace traktor::drawing
{

class Image;

}

namespace traktor::render
{

class IRenderSystem;
class IRenderTargetSet;

/*! Verification render view.
 * \ingroup Vrfy
 */
class RenderViewVrfy : public IRenderView
{
	T_RTTI_CLASS;

public:
	explicit RenderViewVrfy(const RenderViewDesc& desc, IRenderSystem* renderSystem, IRenderView* renderView);

	virtual bool nextEvent(RenderEvent& outEvent) override final;

	virtual void close() override final;

	virtual bool reset(const RenderViewDefaultDesc& desc) override final;

	virtual bool reset(int32_t width, int32_t height) override final;

	virtual uint32_t getDisplay() const override final;

	virtual int getWidth() const override final;

	virtual int getHeight() const override final;

	virtual bool isActive() const override final;

	virtual bool isMinimized() const override final;

	virtual bool isFullScreen() const override final;

	virtual void showCursor() override final;

	virtual void hideCursor() override final;

	virtual bool isCursorVisible() const override final;

	virtual bool setGamma(float gamma) override final;

	virtual void setViewport(const Viewport& viewport) override final;

	virtual SystemWindow getSystemWindow() override final;

	virtual bool beginFrame() override final;

	virtual void endFrame() override final;

	virtual void present() override final;

	virtual bool beginPass(const Clear* clear, uint32_t load, uint32_t store) override final;

	virtual bool beginPass(IRenderTargetSet* renderTargetSet, const Clear* clear, uint32_t load, uint32_t store) override final;

	virtual bool beginPass(IRenderTargetSet* renderTargetSet, int32_t renderTarget, const Clear* clear, uint32_t load, uint32_t store) override final;

	virtual void endPass() override final;

	virtual void draw(const IBufferView* vertexBuffer, const IVertexLayout* vertexLayout, const IBufferView* indexBuffer, IndexType indexType, IProgram* program, const Primitives& primitives, uint32_t instanceCount) override final;

	virtual void drawIndirect(const IBufferView* vertexBuffer, const IVertexLayout* vertexLayout, const IBufferView* indexBuffer, IndexType indexType, IProgram* program, PrimitiveType primitiveType, const IBufferView* drawBuffer, uint32_t drawCount) override final;

	virtual void compute(IProgram* program, const int32_t* workSize) override final;

	virtual void barrier(Stage from, Stage to, ITexture* written, uint32_t writtenMip) override final;

	virtual bool copy(ITexture* destinationTexture, const Region& destinationRegion, ITexture* sourceTexture, const Region& sourceRegion) override final;

	virtual int32_t beginTimeQuery() override final;

	virtual void endTimeQuery(int32_t query) override final;

	virtual bool getTimeQuery(int32_t query, bool wait, double& outStart, double& outEnd) const override final;

	virtual void pushMarker(const std::wstring& marker) override final;

	virtual void popMarker() override final;

	virtual void getStatistics(RenderViewStatistics& outStatistics) const override final;

private:
	struct ProfileVrfy
	{
		const wchar_t* name;
		intptr_t begin;
		intptr_t end;
	};

	RenderViewDesc m_desc;
	Ref< IRenderSystem > m_renderSystem;
	Ref< IRenderView > m_renderView;
	bool m_insideFrame = false;
	bool m_insidePass = false;
	Thread* m_threadFrame = nullptr;
	AlignedVector< ProfileVrfy > m_timeStamps;
	mutable SmallSet< int32_t > m_queriesPending;
};

}
