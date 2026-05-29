/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

class GraphControl;

/*! Graph layout operation.
 * \ingroup UI
 *
 * Strategy interface for an algorithm that rearranges the nodes (and
 * possibly groups) of a GraphControl. Apply via GraphControl::apply.
 */
class T_DLLCLASS IGraphLayoutOperation : public Object
{
	T_RTTI_CLASS;

public:
	virtual void apply(GraphControl* graph) const = 0;
};

}
