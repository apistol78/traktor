#ifndef traktor_flash_FlashTextInstance_H
#define traktor_flash_FlashTextInstance_H

#include "Flash/FlashCharacterInstance.h"

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

class FlashText;

/*! \brief Flash static text instance.
 * \ingroup Flash
 */
class T_DLLCLASS FlashTextInstance : public FlashCharacterInstance
{
	T_RTTI_CLASS;

public:
	FlashTextInstance(ActionContext* context, FlashDictionary* dictionary, FlashCharacterInstance* parent, const FlashText* text);

	const FlashText* getText() const;

	virtual Aabb2 getBounds() const T_OVERRIDE T_FINAL;

private:
	Ref< const FlashText > m_text;
};

	}
}

#endif	// traktor_flash_FlashTextInstance_H
