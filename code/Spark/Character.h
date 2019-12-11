#pragma once

#include "Core/Ref.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Math/Matrix33.h"
#include "Core/Serialization/ISerializable.h"
#include "Spark/SwfTypes.h"

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

class ActionContext;
class ActionObject;
class CharacterInstance;
class Dictionary;
class IActionVMImage;

/*! Character definition.
 * \ingroup Spark
 */
class T_DLLCLASS Character : public ISerializable
{
	T_RTTI_CLASS;

public:
	Character();

	Character(uint16_t id);

	/*! Get character identification.
	 *
	 * \return Id
	 */
	uint16_t getId() const { return m_id; }

	/*! Get character unique tag.
	 *
	 * The tag is guaranteed to be unique during the
	 * life-time of the running process even
	 * if multiple Flash players are created
	 * and destroyed during this time.
	 */
	int32_t getCacheTag() const { return m_tag; }

	/*! Create character instance.
	 *
	 * \param context ActionScript execution context.
	 * \param parent Parent instance.
	 * \param name Character name.
	 * \param transform Character transform.
	 * \param initObject Initialization object.
	 * \return Character instance.
	 */
	virtual Ref< CharacterInstance > createInstance(
		ActionContext* context,
		Dictionary* dictionary,
		CharacterInstance* parent,
		const std::string& name,
		const Matrix33& transform,
		const ActionObject* initObject,
		const SmallMap< uint32_t, Ref< const IActionVMImage > >* events
	) const = 0;

	/*! \brief
	 */
	virtual void serialize(ISerializer& s) override;

private:
	uint16_t m_id;
	int32_t m_tag;
};

	}
}

