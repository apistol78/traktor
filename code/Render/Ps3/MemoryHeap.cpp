/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <algorithm>
#include "Core/Log/Log.h"
#include "Core/Misc/Align.h"
#include "Core/Thread/Acquire.h"
#include "Render/Ps3/PlatformPs3.h"
#include "Render/Ps3/MemoryHeap.h"
#include "Render/Ps3/MemoryHeapObject.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

const uint32_t c_blockSize = 128;					//< Allocation size must be a multiple of c_blockSize.

uint32_t incrementLabel(uint32_t label)
{
	return (++label != 0) ? label : 1;
}
		}

MemoryHeap::MemoryHeap(void* heap, size_t heapSize, size_t immutableOffset, uint8_t location)
:	m_heap(static_cast< uint8_t* >(heap))
,	m_heapSize(heapSize)
,	m_immutableOffset(immutableOffset)
,	m_location(location)
,	m_shouldCompact(0)
,	m_waitLabel(1)
{
}

MemoryHeapObject* MemoryHeap::alloc(size_t size, size_t align, bool immutable)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	T_ASSERT (size > 0);
	T_ASSERT (align > 0);

	MemoryHeapObject* object = 0;

	size = alignUp(size, c_blockSize);

	if (!immutable)
	{
		uint8_t* ptr = m_heap + m_immutableOffset;

		if (!m_objects.empty())
		{
			MemoryHeapObject* last = m_objects.back();
			uint8_t* lastTailPtr = (uint8_t*)last->m_pointer + last->m_size;
			if (lastTailPtr > ptr)
				ptr = lastTailPtr;
		}

		uint8_t* alignedPtrStart = alignUp(ptr, align);
		uint8_t* alignedPtrEnd = alignedPtrStart + size;

		if (alignedPtrEnd > m_heap + m_heapSize)
		{
			log::error << L"Out of mutable RSX heap memory; heap size " << m_heapSize << Endl;
			T_FATAL_ERROR;
			return 0;
		}

		object = new MemoryHeapObject();
		object->m_heap = this;
		object->m_immutable = false;
		object->m_alignment = align;
		object->m_size = size;
		object->m_pointer = alignedPtrStart;
		object->m_offset = 0;
		object->m_location = m_location;

		int err = cellGcmAddressToOffset(object->m_pointer, &object->m_offset);
		if (err != CELL_OK)
		{
			log::error << L"cellGcmAddressToOffset failed, err " << err << Endl;
			T_FATAL_ERROR;
			return 0;
		}

		m_objects.push_back(object);
	}
	else
	{
		int32_t count = int32_t(m_objects.size());
		if (count > 0)
		{
			// Find last immutable object.
			int32_t last = -1;
			for (int32_t i = count - 1; i >= 0; --i)
			{
				if (m_objects[i]->m_immutable)
				{
					last = i;
					break;
				}
			}

			// Immutable object chunk.
			uint8_t* immPtrStart = alignUp(last >= 0 ? (uint8_t*)m_objects[last]->m_pointer + m_objects[last]->m_size : m_heap, align);
			uint8_t* immPtrEnd = immPtrStart + size;

			if (immPtrEnd > m_heap + m_heapSize)
			{
				log::error << L"Out of immutable RSX heap memory; heap size " << m_heapSize << Endl;
				T_FATAL_ERROR;
				return 0;
			}

			// Move mutable objects occupying desired space to the back
			// of all mutables.
			MemoryHeapObject* mutLast = m_objects.back();
			if (!mutLast->m_immutable)
			{
				uint8_t* mutPtrEnd = (uint8_t*)mutLast->m_pointer + mutLast->m_size;
				for (;;)
				{
					MemoryHeapObject* mutObject = m_objects[last + 1];
					T_ASSERT (!mutObject->m_immutable);

					uint8_t* ptr1 = (uint8_t*)mutObject->m_pointer;
					if (ptr1 > immPtrEnd)
						break;

					uint8_t* ptr2 = alignUp(mutPtrEnd, mutObject->m_alignment);
					if (ptr2 > m_heap + m_heapSize)
					{
						T_FATAL_ERROR;
						return 0;
					}

					log::info << L"Moving mutable block from " << uint32_t(ptr1 - m_heap) << L" to " << uint32_t(ptr2 - m_heap) << (m_location == CELL_GCM_LOCATION_LOCAL ? L" (local)" : L" (main)") << Endl;

					__builtin_memcpy(
						ptr2,
						ptr1,
						mutObject->m_size
					);

					mutObject->m_pointer = ptr2;
					int err = cellGcmAddressToOffset(mutObject->m_pointer, &mutObject->m_offset);
					if (err != CELL_OK)
					{
						log::error << L"cellGcmAddressToOffset failed, err " << err << Endl;
						T_FATAL_ERROR;
						return 0;
					}

					mutPtrEnd = ptr2 + mutObject->m_size;

					// Place object last in sorted array.
					m_objects.erase(m_objects.begin() + (last + 1));
					m_objects.push_back(mutObject);
				}
			}

			object = new MemoryHeapObject();
			object->m_heap = this;
			object->m_immutable = true;
			object->m_alignment = align;
			object->m_size = size;
			object->m_pointer = immPtrStart;
			object->m_offset = 0;
			object->m_location = m_location;

			int err = cellGcmAddressToOffset(object->m_pointer, &object->m_offset);
			if (err != CELL_OK)
			{
				log::error << L"cellGcmAddressToOffset failed, err " << err << Endl;
				T_FATAL_ERROR;
				return 0;
			}

			m_objects.insert(m_objects.begin() + (last + 1), object);
		}
		else
		{
			object = new MemoryHeapObject();
			object->m_heap = this;
			object->m_immutable = true;
			object->m_alignment = align;
			object->m_size = size;
			object->m_pointer = alignUp(m_heap, align);
			object->m_offset = 0;
			object->m_location = m_location;

			int err = cellGcmAddressToOffset(object->m_pointer, &object->m_offset);
			if (err != CELL_OK)
			{
				log::error << L"cellGcmAddressToOffset failed, err " << err << Endl;
				T_FATAL_ERROR;
				return 0;
			}

			m_objects.push_back(object);
		}
	}

	return object;
}

