/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Timer/Timer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace net
	{

class BidirectionalObjectTransport;

	}

	namespace spark
	{

class Movie;
class SpriteInstance;

/*! \brief
 * \ingroup Spark
 */
class T_DLLCLASS MovieDebugger : public Object
{
	T_RTTI_CLASS;

public:
	MovieDebugger(net::BidirectionalObjectTransport* transport, const std::wstring& name);

	void postExecuteFrame(
		const Movie* movie,
		const SpriteInstance* movieInstance,
		const Vector4& stageTransform,
		int32_t viewWidth,
		int32_t viewHeight
	) const;

private:
	Ref< net::BidirectionalObjectTransport > m_transport;
	mutable int32_t m_captureFrames;
};

	}
}

