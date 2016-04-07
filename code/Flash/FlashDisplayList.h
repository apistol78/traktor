#ifndef traktor_flash_FlashDisplayList_H
#define traktor_flash_FlashDisplayList_H

#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Containers/SmallMap.h"
#include "Flash/FlashCharacterInstance.h"
#include "Flash/SwfTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

class ActionContext;
class FlashMovie;
class FlashFrame;

/*! \brief Movie clip display list.
 * \ingroup Flash
 */
class T_DLLCLASS FlashDisplayList : public Object
{
	T_RTTI_CLASS;

public:
	struct Layer
	{
		uint16_t id;
		uint32_t name;
		Ref< FlashCharacterInstance > instance;
		bool clipEnable;
		int32_t clipDepth;
		bool immutable;
		bool collect;

		Layer()
		:	id(0)
		,	name(0)
		,	clipEnable(false)
		,	clipDepth(0)
		,	immutable(false)
		,	collect(false)
		{
		}
	};

	typedef SmallMap< int32_t, Layer > layer_map_t;

	FlashDisplayList(ActionContext* context);

	/*! \brief Reset display list. */
	void reset();

	/*! \brief Begin update display list.
	 *
	 * \param reset If display list should be considered "empty"; all characters not placed are removed when update are finished.
	 */
	void updateBegin(bool reset);

	/*! \brief End updating display list.
	 *
	 * Not "placed" characters are removed.
	 */
	void updateEnd();

	/*! \brief Update display list from frame.
	 *
	 * \param ownerInstance Instance which will be parent to other character instances created from this frame.
	 * \param frame Frame actions.
	 */
	void updateFrame(FlashCharacterInstance* ownerInstance, const FlashFrame* frame);

	/*! \brief Show character instance at depth.
	 *
	 * \param depth Depth to place character instance.
	 * \param characterId Identifier of character.
	 * \param characterInstance Character instance.
	 * \param immutable If object is immutable; ie object manually places through AS which shouldn't be removed.
	 */
	void showObject(int32_t depth, uint16_t characterId, FlashCharacterInstance* characterInstance, bool immutable);

	/*! \brief Remove character instance from display list.
	 *
	 * \param characterInstance Character instance.
	 */
	void removeObject(FlashCharacterInstance* characterInstance);

	/*! \brief Get depth where character instance is placed.
	 *
	 * \param characterInstance Character instance.
	 * \return Depth of instance.
	 */
	int32_t getObjectDepth(const FlashCharacterInstance* characterInstance) const;

	/*! \brief Get next highest depth which is free.
	 *
	 * \return Next highest depth.
	 */
	int32_t getNextHighestDepth() const;

	/*! \brief Swap character instances at two depths.
	 *
	 * \param depth1 First depth.
	 * \param depth2 Second depth.
	 */
	void swap(int32_t depth1, int32_t depth2);

	/*! \brief Get character instances.
	 *
	 * \param outCharacterInstances Output array of character instances.
	 */
	void getObjects(RefArray< FlashCharacterInstance >& outCharacterInstances) const;

	/*! \brief For each visible character instances.
	 *
	 * \param fn Callback function.
	 */
	template < typename fn_t >
	void forEachVisibleObject(fn_t fn) const
	{
		RefArray< FlashCharacterInstance > tmp;
		for (FlashDisplayList::layer_map_t::const_iterator i = m_layers.begin(); i != m_layers.end(); ++i)
		{
			T_ASSERT (i->second.instance);
			if (i->second.instance->isVisible())
				tmp.push_back(i->second.instance);
		}
		for (RefArray< FlashCharacterInstance >::const_iterator i = tmp.begin(); i != tmp.end(); ++i)
			fn(*i);
	}

	/*! \brief For each visible character instances in reverse.
	 *
	 * \param fn Callback function.
	 */
	template < typename fn_t >
	void forEachVisibleObjectReverse(fn_t fn) const
	{
		RefArray< FlashCharacterInstance > tmp;
		for (FlashDisplayList::layer_map_t::const_reverse_iterator i = m_layers.rbegin(); i != m_layers.rend(); ++i)
		{
			T_ASSERT (i->second.instance);
			if (i->second.instance->isVisible())
				tmp.push_back(i->second.instance);
		}
		for (RefArray< FlashCharacterInstance >::const_iterator i = tmp.begin(); i != tmp.end(); ++i)
			fn(*i);
	}

	/*! \brief Get background clear color.
	 *
	 * \return Background colors.
	 */
	const SwfColor& getBackgroundColor() const { return m_backgroundColor; }

	/*! \brief Get depth layers.
	 *
	 * \return Layers.
	 */
	const layer_map_t& getLayers() const { return m_layers; }

private:
	ActionContext* m_context;
	SwfColor m_backgroundColor;
	layer_map_t m_layers;
};

	}
}

#endif	// traktor_flash_FlashDisplayList_H
