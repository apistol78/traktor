/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Containers/SmallMap.h"
#include "Ui/Point.h"
#include "Ui/Rect.h"
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

class Node;

/*! Auto-layout the graph (or the current selection if anything is selected).
 * \ingroup UI
 *
 * Arranges nodes into columns following the data flow (output → input),
 * trying to reduce edge crossings and edges passing behind nodes.
 * Groups become per-group islands, and disjoint sub-graphs are laid out
 * independently. Cyclic edges are tolerated as back-edges (the resulting
 * layout for cycles is approximate).
 *
 * If at least one node or group is selected, only the selection is
 * processed. With nothing selected the whole graph is laid out.
 */
class T_DLLCLASS PrettifyLayoutOperation : public IGraphLayoutOperation
{
	T_RTTI_CLASS;

public:
	virtual void apply(GraphControl* graph) const override;

private:
	UnitRect prettifyIsland(
		GraphControl* graph,
		const AlignedVector< int32_t >& nodeIndices,
		const UnitPoint& center,
		AlignedVector< UnitPoint >& outPositions) const;

	void partitionByConnectivity(
		GraphControl* graph,
		const AlignedVector< int32_t >& nodeIndices,
		const SmallMap< const Node*, int32_t >& globalIndex,
		AlignedVector< AlignedVector< int32_t > >& outComponents) const;
};

}
