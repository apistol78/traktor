#ifndef traktor_spark_SoundComponentInstance_H
#define traktor_spark_SoundComponentInstance_H

#include "Core/Ref.h"
#include "Core/Containers/SmallMap.h"
#include "Resource/Proxy.h"
#include "Spark/IComponentInstance.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

class ISoundHandle;
class ISoundPlayer;
class Sound;

	}

	namespace spark
	{

/*! \brief Sound component instance.
 * \ingroup Spark
 */
class T_DLLCLASS SoundComponentInstance : public IComponentInstance
{
	T_RTTI_CLASS;

public:
	SoundComponentInstance(sound::ISoundPlayer* soundPlayer, const SmallMap< std::wstring, resource::Proxy< sound::Sound > >& sounds);

	Ref< sound::ISoundHandle > play(const std::wstring& id);

	virtual void update();

	virtual void eventKey(wchar_t unicode);

	virtual void eventKeyDown(int32_t keyCode);

	virtual void eventKeyUp(int32_t keyCode);

	virtual void eventMouseDown(const Vector2& position, int32_t button);

	virtual void eventMouseUp(const Vector2& position, int32_t button);

	virtual void eventMouseMove(const Vector2& position, int32_t button);

	virtual void eventMouseWheel(const Vector2& position, int32_t delta);

	virtual void eventViewResize(int32_t width, int32_t height);

private:
	Ref< sound::ISoundPlayer > m_soundPlayer;
	SmallMap< std::wstring, resource::Proxy< sound::Sound > > m_sounds;
};

	}
}

#endif	// traktor_spark_SoundComponentInstance_H
