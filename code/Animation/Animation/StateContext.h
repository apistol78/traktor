#pragma once

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace animation
	{

/*! State evaluation context.
 * \ingroup Animation
 */
class T_DLLCLASS StateContext : public Object
{
	T_RTTI_CLASS;

public:
	void setTime(float time) { m_time = time; }

	float getTime() const { return m_time; }

	void setDuration(float duration) { m_duration = duration; }

	float getDuration() const { return m_duration; }

	void setIndexHint(int32_t indexHint) { m_indexHint = indexHint; }

	int32_t getIndexHint() const { return m_indexHint; }

private:
	float m_time = 0.0f;
	float m_duration = 0.0f;
	int32_t m_indexHint = -1;
};

	}
}

