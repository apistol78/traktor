#ifndef traktor_flash_FlashText_H
#define traktor_flash_FlashText_H

#include "Core/Math/Matrix33.h"
#include "Flash/FlashCharacter.h"
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
class T_DLLCLASS FlashText : public FlashCharacter
{
	T_RTTI_CLASS;

public:
	struct Character
	{
		uint16_t fontId;
		int16_t offsetX;
		int16_t offsetY;
		int16_t height;
		SwfColor color;
		union
		{
			uint32_t glyphIndex;
			uint16_t code;
		};

		void serialize(ISerializer& s);
	};

	FlashText();

	FlashText(uint16_t id, const Aabb2& textBounds, const Matrix33& textMatrix);

	bool create(const AlignedVector< SwfTextRecord* >& textRecords);

	const AlignedVector< Character >& getCharacters() const;

	virtual Ref< FlashCharacterInstance > createInstance(
		ActionContext* context,
		FlashCharacterInstance* parent,
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
	AlignedVector< Character > m_characters;
};

	}
}

#endif	// traktor_flash_FlashText_H
