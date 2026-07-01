/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Class/IRuntimeDispatch.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! Polymorphic dispatch
 * \ingroup Core
 */
class T_DLLCLASS PolymorphicDispatch : public IRuntimeDispatch
{
	T_RTTI_CLASS;

public:
	void set(uint32_t argc, const IRuntimeDispatch* dispatch);

#if defined(T_NEED_RUNTIME_SIGNATURE)
	virtual void signature(OutputStream& os) const override final;
#endif

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const override final;

private:
	AlignedVector< RefArray< const IRuntimeDispatch > > m_dispatches;
};

}

