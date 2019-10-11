#pragma once

#include "Core/Thread/Result.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ai
	{

class MoveQuery;

class T_DLLCLASS MoveQueryResult : public Result
{
	T_RTTI_CLASS;

public:
	void succeed(MoveQuery* moveQuery);

	MoveQuery* get() const;

private:
	Ref< MoveQuery > m_moveQuery;
};

	}
}

