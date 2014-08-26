#ifndef traktor_animation_StateContext_H
#define traktor_animation_StateContext_H

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

/*! \brief State evaluation context.
 * \ingroup Animation
 */
class T_DLLCLASS StateContext : public Object
{
	T_RTTI_CLASS;

public:
	StateContext();

	void setTime(float time) { m_time = time; }

	float getTime() const { return m_time; }

	void setDuration(float duration) { m_duration = duration; }

	float getDuration() const { return m_duration; }

	void setIndexHint(int32_t indexHint) { m_indexHint = indexHint; }

	int32_t getIndexHint() const { return m_indexHint; }

private:
	float m_time;
	float m_duration;
	int32_t m_indexHint;
};

	}
}

#endif	// traktor_animation_StateContext_H
