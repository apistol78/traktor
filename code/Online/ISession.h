#ifndef traktor_online_ISession_H
#define traktor_online_ISession_H

#include "Core/Object.h"
#include "Core/RefArray.h"

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

class IUser;
class IAchievement;
class ISaveGame;

/*! \brief Single game session.
* \ingroup Online
*/
class T_DLLCLASS ISession : public Object
{
	T_RTTI_CLASS;

public:
	/*! \brief Destroy session. */
	virtual void destroy() = 0;

	/*! \brief Query if we're currently connected to service.
	 *
	 * \return True if connected; otherwise we're offline.
	 */
	virtual bool isConnected() const = 0;

	/*! \brief User playing this session.
	 *
	 * \return Playing user.
	 */
	virtual Ref< IUser > getUser() = 0;

	/*! \brief Get array of available achievements.
	 *
	 * \param outAchievements Array of achievements.
	 * \return True if achievements are available.
	 */
	virtual bool getAvailableAchievements(RefArray< IAchievement >& outAchievements) const = 0;

	/*! \brief Create save game.
	 *
	 * \param name Name of saved game.
	 * \param attachment Save game attachment.
	 * \return Save game description object; null if unable to save game.
	 */
	virtual Ref< ISaveGame > createSaveGame(const std::wstring& name, ISerializable* attachment) = 0;

	/*! \brief Get available save games.
	 *
	 * \param outSaveGames Array of saved game states.
	 * \return True if array recovered successfully.
	 */
	virtual bool getAvailableSaveGames(RefArray< ISaveGame >& outSaveGames) const = 0;
};

	}
}

#endif	// traktor_online_ISession_H
