/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Singleton/ISingleton.h"
#include "Core/Thread/SpinLock.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

/*! Frame fenced destruction of render resources.
 * \ingroup Render
 *
 * Render contexts reference resources through raw pointers; \sa RenderBlock and
 * \sa ProgramParameters both store bare IProgram, IBufferView, ITexture etc.
 * pointers into the context's heap. Since the context built for frame N is
 * consumed by the render thread while the simulation is already producing
 * frame N+1 a resource must remain alive, and functional, until every context
 * which might reference it has been rendered.
 *
 * Resources therefore aren't destroyed when their last reference is released;
 * they are handed to this queue and destroyed a couple of frames later, from
 * the thread which consumes the render contexts.
 *
 * Nothing is paid while building or rendering a frame; the only cost is a
 * single push per destroyed resource plus a pointer swap per frame.
 */
class T_DLLCLASS ResourceMorgue : public ISingleton
{
public:
	static ResourceMorgue& getInstance();

	virtual void destroy() override final;

	/*! Register a render view as driving the fence.
	 *
	 * Retirement is only active while at least one view is registered since a
	 * view is what advances the fence. When none are registered, resources are
	 * destroyed immediately; tools which never render a frame, such as the
	 * pipeline or the lightmap baker, are thus unaffected.
	 */
	void addView();

	/*! Unregister a render view; drains the queue when the last one is removed. */
	void removeView();

	/*! Retire resource which has reached zero references.
	 *
	 * Takes over ownership of the resource; it is deleted once the fence has been
	 * advanced past every context which might reference it. Should the resource
	 * be referenced again in the meantime, which happens when a context still
	 * being rendered binds it, ownership is handed back over and it is retired
	 * anew when released. Callable from any thread.
	 */
	void retire(const Object* resource);

	/*! Advance the fence one frame.
	 *
	 * Called by the render view when it begins a frame, thus from the thread
	 * which consumes the render contexts.
	 */
	void advance();

	/*! Destroy everything pending immediately.
	 *
	 * The caller guarantees no render context which might reference any retired
	 * resource will be rendered, i.e. rendering is idle.
	 */
	void flush();

private:
	/*! Number of frames a resource is kept alive after having been retired.
	*
	* Must be at least the render thread's frame queue depth plus one; a resource
	* can be retired while the context it was written into is still being built, so
	* it has to survive both that context and the one being rendered.
	*
	* A retired resource is pushed into the bucket which is current at the time;
	* that bucket isn't drained until the head has wrapped all the way around, i.e.
	* after three advances. By then every context which existed when the resource
	* was retired has been rendered in its entirety.
	*/
	constexpr static uint32_t c_bucketCount = 3;

	SpinLock m_lock;
	AlignedVector< const Object* > m_buckets[c_bucketCount];
	uint32_t m_head = 0;
	int32_t m_views = 0;
};

}
