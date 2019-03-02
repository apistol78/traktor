#pragma once

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

class Sound;

/*! \brief Sound rendering interface.
 * \ingroup Flash
 */
class T_DLLCLASS ISoundRenderer : public Object
{
	T_RTTI_CLASS;

public:
	virtual void destroy() = 0;

	virtual void play(const Sound* sound) = 0;
};

	}
}

