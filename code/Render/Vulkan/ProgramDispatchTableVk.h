/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Render/IProgramDispatchTable.h"

namespace traktor::render
{

class ProgramVk;

class ProgramDispatchTableVk : public IProgramDispatchTable
{
	T_RTTI_CLASS;

public:
	virtual int32_t addProgram(IProgram* program) override final;

	const RefArray< ProgramVk >& getPrograms() const { return m_programs; }

private:
	RefArray< ProgramVk > m_programs;
};

}
