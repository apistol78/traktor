#pragma once

#include "Core/RefArray.h"
#include "Core/Thread/Result.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ONLINE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace online
	{

class Score;

class T_DLLCLASS ScoreArrayResult : public Result
{
	T_RTTI_CLASS;

public:
	void succeed(const RefArray< Score >& score);

	const RefArray< Score >& get() const;

private:
	RefArray< Score > m_score;
};

	}
}

