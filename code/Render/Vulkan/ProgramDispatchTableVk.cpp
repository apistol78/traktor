/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Vulkan/ProgramVk.h"
#include "Render/Vulkan/ProgramDispatchTableVk.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ProgramDispatchTableVk", ProgramDispatchTableVk, IProgramDispatchTable)

int32_t ProgramDispatchTableVk::addProgram(IProgram* program)
{
	ProgramVk* pvk = mandatory_non_null_type_cast< ProgramVk* >(program);
	m_programs.push_back(pvk);
	m_hash += pvk->getShaderHash();	//#fixme Should be noncommutative.
	return (int32_t)m_programs.size() - 1;
}

}
