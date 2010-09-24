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
class ILeaderboard;
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

	/*! \brief Reward playing user with an achievement.
	 *
	 * \return True if achievement was successfully received.
	 */
	virtual bool rewardAchievement(const std::wstring& achievementId) = 0;

	/*! \brief Withdraw playing user of an achievement.
	 *
	 * \return True if achievement was successfully withdrawn.
	 */
	virtual bool withdrawAchievement(const std::wstring& achievementId) = 0;

	/*! \brief Have achievement.
	 *
	 * \return True if achievement is awared.
	 */
	virtual bool haveAchievement(const std::wstring& achievementId) = 0;

	/*! \brief Get leaderboard.
	 *
	 * \param id Leaderboard id.
	 * \return Leaderboard instance.
	 */
	virtual Ref< ILeaderboard > getLeaderboard(const std::wstring& id) = 0;

	/*! \brief Set value.
	 *
	 * \return True if stat was successfully updated.
	 */
	virtual bool setStatValue(const std::wstring& statId, float value) = 0;

	/*! \brief Get value.
	 *
	 * \return True if stat was successfully received.
	 */
	virtual bool getStatValue(const std::wstring& statId, float& outValue) = 0;

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
