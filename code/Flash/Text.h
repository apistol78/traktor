/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_Text_H
#define traktor_flash_Text_H

#include "Core/Math/Matrix33.h"
#include "Flash/Character.h"
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

/*! \brief Flash static text.
 * \ingroup Flash
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

	Text(uint16_t id, const Aabb2& textBounds, const Matrix33& textMatrix);

	bool create(const AlignedVector< SwfTextRecord* >& textRecords);

	const AlignedVector< Char >& getCharacters() const;

	virtual Ref< CharacterInstance > createInstance(
		ActionContext* context,
		Dictionary* dictionary,
		CharacterInstance* parent,
		const std::string& name,
		const Matrix33& transform,
		const ActionObject* initObject,
		const SmallMap< uint32_t, Ref< const IActionVMImage > >* events
	) const T_OVERRIDE T_FINAL;

	const Aabb2& getTextBounds() const;

	const Matrix33& getTextMatrix() const;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	Aabb2 m_textBounds;
	Matrix33 m_textMatrix;
	AlignedVector< Char > m_characters;
};

	}
}

#endif	// traktor_flash_Text_H
