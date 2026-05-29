/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/Graph/IGraphLayoutOperation.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

/*! Align selected nodes to a common edge.
 * \ingroup UI
 */
class T_DLLCLASS AlignNodesLayoutOperation : public IGraphLayoutOperation
{
	T_RTTI_CLASS;

public:
	enum Alignment
	{
		AnLeft,
		AnTop,
		AnRight,
		AnBottom
	};

	explicit AlignNodesLayoutOperation(Alignment alignment);

	virtual void apply(GraphControl* graph) const override;

private:
	Alignment m_alignment;
};

}
