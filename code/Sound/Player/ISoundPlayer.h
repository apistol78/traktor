/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_sound_ISoundPlayer_H
#define traktor_sound_ISoundPlayer_H

#include "Core/Object.h"
#include "Core/Math/Transform.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

class ISoundHandle;
class Sound;

/*! \brief High-level sound player.
 * \ingroup Sound
 */
class T_DLLCLASS ISoundPlayer : public Object
{
	T_RTTI_CLASS;

public:
	virtual void destroy() = 0;

	/*! \brief Play global sound.
	 */
	virtual Ref< ISoundHandle > play(const Sound* sound, uint32_t priority) = 0;

	/*! \brief Play positional sound.
	 */
	virtual Ref< ISoundHandle > play3d(const Sound* sound, const Vector4& position, uint32_t priority, bool autoStopFar) = 0;

	/*! \brief Set listener transform.
	 */
	virtual void setListenerTransform(const Transform& listenerTransform) = 0;

	/*! \brief Get listener transform.
	 */
	virtual Transform getListenerTransform() const = 0;

	/*! \brief Update sound player.
	 */
	virtual void update(float dT) = 0;
};

	}
}

#endif	// traktor_sound_ISoundPlayer_H
