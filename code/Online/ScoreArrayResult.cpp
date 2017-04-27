/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Online/ScoreArrayResult.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.ScoreArrayResult", ScoreArrayResult, Result)

void ScoreArrayResult::succeed(const RefArray< Score >& score)
{
	m_score = score;
	Result::succeed();
}

const RefArray< Score >& ScoreArrayResult::get() const
{
	wait();
	return m_score;
}

	}
}
