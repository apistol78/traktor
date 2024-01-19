/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Sound/IAudioFilter.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::sound
{

/*! Group filter.
 * \ingroup Sound
 */
class T_DLLCLASS GroupFilter : public IAudioFilter
{
	T_RTTI_CLASS;

public:
	GroupFilter() = default;

	explicit GroupFilter(IAudioFilter* filter1);

	explicit GroupFilter(IAudioFilter* filter1, IAudioFilter* filter2);

	explicit GroupFilter(IAudioFilter* filter1, IAudioFilter* filter2, IAudioFilter* filter3);

	void addFilter(IAudioFilter* filter);

	virtual Ref< IAudioFilterInstance > createInstance() const override final;

	virtual void apply(IAudioFilterInstance* instance, AudioBlock& outBlock) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	RefArray< IAudioFilter > m_filters;
};

}
