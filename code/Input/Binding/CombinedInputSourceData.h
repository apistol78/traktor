/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Input/InputTypes.h"
#include "Input/Binding/CombinedInputSource.h"
#include "Input/Binding/IInputSourceData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::input
{

/*! Boolean combination of multiple input sources.
 * \ingroup Input
 */
class T_DLLCLASS CombinedInputSourceData : public IInputSourceData
{
	T_RTTI_CLASS;

public:
	CombinedInputSourceData();

	explicit CombinedInputSourceData(CombinedInputSource::CombineMode mode);

	explicit CombinedInputSourceData(const RefArray< IInputSourceData >& sources, CombinedInputSource::CombineMode mode);

	void addSource(IInputSourceData* source);

	const RefArray< IInputSourceData >& getSources() const;

	virtual Ref< IInputSource > createInstance(DeviceControlManager* deviceControlManager) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	RefArray< IInputSourceData > m_sources;
	CombinedInputSource::CombineMode m_mode;
};

}
