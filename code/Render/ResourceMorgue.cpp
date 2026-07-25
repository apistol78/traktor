/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/ResourceMorgue.h"

#include "Core/Singleton/SingletonManager.h"
#include "Core/Thread/Acquire.h"

namespace traktor::render
{
namespace
{

/*! Release the queue's reference to a retired resource.
 *
 * A resource can be referenced again while it is queued; a render context which
 * has not been consumed yet still holds a bare pointer to it and ProgramVk takes
 * a reference to every texture it binds. In that case its new owner keeps it
 * alive and will retire it once more when done, otherwise the queue holds the
 * only reference left and the resource is deleted here.
 */
void releaseRetired(const Object* resource)
{
	if (resource->getReferenceCount() > 1)
		resource->release(nullptr);
	else
		delete resource;
}

}

ResourceMorgue& ResourceMorgue::getInstance()
{
	static ResourceMorgue* s_instance = nullptr;
	if (!s_instance)
	{
		s_instance = new ResourceMorgue();
		SingletonManager::getInstance().add(s_instance);
	}
	return *s_instance;
}

void ResourceMorgue::destroy()
{
	T_FATAL_ASSERT(m_views == 0);
}

void ResourceMorgue::addView()
{
	T_ANONYMOUS_VAR(Acquire< SpinLock >)(m_lock);
	++m_views;
}

void ResourceMorgue::removeView()
{
	{
		T_ANONYMOUS_VAR(Acquire< SpinLock >)(m_lock);
		if (--m_views > 0)
			return;
	}

	// Last view removed; the fence will not be advanced any more so drain
	// everything which is still pending.
	flush();
}

void ResourceMorgue::retire(const Object* resource)
{
	T_ASSERT(resource != nullptr);

	// Take over ownership. Holding a reference is what makes it safe for the
	// resource to be referenced again while queued; its count never returns to
	// zero so it can never end up in the queue twice.
	resource->addRef(nullptr);

	{
		T_ANONYMOUS_VAR(Acquire< SpinLock >)(m_lock);
		if (m_views > 0)
		{
			m_buckets[m_head].push_back(resource);
			return;
		}
	}

	// No view is driving the fence thus no render context can be in flight.
	releaseRetired(resource);
}

void ResourceMorgue::advance()
{
	AlignedVector< const Object* > retired;
	{
		T_ANONYMOUS_VAR(Acquire< SpinLock >)(m_lock);
		m_head = (m_head + 1) % c_bucketCount;
		retired.swap(m_buckets[m_head]);
	}

	// Release outside of the lock; destructors commonly release further resources
	// which are then retired in turn.
	for (auto resource : retired)
		releaseRetired(resource);
}

void ResourceMorgue::flush()
{
	for (;;)
	{
		AlignedVector< const Object* > retired;
		{
			T_ANONYMOUS_VAR(Acquire< SpinLock >)(m_lock);
			for (uint32_t i = 0; i < c_bucketCount; ++i)
			{
				if (!m_buckets[i].empty())
				{
					retired.insert(retired.end(), m_buckets[i].begin(), m_buckets[i].end());
					m_buckets[i].resize(0);
				}
			}
		}
		if (retired.empty())
			break;

		// Might retire further resources, thus keep draining until settled.
		for (auto resource : retired)
			releaseRetired(resource);
	}
}

}
