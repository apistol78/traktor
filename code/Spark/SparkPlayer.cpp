#include "Spark/CharacterInstance.h"
#include "Spark/SparkPlayer.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.SparkPlayer", SparkPlayer, Object)

SparkPlayer::SparkPlayer(CharacterInstance* character)
:	m_character(character)
{
}

void SparkPlayer::update()
{
	m_character->update();
}

void SparkPlayer::postKey(wchar_t unicode)
{
	m_character->eventKey(unicode);
}

void SparkPlayer::postKeyDown(int32_t keyCode)
{
	m_character->eventKeyDown(keyCode);
}

void SparkPlayer::postKeyUp(int32_t keyCode)
{
	m_character->eventKeyUp(keyCode);
}

void SparkPlayer::postMouseDown(int32_t x, int32_t y, int32_t button)
{
	m_character->eventMouseDown(x, y, button);
}

void SparkPlayer::postMouseUp(int32_t x, int32_t y, int32_t button)
{
	m_character->eventMouseUp(x, y, button);
}

void SparkPlayer::postMouseMove(int32_t x, int32_t y, int32_t button)
{
	m_character->eventMouseMove(x, y, button);
}

void SparkPlayer::postMouseWheel(int32_t x, int32_t y, int32_t delta)
{
	m_character->eventMouseWheel(x, y, delta);
}

void SparkPlayer::postViewResize(int32_t width, int32_t height)
{
	m_character->eventViewResize(width, height);
}

	}
}