void MemoryHeap::free(MemoryHeapObject* object)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	T_ASSERT (object->m_heap == this);

	// Ensure GPU is idle, i.e. not using any of our resources.
	cellGcmFinish(gCellGcmCurrentContext, m_waitLabel);
	m_waitLabel = incrementLabel(m_waitLabel);

	std::vector< MemoryHeapObject* >::iterator i = std::find(m_objects.begin(), m_objects.end(), object);
	T_ASSERT (i != m_objects.end());

	m_objects.erase(i);
	delete object;

	m_shouldCompact = true;
}

size_t MemoryHeap::getAvailable() const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	uint32_t count = uint32_t(m_objects.size());
	if (count)
	{
		MemoryHeapObject* last = m_objects[count - 1];
		size_t addr = (size_t)(last->m_pointer) + last->m_size;
		return m_heapSize - (addr - (size_t)m_heap);
	}
	else
		return m_heapSize;
}

size_t MemoryHeap::getObjectCount() const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	return m_objects.size();
}

void MemoryHeap::compact()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	if (!m_shouldCompact)
		return;

	m_shouldCompact = false;

	// Get number of alive objects; if none then there is no to move.
	uint32_t count = uint32_t(m_objects.size());
	if (!count)
		return;

	// Ensure GPU is idle, i.e. not using any of our resources.
	cellGcmFinish(gCellGcmCurrentContext, m_waitLabel);
	m_waitLabel = incrementLabel(m_waitLabel);

	uint32_t i = 0;

	// Skip immutable objects; mustn't be moved.
	for (; i < count - 1; ++i)
	{
		if (!m_objects[i + 1]->m_immutable)
			break;
	}

	// Close gaps between objects by moving them.
	for (; i < count - 1; ++i)
	{
		MemoryHeapObject* object1 = m_objects[i];
		MemoryHeapObject* object2 = m_objects[i + 1];
		T_ASSERT (!object2->m_immutable);

		uint8_t* ptr1 = (uint8_t*)object1->m_pointer;
		uint8_t* ptr2 = (uint8_t*)object2->m_pointer;

		uint8_t* ptr1Tail = ptr1 + object1->m_size;

		// Don't compact into immutable space.
		if (ptr1Tail < m_heap + m_immutableOffset)
		{
			ptr1Tail = m_heap + m_immutableOffset;
			T_ASSERT (ptr1Tail <= ptr2);
		}

		uint32_t gap = ptr2 - ptr1Tail;
		uint32_t alignmentGaps = gap / object2->m_alignment;
		if (!alignmentGaps)
			continue;

		uint8_t* ptr2New = ptr2 - alignmentGaps * object2->m_alignment;
		T_ASSERT (ptr2New >= ptr1Tail);
		T_ASSERT (alignUp(ptr2New, object2->m_alignment) == ptr2New);

		uint32_t offsetNew = 0;
		int err = cellGcmAddressToOffset(ptr2New, &offsetNew);
		if (err == CELL_OK)
		{
			err = cellGcmSetTransferImage(
				gCellGcmCurrentContext,
				(m_location == CELL_GCM_LOCATION_LOCAL) ? CELL_GCM_TRANSFER_LOCAL_TO_LOCAL : CELL_GCM_TRANSFER_MAIN_TO_MAIN,
				offsetNew,
				128,
				0,
				0,
				object2->m_offset,
				128,
				0,
				0,
				c_blockSize / 2,
				object2->m_size / c_blockSize,
				2
			);

			if (err == CELL_OK)
			{
				object2->m_pointer = ptr2New;
				object2->m_offset = offsetNew;
			}
			else
				log::error << L"Stale memory; cellGcmSetTransferImage failed" << Endl;
		}
		else
			log::error << L"Stale memory; cellGcmAddressToOffset failed" << Endl;
	}

	// Ensure all transfers have been executed.
	cellGcmFinish(gCellGcmCurrentContext, m_waitLabel);
	m_waitLabel = incrementLabel(m_waitLabel);
}

	}
}
