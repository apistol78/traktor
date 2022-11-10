/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Reflection/ReflectionMemberPredicate.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! Member name query predicate.
 * \ingroup Core
 */
class T_DLLCLASS RfpMemberName : public ReflectionMemberPredicate
{
	T_RTTI_CLASS;

public:
	RfpMemberName(const std::wstring& memberName);

	virtual bool operator () (const ReflectionMember* member) const override final;

private:
	std::wstring m_memberName;
};

}

