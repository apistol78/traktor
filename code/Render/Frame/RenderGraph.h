/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <functional>
#include <string>
#include <vector>
#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Containers/StaticSet.h"
#include "Core/Containers/StaticVector.h"
#include "Render/Frame/RenderGraphTypes.h"
#include "Render/Frame/RenderPass.h"

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
class IRenderSystem;
class ITexture;
class RenderContext;
class RenderGraphBufferPool;
class RenderGraphTargetSetPool;
class RenderGraphTexturePool;

/*! Render graph.
 * \ingroup Render
 * 
 * A render graph describe all passes which is required
 * when rendering a frame. Since all passes is registered
 * beforehand passes can be organized to reduce
 * transitions of targets between passes.
 * 
 * External handlers are registered which
 * are called at appropriate times to render each pass.
 */
class T_DLLCLASS RenderGraph : public Object
{
	T_RTTI_CLASS;

public:
	typedef std::function< void(int32_t, int32_t, const std::wstring&, double, double) > fn_profiler_t;

	struct TargetSize
	{
		int32_t width = 0;
		int32_t height = 0;
	};

	struct TargetResource
	{
		const wchar_t* name = nullptr;
		handle_t persistentHandle = 0;
		bool doubleBuffered = false;
		RenderGraphTargetSetDesc targetSetDesc;
		Ref< IRenderTargetSet > writeTargetSet;
		Ref< IRenderTargetSet > readTargetSet;
		handle_t sharedDepthStencilTargetSetId = ~0U;
		handle_t sizeReferenceTargetSetId = 0;
		TargetSize realized = { 0, 0 };
		int32_t inputRefCount = 0;
		int32_t outputRefCount = 0;
		bool external = false;
	};

	struct BufferResource
	{
		const wchar_t* name = nullptr;
		handle_t persistentHandle = 0;
		Ref< Buffer > buffer;
		uint32_t bufferSize;
	};

	struct TextureResource
	{
		const wchar_t* name = nullptr;
		handle_t persistentHandle = 0;
		RenderGraphTextureDesc textureDesc;
		Ref< ITexture > texture;
	};

	/*! */
	explicit RenderGraph(
		IRenderSystem* renderSystem,
		uint32_t multiSample,
		const fn_profiler_t& profiler = fn_profiler_t()
	);

	/*! */
	virtual ~RenderGraph();

	/*! */
	void destroy();

	/*! Add explicit target set resource.
	 *
	 * \param name Name of target set, used for debugging only.
	 * \param targetSet Render target set.
	 * \return Opaque resource handle.
	 */
	handle_t addExplicitTargetSet(const wchar_t* const name, IRenderTargetSet* targetSet);

	/*! Add transient target set resource.
	 *
	 * \param name Name of target set, used for debugging only.
	 * \param targetSetDesc Render target set create description.
	 * \param sharedDepthStencilTargetSetId Share depth/stencil with target set.
	 * \param sizeReferenceTargetSetId Target to get reference size from when determine target set.
	 * \return Opaque resource handle.
	 */
	handle_t addTransientTargetSet(
		const wchar_t* const name,
		const RenderGraphTargetSetDesc& targetSetDesc,
		handle_t sharedDepthStencilTargetSetId = ~0U,
		handle_t sizeReferenceTargetSetId = 0
	);

	/*! Add persistent target set resource.
	 *
	 * A persistent target set is a target set which is reused
	 * for multiple frames, such as last frame etc.
	 * The persistent handle is used to track target so
	 * same target is reused (if possible).
	 * First time target is requested it's created thus
	 * algorithms should expect target's content to not always be valid.
	 *
	 * \param name Name of target set, used for debugging only.
	 * \param persistentHandle Unique handle to track persistent targets.
	 * \param doubleBuffered Double buffered target.
	 * \param targetSetDesc Render target set create description.
	 * \param sharedDepthStencilTargetSetId Share depth/stencil with target set.
	 * \param sizeReferenceTargetSetId Target to get reference size from when determine target set.
	 * \return Opaque resource handle.
	 */
	handle_t addPersistentTargetSet(
		const wchar_t* const name,
		handle_t persistentHandle,
		bool doubleBuffered,
		const RenderGraphTargetSetDesc& targetSetDesc,
		handle_t sharedDepthStencilTargetSetId = ~0U,
		handle_t sizeReferenceTargetSetId = 0
	);

