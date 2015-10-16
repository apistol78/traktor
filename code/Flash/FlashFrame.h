#ifndef traktor_flash_FlashFrame_H
#define traktor_flash_FlashFrame_H

#include "Core/RefArray.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Math/Matrix33.h"
#include "Core/Serialization/ISerializable.h"
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

class IActionVMImage;

/*! \brief SWF frame actions.
 * \ingroup Flash
 *
 * Keep display list actions for a single
 * frame.
 */
class T_DLLCLASS FlashFrame : public ISerializable
{
	T_RTTI_CLASS;

public:
	enum PlaceFlags
	{
		PfHasBitmapCaching = 1 << 0,
		PfHasBlendMode = 1 << 1,
		PfHasFilters = 1 << 2,
		PfHasActions = 1 << 3,
		PfHasClipDepth = 1 << 4,
		PfHasName = 1 << 5,
		PfHasRatio = 1 << 6,
		PfHasCxTransform = 1 << 7,
		PfHasMatrix = 1 << 8,
		PfHasCharacterId = 1 << 9,
		PfHasMove = 1 << 10,
		PfHasOpaqueBackground = 1 << 11,
		PfHasVisible = 1 << 12
	};

	struct PlaceObject
	{
		uint16_t hasFlags;
		uint16_t depth;
		uint8_t bitmapCaching;
		uint8_t visible;
		uint8_t blendMode;
		uint8_t filter;
		SwfColor filterColor;
		SmallMap< uint32_t, Ref< const IActionVMImage > > events;
		uint16_t clipDepth;
		std::string name;
		uint16_t ratio;
		SwfCxTransform cxTransform;
		Matrix33 matrix;
		uint16_t characterId;

		PlaceObject()
		:	hasFlags(0)
		,	depth(0)
		,	bitmapCaching(0)
		,	visible(1)
		,	blendMode(0)
		,	filter(0)
		{
		}

		bool has(PlaceFlags placeFlag) const
		{
			return (hasFlags & placeFlag) != 0;
		}

		void serialize(ISerializer& s);
	};

	struct RemoveObject
	{
		bool hasCharacterId;
		uint16_t depth;
		uint16_t characterId;

		RemoveObject()
		:	hasCharacterId(false)
		,	depth(0)
		{
		}

		void serialize(ISerializer& s);
	};

	FlashFrame();

	/*! \brief Set frame label.
	 *
	 * \param label Frame label.
	 */
	void setLabel(const std::string& label);

	/*! \brief Get frame label.
	 *
	 * \return Frame label.
	 */
	const std::string& getLabel() const;

	/*! \brief Change background color used to clear frame.
	 *
	 * \param backgroundColor Background color.
	 */
	void changeBackgroundColor(const SwfColor& backgroundColor);

	/*! \brief Check if this frame changes background color.
	 *
	 * \return True if background color changed.
	 */
	bool hasBackgroundColorChanged() const;

	/*! \brief Get background color.
	 *
	 * \return Background color.
	 */
	const SwfColor& getBackgroundColor() const;

	/*! \brief Place object onto display list.
	 *
	 * \param placeObject Place object description.
	 */
	void placeObject(const PlaceObject& placeObject);

	/*! \brief Remove object from display list.
	 *
	 * \param removeObject Remove object description.
	 */
	void removeObject(const RemoveObject& removeObject);

	/*! \brief Add frame action script.
	 *
	 * \param actionScript Frame action script.
	 */
	void addActionScript(const IActionVMImage* actionScript);

	/*! \brief Start sound.
	 */
	void startSound(uint16_t soundId);

	/*! \brief Get place object descriptions by this frame.
	 *
	 * \return Place object descriptions.
	 */
	const SmallMap< uint16_t, PlaceObject >& getPlaceObjects() const;

	/*! \brief Get remove object descriptions by this frame.
	 *
	 * \return Remove object descriptions.
	 */
	const SmallMap< uint16_t, RemoveObject >& getRemoveObjects() const;

	/*! \brief Get start sounds.
	 */
	const AlignedVector< uint16_t >& getStartSounds() const;

	/*! \brief Get action scripts associated with this frame.
	 *
	 * \return Action scripts.
	 */
	const RefArray< const IActionVMImage >& getActionScripts() const;

	/*! \brief
	 */
	virtual void serialize(ISerializer& s);

private:
	std::string m_label;
	bool m_backgroundColorChange;
	SwfColor m_backgroundColor;
	SmallMap< uint16_t, PlaceObject > m_placeObjects;
	SmallMap< uint16_t, RemoveObject > m_removeObjects;
	AlignedVector< uint16_t > m_startSounds;
	RefArray< const IActionVMImage > m_actionScripts;
};

	}
}

#endif	// traktor_flash_FlashFrame_H
