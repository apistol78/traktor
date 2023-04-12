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

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Attribute;

/*! Reflected member base class.
 * \ingroup Core
 */
class T_DLLCLASS ReflectionMember : public Object
{
	T_RTTI_CLASS;

public:
	const wchar_t* getName() const;

	const Attribute* getAttributes() const;

	virtual bool replace(const ReflectionMember* source) = 0;

protected:
	ReflectionMember(const wchar_t* name, const Attribute* attributes);

private:
	const wchar_t* m_name;
	Ref< const Attribute > m_attributes;
};

}

