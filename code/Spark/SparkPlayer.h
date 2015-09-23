#ifndef traktor_spark_SparkPlayer_H
#define traktor_spark_SparkPlayer_H

#include "Core/Object.h"

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

class CharacterInstance;

/*! \brief
 * \ingroup Spark
 */
class T_DLLCLASS SparkPlayer : public Object
{
	T_RTTI_CLASS;

public:
	/*! \brief Construct player.
	 */
	SparkPlayer(CharacterInstance* character);

	/*! \brief Update player.
	 */
	void update();

	/*! \brief Post key event.
	 *
	 * \param unicode Unicode character.
	 */
	void postKey(wchar_t unicode);

	/*! \brief Post key down event.
	 *
	 * \param keyCode Key code.
	 */
	void postKeyDown(int32_t keyCode);

	/*! \brief Post key up event.
	 *
	 * \param keyCode Key code.
	 */
	void postKeyUp(int32_t keyCode);

	/*! \brief Post mouse button down event.
	 *
	 * \param x Mouse cursor X coordinate.
	 * \param y Mouse cursor Y coordinate.
	 * \param button Mouse buttons being held down.
	 */
	void postMouseDown(int32_t x, int32_t y, int32_t button);

	/*! \brief Post mouse button up event.
	 *
	 * \param x Mouse cursor X coordinate.
	 * \param y Mouse cursor Y coordinate.
	 * \param button Mouse buttons being held down.
	 */
	void postMouseUp(int32_t x, int32_t y, int32_t button);

	/*! \brief Post mouse move event.
	 *
	 * \param x Mouse cursor X coordinate.
	 * \param y Mouse cursor Y coordinate.
	 * \param button Mouse buttons being held down.
	 */
	void postMouseMove(int32_t x, int32_t y, int32_t button);

	/*! \brief Post mouse wheel event.
	 *
	 * \param x Mouse cursor X coordinate.
	 * \param y Mouse cursor Y coordinate.
	 * \param delta Wheel delta motion
	 */
	void postMouseWheel(int32_t x, int32_t y, int32_t delta);

	/*! \brief Post view resize event.
	 *
	 * \param width New view width.
	 * \param height New view height.
	 */
	void postViewResize(int32_t width, int32_t height);

private:
	Ref< CharacterInstance > m_character;
};

	}
}

#endif	// traktor_spark_SparkPlayer_H
