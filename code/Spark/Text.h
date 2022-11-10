/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/Aabb2.h"
#include "Core/Math/Color4f.h"
#include "Core/Math/Matrix33.h"
#include "Spark/Character.h"

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

struct SwfTextRecord;

/*! Static text.
 * \ingroup Spark
 */
class T_DLLCLASS Text : public Character
{
	T_RTTI_CLASS;

public:
	struct Char
	{
		uint16_t fontId;
		int16_t offsetX;
		int16_t offsetY;
		int16_t height;
		Color4f color;
		union
		{
			uint32_t glyphIndex;
			uint16_t code;
		};

		void serialize(ISerializer& s);
	};

	Text();

	explicit Text(const Aabb2& textBounds, const Matrix33& textMatrix);

	bool create(const AlignedVector< SwfTextRecord* >& textRecords);

	const AlignedVector< Char >& getCharacters() const;

	virtual Ref< CharacterInstance > createInstance(
		Context* context,
		Dictionary* dictionary,
		CharacterInstance* parent,
		const std::string& name,
		const Matrix33& transform
	) const override final;

	const Aabb2& getTextBounds() const;

	const Matrix33& getTextMatrix() const;

	virtual void serialize(ISerializer& s) override final;

private:
	Aabb2 m_textBounds;
	Matrix33 m_textMatrix;
	AlignedVector< Char > m_characters;
};

	}
}

