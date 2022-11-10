/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Containers/SmallMap.h"
#include "Spark/CharacterInstance.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spark
	{

class Context;
class Movie;
class Frame;

/*! Movie clip display list.
 * \ingroup Spark
 */
class T_DLLCLASS DisplayList : public Object
{
	T_RTTI_CLASS;

public:
	struct Layer
	{
		uint16_t id = 0;
		Ref< CharacterInstance > instance;
		bool clipEnable = false;
		int32_t clipDepth = 0;
		bool immutable = false;
		bool collect = false;
	};

	typedef SmallMap< int32_t, Layer > layer_map_t;

	explicit DisplayList(Context* context);

	/*! Reset display list. */
	void reset();

	/*! Begin update display list.
	 *
	 * \param reset If display list should be considered "empty"; all characters not placed are removed when update are finished.
	 */
	void updateBegin(bool reset);

	/*! End updating display list.
	 *
	 * Not "placed" characters are removed.
	 */
	void updateEnd();

	/*! Update display list from frame.
	 *
	 * \param ownerInstance Instance which will be parent to other character instances created from this frame.
	 * \param frame Frame actions.
	 */
	void updateFrame(CharacterInstance* ownerInstance, const Frame* frame);

	/*! Show character instance at depth.
	 *
	 * \param depth Depth to place character instance.
	 * \param characterInstance Character instance.
	 * \param immutable If object is immutable; ie object manually placed which shouldn't be removed.
	 */
	void showObject(int32_t depth, CharacterInstance* characterInstance, bool immutable);

	/*! Remove character instance from display list.
	 *
	 * \param characterInstance Character instance.
	 */
	bool removeObject(CharacterInstance* characterInstance);

	/*! Remove character instance from display list.
	 *
	 * \param characterInstance Character instance.
	 */
	bool removeObject(int32_t depth);

	/*! Get depth where character instance is placed.
	 *
	 * \param characterInstance Character instance.
	 * \return Depth of instance.
	 */
	int32_t getObjectDepth(const CharacterInstance* characterInstance) const;

	/*! Get next highest depth which is free.
	 *
	 * \return Next highest depth.
	 */
	int32_t getNextHighestDepth() const;

	/*! Get next highest depth in given range which is free.
	 *
	 * \param minDepth Minimum depth.
	 * \param maxDepth Maximum depth.
	 * \return Next highest depth.
	 */
	int32_t getNextHighestDepthInRange(int32_t minDepth, int32_t maxDepth) const;

	/*! Swap character instances at two depths.
	 *
	 * \param depth1 First depth.
	 * \param depth2 Second depth.
	 */
	void swap(int32_t depth1, int32_t depth2);

	/*! Get character instances.
	 *
	 * \param outCharacterInstances Output array of character instances.
	 */
	void getObjects(RefArray< CharacterInstance >& outCharacterInstances) const;

	/*! For each character instances.
	 *
	 * \param fn Callback function.
	 */
	template < typename fn_t >
	void forEachObject(fn_t fn) const
	{
		size_t f = m_gather.size();
		for (DisplayList::layer_map_t::const_iterator i = m_layers.begin(); i != m_layers.end(); ++i)
		{
			T_ASSERT(i->second.instance);
			m_gather.push_back(i->second.instance);
		}
		for (size_t i = f; i < m_gather.size(); ++i)
			fn(m_gather[i]);
		m_gather.resize(f);
	}

	/*! For each character instances.
	 *
	 * \param fn Callback function.
	 */
	template < typename fn_t >
	void forEachObjectDirect(fn_t fn) const
	{
		for (DisplayList::layer_map_t::const_iterator i = m_layers.begin(); i != m_layers.end(); ++i)
		{
			T_ASSERT(i->second.instance);
			fn(i->second.instance);
		}
	}

	/*! For each visible character instances.
	 *
	 * \param fn Callback function.
	 */
	template < typename fn_t >
	void forEachVisibleObject(fn_t fn) const
	{
		size_t f = m_gather.size();
		for (DisplayList::layer_map_t::const_iterator i = m_layers.begin(); i != m_layers.end(); ++i)
		{
			T_ASSERT(i->second.instance);
			if (i->second.instance->isVisible())
				m_gather.push_back(i->second.instance);
		}
		for (size_t i = f; i < m_gather.size(); ++i)
			fn(m_gather[i]);
		m_gather.resize(f);
	}

	/*! For each visible character instances.
	 *
	 * \param fn Callback function.
	 */
	template < typename fn_t >
	void forEachVisibleObjectDirect(fn_t fn) const
	{
		for (DisplayList::layer_map_t::const_iterator i = m_layers.begin(); i != m_layers.end(); ++i)
		{
			T_ASSERT(i->second.instance);
			if (i->second.instance->isVisible())
				fn(i->second.instance);
		}
	}

	/*! For each visible character instances in reverse.
	 *
	 * \param fn Callback function.
	 */
	template < typename fn_t >
	void forEachVisibleObjectReverse(fn_t fn) const
	{
		size_t f = m_gather.size();
		for (DisplayList::layer_map_t::const_reverse_iterator i = m_layers.rbegin(); i != m_layers.rend(); ++i)
		{
			T_ASSERT(i->second.instance);
			if (i->second.instance->isVisible())
				m_gather.push_back(i->second.instance);
		}
		for (size_t i = f; i < m_gather.size(); ++i)
			fn(m_gather[i]);
		m_gather.resize(f);
	}

	/*! Get background clear color.
	 *
	 * \return Background colors.
	 */
	const Color4f& getBackgroundColor() const { return m_backgroundColor; }

	/*! Get depth layers.
	 *
	 * \return Layers.
	 */
	const layer_map_t& getLayers() const { return m_layers; }

private:
	Context* m_context;
	Color4f m_backgroundColor;
	layer_map_t m_layers;
	mutable RefArray< CharacterInstance > m_gather;
};

	}
}

