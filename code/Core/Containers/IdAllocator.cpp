/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include <limits>
#include "Core/Containers/IdAllocator.h"

namespace traktor
{

IdAllocator::IdAllocator()
{
	m_free.push_back(Interval(0, std::numeric_limits< uint32_t >::max()));
}

IdAllocator::IdAllocator(uint32_t minId, uint32_t maxId)
{
	m_free.push_back(Interval(minId, maxId));
}

uint32_t IdAllocator::alloc()
{
	Interval& f = m_free.front();
	const uint32_t freeId = f.left;
	if (++f.left > f.right)
		m_free.erase(m_free.begin());
	return freeId;
}

uint32_t IdAllocator::allocSequential(uint32_t span)
{
	T_ASSERT(span > 0);
	for (auto it = m_free.begin(); it != m_free.end(); ++it)
	{
		if ((span - 1) <= (it->right - it->left))
		{
			const uint32_t freeId = it->left;
			
			it->left += span;
			if (it->left > it->right)
				m_free.erase(it);

			return freeId;
		}
	}
	return ~0U;
}

void IdAllocator::free(uint32_t id)
{
	Interval iv(id, id);

	// Simple case, if full then just add interval back.
	if (m_free.empty())
	{
		m_free.push_back(iv);
		return;
	}

	// Insert interval in incremental position.
	if (id < m_free.front().left)
		m_free.insert(m_free.begin(), iv);
	else if (id > m_free.back().right)
		m_free.insert(m_free.end(), iv);
	else
	{
		for (size_t i = 0, j = 1; j < m_free.size(); i = j++)
		{
			if (m_free[i].right < id && m_free[j].left > id)
			{
				m_free.insert(m_free.begin() + j, iv);
				break;
			}
		}
	}

	// Merge free intervals.
	for (size_t i = 0, j = 1; j < m_free.size(); )
	{
		if (m_free[i].right + 1 == m_free[j].left)
		{
			m_free[i].right = m_free[j].right;
			m_free.erase(m_free.begin() + j);
		}
		else
			i = j++;
	}
}

void IdAllocator::freeSequential(uint32_t id, uint32_t span)
{
	T_ASSERT(span > 0);
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

bool IdAllocator::Interval::operator == (const Interval& rh) const
{
	return (left == rh.left) && (right == rh.right);
}

}
