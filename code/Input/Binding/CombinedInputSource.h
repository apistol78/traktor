/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <list>
#include "Core/RefArray.h"
#include "Input/Binding/IInputSource.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace input
	{

/*! Boolean combination of multiple input sources.
 * \ingroup Input
 */
class T_DLLCLASS CombinedInputSource : public IInputSource
{
	T_RTTI_CLASS;

public:
	enum CombineMode
	{
		CmAny,			//!< Any source is true.
		CmExclusive,	//!< Only one source is true.
		CmAll			//!< All sources must be true.
	};

	CombinedInputSource(const RefArray< IInputSource >& sources, CombineMode mode);

	virtual std::wstring getDescription() const override final;

	virtual void prepare(float T, float dT) override final;

	virtual float read(float T, float dT) override final;

private:
	RefArray< IInputSource > m_sources;
	CombineMode m_mode;
};

	}
}

