#ifndef traktor_sound_SurroundFilter_H
#define traktor_sound_SurroundFilter_H

#include "Sound/IFilter.h"
#include "Core/Math/Vector4.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

class SurroundEnvironment;

/*! \brief Surround filter.
 * \ingroup Sound
 */
class T_DLLCLASS SurroundFilter : public IFilter
{
	T_RTTI_CLASS;

public:
	SurroundFilter(SurroundEnvironment* environment);

	void setSpeakerPosition(const Vector4& position);

	virtual void apply(SoundBlock& outBlock);

private:
	Ref< SurroundEnvironment > m_environment;
	Vector4 m_speakerPosition;
	float m_buffer[SbcMaxChannelCount][4096];
};

	}
}

#endif	// traktor_sound_SurroundFilter_H
