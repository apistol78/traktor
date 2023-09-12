/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
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

/*! Frame actions.
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
		uint16_t hasFlags = 0;
		uint16_t depth = 0;
		uint8_t bitmapCaching = 0;
		uint8_t visible = 1;
		uint8_t blendMode = 0;
		uint8_t filter = 0;
		Color4f filterColor;
		uint16_t clipDepth = 0;
		std::string name;
		uint16_t ratio = 0;
		ColorTransform cxTransform;
		Matrix33 matrix;
		uint16_t characterId = 0;

		bool has(PlaceFlags placeFlag) const
		{
			return (hasFlags & placeFlag) != 0;
		}

		void serialize(ISerializer& s);
	};

	struct RemoveObject
	{
		bool hasCharacterId = false;
		uint16_t depth = 0;
		uint16_t characterId = 0;

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

	/*! Get next available depth.
	 */
	uint16_t nextUnusedDepth() const;

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

	/*!
	 */
	virtual void serialize(ISerializer& s) override final;

private:
	std::string m_label;
	bool m_backgroundColorChange;
	Color4f m_backgroundColor;
	SmallMap< uint16_t, PlaceObject > m_placeObjects;
	SmallMap< uint16_t, RemoveObject > m_removeObjects;
	AlignedVector< uint16_t > m_startSounds;
};

	}
}

