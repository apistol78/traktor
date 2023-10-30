/*
 * TRAKTOR
 * Copyright (c) 2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Spark/MovieResult.h"

namespace traktor::spark
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.MovieResult", MovieResult, Result)

void MovieResult::succeed(Movie* movie)
{
	m_movie = movie;
	Result::succeed();
}

Movie* MovieResult::get()
{
	wait();
	return m_movie;
}

}
