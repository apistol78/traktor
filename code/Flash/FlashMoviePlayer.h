#ifndef traktor_flash_FlashMoviePlayer_H
#define traktor_flash_FlashMoviePlayer_H

#include <list>
#include <map>
#include "Core/Object.h"

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

class ActionObject;
class ActionFunction;
class ActionValue;
class AsKey;
class AsMouse;
class FlashMovie;
class FlashMovieRenderer;
class FlashSpriteInstance;
class IActionVM;
class IDisplayRenderer;

struct CallArgs;

/*! \brief Flash movie player.
 * \ingroup Flash
 *
 * Flash movie player is the main class used by applications
 * in order to properly play SWF movies.
 */
class T_DLLCLASS FlashMoviePlayer : public Object
{
	T_RTTI_CLASS;

public:
	FlashMoviePlayer(IDisplayRenderer* displayRenderer);

	virtual ~FlashMoviePlayer();

	/*! \brief Create movie player.
	 *
	 * \param movie Root movie.
	 * \return True if created successfully.
	 */
	bool create(FlashMovie* movie);

	/*! \brief Destroy resources used by movie player. */
	void destroy();

	/*! \brief Goto frame and continue playing.
	 *
	 * \param frame Frame number.
	 */
	void gotoAndPlay(uint32_t frame);

	/*! \brief Goto frame and stop playing.
	 *
	 * \param frame Frame number.
	 */
	void gotoAndStop(uint32_t frame);

	/*! \brief Goto frame and continue playing.
	 *
	 * \param frameLabel Frame label.
	 */
	void gotoAndPlay(const std::string& frameLabel);

	/*! \brief Goto frame and stop playing.
	 *
	 * \param frameLabel Frame label.
	 */
	void gotoAndStop(const std::string& frameLabel);

	/*! \brief Get number of frames in root movie.
	 *
	 * \return Number of frames.
	 */
	uint32_t getFrameCount() const;

	/*! \brief Render frame using associated display renderer. */
	void renderFrame();

	/*! \brief Execute events in current frame. */
	void executeFrame();

	/*! \brief Progress until next frame.
	 *
	 * \param deltaTime Time in seconds to progress movie.
	 * \return True if new frame.
	 */
	bool progressFrame(float deltaTime);

	/*! \brief Post key down event.
	 *
	 * \param keyCode Key code.
	 */
	void postKeyDown(int keyCode);

	/*! \brief Post key up event.
	 *
	 * \param keyCode Key code.
	 */
	void postKeyUp(int keyCode);

	/*! \brief Post mouse button down event.
	 *
	 * \param x Mouse cursor X coordinate.
	 * \param y Mouse cursor Y coordinate.
	 * \param button Mouse buttons being held down.
	 */
	void postMouseDown(int x, int y, int button);

	/*! \brief Post mouse button up event.
	 *
	 * \param x Mouse cursor X coordinate.
	 * \param y Mouse cursor Y coordinate.
	 * \param button Mouse buttons being held down.
	 */
	void postMouseUp(int x, int y, int button);

	/*! \brief Post mouse move event.
	 *
	 * \param x Mouse cursor X coordinate.
	 * \param y Mouse cursor Y coordinate.
	 * \param button Mouse buttons being held down.
	 */
	void postMouseMove(int x, int y, int button);

	/*! \brief Pop FS command from queue.
	 *
	 * \param outCommand Output FS command.
	 * \param outArgs Output FS arguments.
	 * \return True any command was popped.
	 */
	bool getFsCommand(std::wstring& outCommand, std::wstring& outArgs);

	/*! \brief Get root movie instance.
	 *
	 * \return Root movie instance.
	 */
	FlashSpriteInstance* getMovieInstance() const;

	/*! \brief Get action virtual machine.
	 *
	 * \return action virtual machine.
	 */
	IActionVM* getVM() const;

	/*! \brief Set global ActionScript value.
	 *
	 * \param name Name of global member.
	 * \param value Value of global member.
	 */
	void setGlobal(const std::string& name, const ActionValue& value);

	/*! \brief Get global ActionScript value.
	 *
	 * \param name Name of global member.
	 * \return Value of global member; undefined if no such member.
	 */
	ActionValue getGlobal(const std::string& name) const;

private:
	struct Event
	{
		uint32_t eventType;
		union
		{
			int keyCode;
			struct
			{
				int x;
				int y;
				int button;
			}
			mouse;
		};
	};

	struct Interval
	{
		uint32_t last;
		uint32_t interval;
		Ref< ActionObject > target;
		Ref< ActionFunction > function;
	};

	Ref< IDisplayRenderer > m_displayRenderer;
	Ref< FlashMovieRenderer > m_movieRenderer;
	Ref< IActionVM > m_actionVM;
	Ref< AsKey > m_key;
	Ref< AsMouse > m_mouse;
	Ref< FlashMovie > m_movie;
	Ref< FlashSpriteInstance > m_movieInstance;
	std::list< Event > m_events;
	std::list< std::pair< std::wstring, std::wstring > > m_fsCommands;
	std::map< uint32_t, Interval > m_interval;
	uint32_t m_intervalNextId;
	double m_timeCurrent;
	double m_timeNext;
	double m_timeNextFrame;
	int32_t m_framesUntilCollection;

	void Global_getUrl(CallArgs& ca);

	void Global_setInterval(CallArgs& ca);

	void Global_clearInterval(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_FlashMoviePlayer_H
