#ifndef traktor_spark_SoundComponentInstance_H
#define traktor_spark_SoundComponentInstance_H

#include "Core/Ref.h"
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
	SoundComponentInstance(sound::ISoundPlayer* soundPlayer, const resource::Proxy< sound::Sound >& sound);

	Ref< sound::ISoundHandle > play();

	virtual void update();

private:
	Ref< sound::ISoundPlayer > m_soundPlayer;
	resource::Proxy< sound::Sound > m_sound;
};

	}
}

#endif	// traktor_spark_SoundComponentInstance_H
