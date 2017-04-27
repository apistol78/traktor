/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_LimitScope_H
#define traktor_LimitScope_H

#include "Core/Timer/Timer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief Time limit scope.
 * \ingroup Core
 *
 * Timed limit debugging aid; will
 * spew into the debug console whenever
 * the time limit has been exceeded.
 *
 * As this tool measure time it's
 * most suitable for homogenous hardware
 * such as consoles or phones.
 */
class T_DLLCLASS LimitScope : public Object
{
	T_RTTI_CLASS;

public:
	struct Context
	{
		int32_t m_count;
		float m_peek;

		Context()
		:	m_count(0)
		,	m_peek(0.0f)
		{
		}
	};

	LimitScope(const char* name, float threshold, Context& context);

	virtual ~LimitScope();

private:
	const char* m_name;
	float m_threshold;
	Context& m_context;
	Timer m_timer;
};

#if defined(T_LIMIT_SCOPE_ENABLE)
#	define T_LIMIT_SCOPE(threshold) \
	static LimitScope::Context s_limitScopeContext; \
	LimitScope __limitScope__(T_FILE_LINE, threshold, s_limitScopeContext);
#endif

#if !defined(T_LIMIT_SCOPE)
#	define T_LIMIT_SCOPE(threshold)
#endif

}

#endif	// traktor_LimitScope_H
