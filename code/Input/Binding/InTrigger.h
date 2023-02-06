/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Input/Binding/IInputNode.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::input
{

/*!
 * \ingroup Input
 */
class T_DLLCLASS InTrigger : public IInputNode
{
	T_RTTI_CLASS;

public:
	enum Flank
	{
		FlPositive,
		FlNegative
	};

	InTrigger();

	explicit InTrigger(IInputNode* source, Flank flank, float duration);

	virtual Ref< Instance > createInstance() const override final;

	virtual float evaluate(
		Instance* instance,
		const InputValueSet& valueSet,
		float T,
		float dT
	) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	friend class InTriggerTraits;

	Ref< IInputNode > m_source;
	Flank m_flank;
	float m_duration;
};

}
