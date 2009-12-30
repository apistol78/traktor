#include <algorithm>
#include "Core/Log/Log.h"
#include "Core/Thread/Acquire.h"
#include "Render/Ps3/PlatformPs3.h"
#include "Render/Ps3/LocalMemoryManager.h"
#include "Render/Ps3/LocalMemoryObject.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

const uint32_t c_blockSize = 128;				//< Allocation size must be a multiple of c_blockSize.
const uint32_t c_transferWaitLabelId = 151;

template < typename T >
T alignDown(T v, size_t a)
{
	return (T)((size_t)v & ~(a - 1));
}

template < typename T >
T alignUp(T v, size_t a)
{
	return (T)(((size_t)v + (a - 1)) & ~(a - 1));
}

		}

LocalMemoryManager& LocalMemoryManager::getInstance()
{
	static LocalMemoryManager instance;
	return instance;
}

void LocalMemoryManager::setHeap(void* heap, size_t heapSize)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	T_ASSERT (m_objects.empty());

	m_heap = (uint8_t*)heap;
	m_heapSize = heapSize;
}

LocalMemoryObject* LocalMemoryManager::alloc(size_t size, size_t align, bool immutable)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	T_ASSERT (size > 0);
	T_ASSERT (align_ > 0);

	LocalMemoryObject* object = 0;

	size = alignUp(size, c_blockSize);

	compact();

	if (!immutable)
	{
		uint8_t* ptr = m_heap;

		if (!m_objects.empty())
		{
			LocalMemoryObject* last = m_objects.back();
			ptr = (uint8_t*)last->m_pointer + last->m_size;
		}

		uint8_t* alignedPtrStart = alignUp(ptr, align);
		uint8_t* alignedPtrEnd = alignedPtrStart + size;

		T_FATAL_ASSERT_M (alignedPtrEnd <= m_heap + m_heapSize, L"Out of local memory");

		object = new LocalMemoryObject();
		object->m_immutable = false;
		object->m_alignment = align;
		object->m_size = size;
		object->m_pointer = alignedPtrStart;
		object->m_offset = 0;

		int err = cellGcmAddressToOffset(object->m_pointer, &object->m_offset);
		T_FATAL_ASSERT (err == CELL_OK);

		m_objects.push_back(object);
	}
	else
	{
		int32_t count = int32_t(m_objects.size());
		if (count > 0)
		{
			// Find last immutable object.
			int32_t last = -1;
			for (int32_t i = 0; i < count - 1; ++i)
			{
				if (m_objects[i]->m_immutable && !m_objects[i + 1]->m_immutable)
				{
					last = i;
					break;
				}
			}

			// Immutable object chunk.
			uint8_t* immPtrStart = alignUp(last >= 0 ? (uint8_t*)m_objects[last]->m_pointer + m_objects[last]->m_size : m_heap, align);
			uint8_t* immPtrEnd = immPtrStart + size;

			T_FATAL_ASSERT_M (immPtrEnd <= m_heap + m_heapSize, L"Out of local memory");

			// Find maximum alignment of all other mutable objects.
			size_t alignMax = 0;
			for (int32_t i = last + 1; i < count; ++i)
				alignMax = std::max(alignMax, m_objects[i]->m_alignment);

			size_t offset = alignUp((size_t)(immPtrEnd - immPtrStart), alignMax);

			// Move mutable objects to make room for this object.
			for (int32_t i = count - 1; i > last; --i)
			{
				LocalMemoryObject* object1 = m_objects[i];

				uint8_t* ptr1 = (uint8_t*)object1->m_pointer;
				uint8_t* ptr1New = ptr1 + offset;

				// Ensure object are still satisfy it's alignment requirement.
				T_ASSERT (alignUp(ptr1New, object1->m_alignment) == ptr1New);

				std::memmove(
					ptr1New,
					ptr1,
					object1->m_size
				);

				object1->m_pointer = ptr1New;
				object1->m_offset = 0;

				int err = cellGcmAddressToOffset(object1->m_pointer, &object1->m_offset);
				T_FATAL_ASSERT (err == CELL_OK);
			}

			object = new LocalMemoryObject();
			object->m_immutable = true;
			object->m_alignment = align;
			object->m_size = size;
			object->m_pointer = immPtrStart;
			object->m_offset = 0;

			int err = cellGcmAddressToOffset(object->m_pointer, &object->m_offset);
			T_FATAL_ASSERT (err == CELL_OK);

			m_objects.insert(m_objects.begin() + (last + 1), object);
		}
		else
		{
			object = new LocalMemoryObject();
			object->m_immutable = true;
			object->m_alignment = align;
			object->m_size = size;
			object->m_pointer = alignUp(m_heap, align);
			object->m_offset = 0;

			int err = cellGcmAddressToOffset(object->m_pointer, &object->m_offset);
			T_FATAL_ASSERT (err == CELL_OK);

			m_objects.push_back(object);
		}
	}

	return object;
}

void LocalMemoryManager::free(LocalMemoryObject* object)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	std::vector< LocalMemoryObject* >::iterator i = std::find(m_objects.begin(), m_objects.end(), object);
	T_ASSERT (i != m_objects.end());

	m_objects.erase(i);
	delete object;

	m_shouldCompact = true;
}

size_t LocalMemoryManager::available() const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	uint32_t count = uint32_t(m_objects.size());
	if (count)
	{
		LocalMemoryObject* last = m_objects[count - 1];
		size_t addr = (size_t)(last->m_pointer) + last->m_size;
		return m_heapSize - (addr - (size_t)m_heap);
	}
	else
		return m_heapSize;
}

void LocalMemoryManager::compact()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	if (!m_shouldCompact)
		return;

	m_shouldCompact = false;

	// Get number of alive objects; if none then there is no to move.
	uint32_t count = uint32_t(m_objects.size());
	if (!count)
		return;

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
		LocalMemoryObject* object1 = m_objects[i];
		LocalMemoryObject* object2 = m_objects[i + 1];
		T_ASSERT (!object2->m_immutable);

		uint8_t* ptr1 = (uint8_t*)object1->m_pointer;
		uint8_t* ptr2 = (uint8_t*)object2->m_pointer;

		uint8_t* ptr1Tail = ptr1 + object1->m_size;

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
				CELL_GCM_TRANSFER_LOCAL_TO_LOCAL,
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
				if (err == CELL_OK)
				{
					object2->m_pointer = ptr2New;
					object2->m_offset = offsetNew;
				}
			}
		}
		if (err != CELL_OK)
			log::error << L"Stale memory; failed to move memory" << Endl;
	}

	// Ensure all transfers have been executed.
	cellGcmSetWriteBackEndLabel(gCellGcmCurrentContext, c_transferWaitLabelId, m_waitLabel);
	cellGcmFlush(gCellGcmCurrentContext);
	cellGcmSetWaitLabel(gCellGcmCurrentContext, c_transferWaitLabelId, m_waitLabel);
	++m_waitLabel;
}

LocalMemoryManager::LocalMemoryManager()
:	m_heap(0)
,	m_heapSize(0)
,	m_shouldCompact(false)
,	m_waitLabel(0)
{
}

	}
}
