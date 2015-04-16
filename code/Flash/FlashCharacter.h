#ifndef traktor_flash_FlashCharacter_H
#define traktor_flash_FlashCharacter_H

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

class ActionContext;
class ActionObject;
class FlashCharacterInstance;
class IActionVMImage;

/*! \brief Character definition.
 * \ingroup Flash
 */
class T_DLLCLASS FlashCharacter : public ISerializable
{
	T_RTTI_CLASS;

public:
	FlashCharacter();

	FlashCharacter(uint16_t id);

	/*! \brief Get character identification.
	 *
	 * \return Id
	 */
	uint16_t getId() const;

	/*! \brief Get character unique tag.
	 *
	 * The tag is guaranteed to be unique during the
	 * life-time of the running process even
	 * if multiple Flash players are created
	 * and destroyed during this time.
	 */
	int32_t getCacheTag() const;

	/*! \brief Create character instance.
	 *
	 * \param context ActionScript execution context.
	 * \param parent Parent instance.
	 * \param name Character name.
	 * \param transform Character transform.
	 * \param initObject Initialization object.
	 * \return Character instance.
	 */
	virtual Ref< FlashCharacterInstance > createInstance(
		ActionContext* context,
		FlashCharacterInstance* parent,
		const std::string& name,
		const Matrix33& transform,
		const ActionObject* initObject,
		const SmallMap< uint32_t, Ref< const IActionVMImage > >* events
	) const = 0;

	/*! \brief
	 */
	virtual void serialize(ISerializer& s);

private:
	uint16_t m_id;
	int32_t m_tag;
};

	}
}

#endif	// traktor_flash_FlashCharacter_H
