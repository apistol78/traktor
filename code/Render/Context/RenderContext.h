/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Matrix44.h"
#include "Core/Math/Vector4.h"
#include "Core/Misc/AutoPtr.h"
#include "Render/Context/ProgramParameters.h"
#include "Render/Context/RenderBlock.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class IRenderView;

/*! Deferred render context.
 * \ingroup Render
 *
 * A render context is used to defer rendering in a
 * multi-threaded renderer.
 */
class T_DLLCLASS RenderContext : public Object
{
	T_RTTI_CLASS;

public:
	explicit RenderContext(uint32_t heapSize);

	virtual ~RenderContext();

	/*! Allocate a unaligned block of memory from context's heap. */
	[[nodiscard]] void* alloc(uint32_t blockSize);

	/*! Allocate a aligned block of memory from context's heap. */
	[[nodiscard]] void* alloc(uint32_t blockSize, uint32_t align);

	/*! Allocate object from context's heap. */
	template < typename ObjectType, typename ... ArgumentTypes >
	[[nodiscard]] ObjectType* alloc(ArgumentTypes&& ... args)
	{
		void* object = alloc((uint32_t)sizeof(ObjectType), (uint32_t)alignOf< ObjectType >());
		return new (object) ObjectType(std::forward< ArgumentTypes >(args) ...);
	}

	/*! Allocate named object from context's heap. */
	template < typename ObjectType, typename ... ArgumentTypes >
	[[nodiscard]] ObjectType* allocNamed(const std::wstring_view& name, ArgumentTypes&& ... args)
	{
		ObjectType* object = alloc< ObjectType, ArgumentTypes... >(std::forward< ArgumentTypes >(args) ...);
		object->name = name;
		return object;
	}

	/*! Add block to compute queue. */
	void compute(RenderBlock* renderBlock);

	/*! Add block to compute queue. */
	template < typename ObjectType, typename ... ArgumentTypes >
	void compute(ArgumentTypes&& ... args)
	{
		ObjectType* object = alloc< ObjectType, ArgumentTypes... >(std::forward< ArgumentTypes >(args) ...);
		compute(object);
	}

	/*! Add a render block to draw queue. */
	void draw(RenderBlock* renderBlock);

	/*! Add a render block to draw queue. */
	template < typename ObjectType, typename ... ArgumentTypes >
	void draw(ArgumentTypes&& ... args)
	{
		ObjectType* object = alloc< ObjectType, ArgumentTypes... >(std::forward< ArgumentTypes >(args) ...);
		draw(object);
	}

	/*! Add render block to sorting queue. */
	void draw(uint32_t type, DrawableRenderBlock* renderBlock);

	/*! Merge sorting queues into draw queue. */
	void mergePriorityIntoDraw(uint32_t priorities);

	/*! Merge compute queues into render queue. */
	void mergeComputeIntoRender();

	/*! Merge draw queues into render queue. */
	void mergeDrawIntoRender();

	/*! Render blocks queued in render queue. */
	void render(IRenderView* renderView) const;

	/*! Flush blocks. */
	void flush();

	/*! Check if any computes are pending for merge. */
	bool havePendingComputes() const;

	/*! Check if any draws is pending for merge. */
	bool havePendingDraws() const;

	/*! Return how much of the heap has been allocated. */
	uint32_t getAllocatedSize() const { return uint32_t(m_heapPtr - m_heap.c_ptr()); }

private:
	AutoPtr< uint8_t, AllocFreeAlign > m_heap;
	uint8_t* m_heapEnd;
	uint8_t* m_heapPtr;
	AlignedVector< RenderBlock* > m_computeQueue;
	AlignedVector< DrawableRenderBlock* > m_priorityQueue[6];
	AlignedVector< RenderBlock* > m_drawQueue;
	AlignedVector< RenderBlock* > m_renderQueue;
};

}
