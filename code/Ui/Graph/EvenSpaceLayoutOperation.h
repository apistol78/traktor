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

/*! Distribute selected nodes evenly along an axis.
 * \ingroup UI
 */
class T_DLLCLASS EvenSpaceLayoutOperation : public IGraphLayoutOperation
{
	T_RTTI_CLASS;

public:
	enum EvenSpace
	{
		EsHorizontally,
		EsVertically
	};

	explicit EvenSpaceLayoutOperation(EvenSpace space);

	virtual void apply(GraphControl* graph) const override;

private:
	EvenSpace m_space;
};

}
