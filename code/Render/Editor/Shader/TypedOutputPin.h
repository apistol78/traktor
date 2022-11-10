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
#include "Render/Types.h"
#include "Render/Editor/OutputPin.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

/*! Typed shader graph node output pin.
 * \ingroup Render
 */
class T_DLLCLASS TypedOutputPin : public OutputPin
{
public:
	TypedOutputPin() = default;

	TypedOutputPin(Node* node, const Guid& id, const std::wstring& name, ParameterType type);

	ParameterType getType() const;

private:
	ParameterType m_type = ParameterType::Scalar;
};

	}
}