	/*! Add explicit buffer resource.
	 *
	 * \param name Name of buffer, used for debugging only.
	 * \param buffer Buffer resource.
	 * \return Opaque resource handle.
	 */
	handle_t addExplicitBuffer(const wchar_t* const name, Buffer* buffer);

	/*! Add transient buffer resource.
	 *
	 * \param name Name of buffer, used for debugging only.
	 * \param bufferSize Size of buffer in bytes.
	 * \return Opaque resource handle.
	 */
	handle_t addTransientBuffer(const wchar_t* const name, uint32_t bufferSize);

	/*! Add persistent buffer resource.
	 *
	 * \param name Name of buffer, used for debugging only.
	 * \param persistentHandle Unique handle to track persistent buffer.
	 * \param bufferSize Size of buffer in bytes.
	 * \return Opaque resource handle.
	 */
	handle_t addPersistentBuffer(const wchar_t* const name, handle_t persistentHandle, uint32_t bufferSize);

	/*! Add a transient texture.
	 *
	 * A transient texture is always created with storage texture
	 * capabilities because it's the only reason that make sense.
	 * Thus useful for intermediate textures between compute passes.
	 * 
	 * \param name Name of texture, used for debugging only.
	 * \param textureDesc Description of transient texture.
	 * \return Opaque resource handle.
	 */
	handle_t addTransientTexture(const wchar_t* const name, const RenderGraphTextureDesc& textureDesc);

	/*!
	 */
	handle_t addPersistentTexture(const wchar_t* const name, handle_t persistentHandle, const RenderGraphTextureDesc& textureDesc);

	/*!
	 */
	handle_t addDependency();

	/*! Get target set from resource handle.
	 *
	 * \param resource Opaque resource handle.
	 * \return Render target set.
	 */
	IRenderTargetSet* getTargetSet(handle_t resource) const;

	/*! Get buffer from resource handle.
	 *
	 * \param resource Opaque resource handle.
	 * \return Buffer
	 */
	Buffer* getBuffer(handle_t resource) const;

	/*!
	 */
	ITexture* getTexture(handle_t resource) const;

	/*! Add render pass to graph.
	 *
	 * \param pass Render pass to add.
	 */
	void addPass(const RenderPass* pass);

	/*! Validate render graph.
	 *
	 * Walks through all registered passes to determine
	 * which order they should be rendered and which
	 * targets needs to be acquired etc.
	 *
	 * This must be called before build since order
	 * of passes is calculated when validation.
	 *
	 * \return True if validation succeeded and graph is ready to be rendered.
	 */
	bool validate();

	/*! */
	bool build(RenderContext* renderContext, int32_t width, int32_t height);

	/*! */
	const SmallMap< handle_t, TargetResource >& getTargets() const { return m_targets; }

	/*! */
	const SmallMap< handle_t, BufferResource >& getBuffers() const { return m_buffers; }

	/*! */
	const SmallMap< handle_t, TextureResource >& getTextures() const { return m_textures; }

	/*! */
	const RefArray< const RenderPass >& getPasses() const { return m_passes; }

private:
	Ref< RenderGraphTargetSetPool > m_targetSetPool;
	Ref< RenderGraphBufferPool > m_bufferPool;
	Ref< RenderGraphTexturePool > m_texturePool;
	SmallMap< handle_t, TargetResource > m_targets;
	SmallMap< handle_t, BufferResource > m_buffers;
	SmallMap< handle_t, TextureResource > m_textures;
	RefArray< const RenderPass > m_passes;
	StaticVector< uint32_t, 32 > m_order[32];
	StaticSet< handle_t, 32 > m_sharedDepthTargets;
	uint32_t m_counter;
	uint32_t m_multiSample;
	handle_t m_nextResourceId;
	fn_profiler_t m_profiler;

	bool realizeTargetDimensions(int32_t width, int32_t height, int32_t targetId);

	bool acquire(TargetResource& inoutTarget);

	void cleanup();
};

}
