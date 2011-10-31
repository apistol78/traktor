#ifndef traktor_flash_FlashCharacter_H
#define traktor_flash_FlashCharacter_H

#include "Core/Object.h"
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
class ActionObject;
class FlashCharacterInstance;

/*! \brief Character definition.
 * \ingroup Flash
 */
class T_DLLCLASS FlashCharacter : public Object
{
	T_RTTI_CLASS;

public:
	FlashCharacter(uint16_t id);

	/*! \brief Get character identification.
	 *
	 * \return Id
	 */
	uint16_t getId() const;

	/*! \brief Create character instance.
	 *
	 * \param context ActionScript execution context.
	 * \param parent Parent instance.
	 * \param name Character name.
	 * \param initObject Initialization object.
	 * \return Character instance.
	 */
	virtual Ref< FlashCharacterInstance > createInstance(ActionContext* context, FlashCharacterInstance* parent, const std::string& name, const ActionObject* initObject) const = 0;

private:
	uint16_t m_id;
};

	}
}

#endif	// traktor_flash_FlashCharacter_H
