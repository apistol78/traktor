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
#include "Core/Ref.h"

namespace traktor::jungle
{

class IValueTemplate;
class State;

class T_DLLCLASS StateTemplate : public Object
{
	T_RTTI_CLASS;

public:
	void declare(const IValueTemplate* value);

	bool match(const State* S) const;

	bool critical(const State* Sn1, const State* S0) const;

	Ref< const State > extrapolate(const State* Sn2, float Tn2, const State* Sn1, float Tn1, const State* S0, float T0, float T) const;

	uint32_t pack(const State* S, void* buffer, uint32_t bufferSize) const;

	Ref< const State > unpack(const void* buffer, uint32_t bufferSize) const;

private:
	RefArray< const IValueTemplate > m_valueTemplates;
};

}
