#pragma once

#include "Sound/Editor/IStreamEncoder.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

/*! \brief
 * \ingroup Sound
 */
class T_DLLCLASS TssStreamEncoder : public IStreamEncoder
{
	T_RTTI_CLASS;

public:
	virtual bool create(IStream* stream) T_OVERRIDE T_FINAL;

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual bool putBlock(SoundBlock& block) T_OVERRIDE T_FINAL;

private:
	Ref< IStream > m_stream;
};

	}
}
