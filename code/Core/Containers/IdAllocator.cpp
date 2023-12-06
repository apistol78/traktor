/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <limits>
#include "Core/Containers/IdAllocator.h"

namespace traktor
{

IdAllocator::IdAllocator()
{
	m_free.insert(Interval(0, std::numeric_limits< uint32_t >::max()));
}

IdAllocator::IdAllocator(uint32_t minId, uint32_t maxId)
{
	m_free.insert(Interval(minId, maxId));
}

uint32_t IdAllocator::alloc()
{
    Interval first = *m_free.begin();
	const uint32_t freeId = first.left;

    m_free.erase(m_free.begin());

    if (first.left + 1 <= first.right)
        m_free.insert(Interval(first.left + 1 , first.right));

    return freeId;
}

uint32_t IdAllocator::allocSequential(uint32_t span)
{
	for (auto it = m_free.begin(); it != m_free.end(); ++it)
	{
		if (span <= (it->right - it->left))
		{
			Interval first = *m_free.begin();
			const uint32_t freeId = first.left;

			m_free.erase(it);

			if (first.left + span <= first.right)
				m_free.insert(Interval(first.left + span, first.right));

			return freeId;
		}
	}
	return ~0U;
}

bool IdAllocator::alloc(uint32_t id)
{
	auto it = m_free.find(Interval(id, id));
	if (it == m_free.end())
		return false;

	Interval freeInterval = *it;
	m_free.erase(it);

	if (freeInterval.left < id)
		m_free.insert(Interval(freeInterval.left, id-1));

	if (id + 1 <= freeInterval.right)
		m_free.insert(Interval(id + 1, freeInterval.right));

	return true;
}

void IdAllocator::free(uint32_t id)
{
	auto it = m_free.find(Interval(id, id));
	if (it != m_free.end() && it->left <= id && it->right > id)
		return;

	it = m_free.upper_bound(Interval(id, id));
	if (it == m_free.end())
		return;

	Interval freeInterval = *(it);

	if (id + 1 != freeInterval.left) {
		m_free.insert(Interval(id, id));
	}
	else
	{
		if (it != m_free.begin())
		{
			auto it2 = it; --it2;
			if (it2->right + 1 == id)
			{
				Interval freeInterval2 = *it2;
				m_free.erase(it);
				m_free.erase(it2);
				m_free.insert(Interval(freeInterval2.left, freeInterval.right));
			}
			else
			{
				m_free.erase(it);
				m_free.insert(Interval(id, freeInterval.right));
			}
		}
		else
		{
			m_free.erase(it);
			m_free.insert(Interval(id, freeInterval.right));
		}
	}
}

void IdAllocator::freeSequential(uint32_t id, uint32_t span)
{
	for (; span-- > 0; ++id)
		free(id);
}

IdAllocator::Interval::Interval(uint32_t left_, uint32_t right_)
:	left(left_)
,	right(right_)
{
}

bool IdAllocator::Interval::operator < (const Interval& rh) const
{
	return (left < rh.left) && (right < rh.left);
}

}
