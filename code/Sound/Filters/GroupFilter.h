/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Sound/IFilter.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

/*! Group filter.
 * \ingroup Sound
 */
class T_DLLCLASS GroupFilter : public IFilter
{
	T_RTTI_CLASS;

public:
	GroupFilter();

	GroupFilter(IFilter* filter1);

	GroupFilter(IFilter* filter1, IFilter* filter2);

	GroupFilter(IFilter* filter1, IFilter* filter2, IFilter* filter3);

	void addFilter(IFilter* filter);

	virtual Ref< IFilterInstance > createInstance() const override final;

	virtual void apply(IFilterInstance* instance, SoundBlock& outBlock) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	RefArray< IFilter > m_filters;
};

	}
}

