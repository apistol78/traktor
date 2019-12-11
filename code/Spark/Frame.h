#pragma once

#include "Core/RefArray.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Math/Matrix33.h"
#include "Core/Serialization/ISerializable.h"
#include "Spark/ColorTransform.h"

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

class IActionVMImage;

/*! SWF frame actions.
 * \ingroup Spark
 *
 * Keep display list actions for a single
 * frame.
 */
class T_DLLCLASS Frame : public ISerializable
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
		Color4f filterColor;
		SmallMap< uint32_t, Ref< const IActionVMImage > > events;
		uint16_t clipDepth;
		std::string name;
		uint16_t ratio;
		ColorTransform cxTransform;
		Matrix33 matrix;
		uint16_t characterId;

		PlaceObject()
		:	hasFlags(0)
		,	depth(0)
		,	bitmapCaching(0)
		,	visible(1)
		,	blendMode(0)
		,	filter(0)
		,	clipDepth(0)
		,	ratio(0)
		,	characterId(0)
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

	Frame();

	/*! Set frame label.
	 *
	 * \param label Frame label.
	 */
	void setLabel(const std::string& label);

	/*! Get frame label.
	 *
	 * \return Frame label.
	 */
	const std::string& getLabel() const;

	/*! Change background color used to clear frame.
	 *
	 * \param backgroundColor Background color.
	 */
	void changeBackgroundColor(const Color4f& backgroundColor);

	/*! Check if this frame changes background color.
	 *
	 * \return True if background color changed.
	 */
	bool hasBackgroundColorChanged() const;

	/*! Get background color.
	 *
	 * \return Background color.
	 */
	const Color4f& getBackgroundColor() const;

	/*! Place object onto display list.
	 *
	 * \param placeObject Place object description.
	 */
	void placeObject(const PlaceObject& placeObject);

	/*! Remove object from display list.
	 *
	 * \param removeObject Remove object description.
	 */
	void removeObject(const RemoveObject& removeObject);

	/*! Add frame action script.
	 *
	 * \param actionScript Frame action script.
	 */
	void addActionScript(const IActionVMImage* actionScript);

	/*! Start sound.
	 */
	void startSound(uint16_t soundId);

	/*! Get place object descriptions by this frame.
	 *
	 * \return Place object descriptions.
	 */
	const SmallMap< uint16_t, PlaceObject >& getPlaceObjects() const;

	/*! Get remove object descriptions by this frame.
	 *
	 * \return Remove object descriptions.
	 */
	const SmallMap< uint16_t, RemoveObject >& getRemoveObjects() const;

	/*! Get start sounds.
	 */
	const AlignedVector< uint16_t >& getStartSounds() const;

	/*! Get action scripts associated with this frame.
	 *
	 * \return Action scripts.
	 */
	const RefArray< const IActionVMImage >& getActionScripts() const;

	/*! \brief
	 */
	virtual void serialize(ISerializer& s) override final;

private:
	std::string m_label;
	bool m_backgroundColorChange;
	Color4f m_backgroundColor;
	SmallMap< uint16_t, PlaceObject > m_placeObjects;
	SmallMap< uint16_t, RemoveObject > m_removeObjects;
	AlignedVector< uint16_t > m_startSounds;
	RefArray< const IActionVMImage > m_actionScripts;
};

	}
}

