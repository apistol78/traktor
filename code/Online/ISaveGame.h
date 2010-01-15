#ifndef traktor_online_ISaveGame_H
#define traktor_online_ISaveGame_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ONLINE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif 

namespace traktor
{

class ISerializable;

	namespace online
	{

/*! \brief Saved game state.
 * \ingroup Online
 */
class T_DLLCLASS ISaveGame : public Object
{
	T_RTTI_CLASS;

public:
	/*! \brief Human readable description of saved game.
	 *
	 * \return Save game description.
	 */
	virtual std::wstring getName() const = 0;

	/*! \brief Game specific attachment.
	 *
	 * Games are responsible of attaching meaningful
	 * data which enables the game to recover to
	 * the saved state.
	 *
	 * \return Attachment.
	 */
	virtual Ref< ISerializable > getAttachment() const = 0;
};

	}
}

#endif	// traktor_online_ISaveGame_H
