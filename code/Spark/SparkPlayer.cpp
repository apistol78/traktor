#include "Spark/Character.h"
#include "Spark/SparkPlayer.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.SparkPlayer", SparkPlayer, Object)

SparkPlayer::SparkPlayer(Character* character)
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

void SparkPlayer::postMouseDown(const Vector2& position, int32_t button)
{
	m_character->eventMouseDown(position, button);
}

void SparkPlayer::postMouseUp(const Vector2& position, int32_t button)
{
	m_character->eventMouseUp(position, button);
}

void SparkPlayer::postMouseMove(const Vector2& position, int32_t button)
{
	m_character->eventMouseMove(position, button);
}

void SparkPlayer::postMouseWheel(const Vector2& position, int32_t delta)
{
	m_character->eventMouseWheel(position, delta);
}

void SparkPlayer::postViewResize(int32_t width, int32_t height)
{
	m_character->eventViewResize(width, height);
}

	}
}
