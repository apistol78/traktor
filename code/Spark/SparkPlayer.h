/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_spark_SparkPlayer_H
#define traktor_spark_SparkPlayer_H

#include "Core/Object.h"
#include "Core/Math/Vector2.h"

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

class Character;

/*! \brief
 * \ingroup Spark
 */
class T_DLLCLASS SparkPlayer : public Object
{
	T_RTTI_CLASS;

public:
	/*! \brief Construct player.
	 */
	SparkPlayer(Character* character);

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
	 * \param position Mouse coordinate in view space.
	 * \param button Mouse buttons being held down.
	 */
	void postMouseDown(const Vector2& position, int32_t button);

	/*! \brief Post mouse button up event.
	 *
	 * \param position Mouse coordinate in view space.
	 * \param button Mouse buttons being held down.
	 */
	void postMouseUp(const Vector2& position, int32_t button);

	/*! \brief Post mouse move event.
	 *
	 * \param position Mouse coordinate in view space.
	 * \param button Mouse buttons being held down.
	 */
	void postMouseMove(const Vector2& position, int32_t button);

	/*! \brief Post mouse wheel event.
	 *
	 * \param position Mouse coordinate in view space.
	 * \param delta Wheel delta motion
	 */
	void postMouseWheel(const Vector2& position, int32_t delta);

	/*! \brief Post view resize event.
	 *
	 * \param width New view width.
	 * \param height New view height.
	 */
	void postViewResize(int32_t width, int32_t height);

private:
	Ref< Character > m_character;
};

	}
}

#endif	// traktor_spark_SparkPlayer_H
