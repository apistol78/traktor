/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Graph/PrettifyLayoutOperation.h"

#include "Ui/Graph/Edge.h"
#include "Ui/Graph/GraphControl.h"
#include "Ui/Graph/Group.h"
#include "Ui/Graph/GroupMovedEvent.h"
#include "Ui/Graph/Node.h"
#include "Ui/Graph/NodeMovedEvent.h"
#include "Ui/Graph/Pin.h"

#include <algorithm>
#include <limits>
#include <utility>

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.PrettifyLayoutOperation", PrettifyLayoutOperation, IGraphLayoutOperation);

UnitRect PrettifyLayoutOperation::prettifyIsland(
	GraphControl* graph,
	const AlignedVector< int32_t >& nodeIndices,
	const UnitPoint& center,
	AlignedVector< UnitPoint >& outPositions) const
{
	const auto& gNodes = graph->getNodes();
	const auto& gEdges = graph->getEdges();

	const int32_t nReal = (int32_t)nodeIndices.size();
	outPositions.resize(0);
	outPositions.resize((size_t)nReal, UnitPoint(0_ut, 0_ut));
	if (nReal == 0)
		return UnitRect();

	// Map Node* → local index (only members of this island).
	SmallMap< const Node*, int32_t > localIndex;
	for (int32_t i = 0; i < nReal; ++i)
		localIndex.insert(gNodes[nodeIndices[i]], i);

	// Build adjacency restricted to edges between members of the island.
	AlignedVector< AlignedVector< int32_t > > outAdj(nReal);
	AlignedVector< AlignedVector< int32_t > > inAdj(nReal);
	for (auto edge : gEdges)
	{
		const Pin* src = edge->getSourcePin();
		const Pin* dst = edge->getDestinationPin();
		if (!src || !dst)
			continue;
		auto si = localIndex.find(src->getNode());
		auto di = localIndex.find(dst->getNode());
		if (si == localIndex.end() || di == localIndex.end())
			continue;
		if (si->second == di->second)
			continue;
		outAdj[si->second].push_back(di->second);
		inAdj[di->second].push_back(si->second);
	}

	// Iterative DFS topological order. Back-edges (cycles) end up
	// pointing "backwards" in the order and are ignored when layering,
	// so cyclic graphs degrade gracefully instead of hanging.
	AlignedVector< int32_t > order;
	AlignedVector< int8_t > color((size_t)nReal, (int8_t)0);
	AlignedVector< int32_t > stack;
	AlignedVector< int32_t > iter((size_t)nReal, 0);
	order.reserve(nReal);

	for (int32_t start = 0; start < nReal; ++start)
	{
		if (color[start] != 0)
			continue;
		stack.push_back(start);
		color[start] = 1;
		while (!stack.empty())
		{
			const int32_t u = stack.back();
			if (iter[u] < (int32_t)outAdj[u].size())
			{
				const int32_t v = outAdj[u][iter[u]++];
				if (color[v] == 0)
				{
					color[v] = 1;
					stack.push_back(v);
				}
			}
			else
			{
				color[u] = 2;
				order.push_back(u);
				stack.pop_back();
			}
		}
	}
	std::reverse(order.begin(), order.end());

	AlignedVector< int32_t > rank((size_t)nReal, 0);
	for (int32_t i = 0; i < nReal; ++i)
		rank[order[i]] = i;

	// Longest-path layer assignment, forward edges only. This sets each
	// node to its earliest valid layer, which often leaves sources
	// stranded at layer 0 even when their only successor is far to the
	// right.
	AlignedVector< int32_t > layer((size_t)nReal, 0);
	for (int32_t u : order)
	{
		for (int32_t v : outAdj[u])
		{
			if (rank[u] < rank[v])
				layer[v] = std::max(layer[v], layer[u] + 1);
		}
	}

	// Pull each non-sink node toward its successors: layer[u] becomes the
	// latest layer still satisfying layer[u] < layer[v] for every forward
	// successor v. Processed in reverse topological order so each node
	// sees its successors at their final layers. Monotonically non-
	// decreasing per node, so forward-edge order is always preserved and
	// no edge gets longer than longest-path would have produced.
	for (auto rit = order.rbegin(); rit != order.rend(); ++rit)
	{
		const int32_t u = *rit;
		int32_t hi = std::numeric_limits< int32_t >::max();
		for (int32_t v : outAdj[u])
		{
			if (rank[u] < rank[v])
				hi = std::min(hi, layer[v] - 1);
		}
		if (hi != std::numeric_limits< int32_t >::max() && hi > layer[u])
			layer[u] = hi;
	}

	// Promotion may have left empty layers (e.g. nothing remains at
	// layer 0 if every former layer-0 source moved right). Compact so
	// adjacent layer indices correspond to adjacent columns; otherwise
	// long edges would still need dummies just to skip the empty slots.
	{
		int32_t maxL = 0;
		for (int32_t L : layer)
			maxL = std::max(maxL, L);
		AlignedVector< uint8_t > used((size_t)(maxL + 1), (uint8_t)0);
		for (int32_t L : layer)
			used[L] = 1;
		AlignedVector< int32_t > remap((size_t)(maxL + 1), -1);
		int32_t cur = 0;
		for (int32_t L = 0; L <= maxL; ++L)
			if (used[L])
				remap[L] = cur++;
		for (int32_t i = 0; i < nReal; ++i)
			layer[i] = remap[layer[i]];
	}

	int32_t nLayers = 1;
	for (int32_t L : layer)
		nLayers = std::max(nLayers, L + 1);

	// Insert dummy nodes along forward edges that span more than one layer.
	// Dummies participate in barycenter ordering exactly like real nodes,
	// which is what causes neighboring columns to shift so long edges have
	// a clear vertical lane to pass through.
	AlignedVector< AlignedVector< int32_t > > xOutAdj(nReal);
	AlignedVector< AlignedVector< int32_t > > xInAdj(nReal);
	AlignedVector< int32_t > xLayer = layer;
	int32_t nTotal = nReal;

	for (int32_t u = 0; u < nReal; ++u)
	{
		for (int32_t v : outAdj[u])
		{
			if (rank[u] >= rank[v])
				continue; // skip back-edges
			if (layer[v] - layer[u] == 1)
			{
				xOutAdj[u].push_back(v);
				xInAdj[v].push_back(u);
			}
			else
			{
				int32_t prev = u;
				for (int32_t L = layer[u] + 1; L < layer[v]; ++L)
				{
					const int32_t d = nTotal++;
					xOutAdj.push_back(AlignedVector< int32_t >());
					xInAdj.push_back(AlignedVector< int32_t >());
					xLayer.push_back(L);
					xOutAdj[prev].push_back(d);
					xInAdj[d].push_back(prev);
					prev = d;
				}
				xOutAdj[prev].push_back(v);
				xInAdj[v].push_back(prev);
			}
		}
	}

	AlignedVector< AlignedVector< int32_t > > layerNodes(nLayers);
	for (int32_t i = 0; i < nTotal; ++i)
		layerNodes[xLayer[i]].push_back(i);

	// Seed within-layer order: real nodes by current Y, dummies by insertion order.
	for (int32_t L = 0; L < nLayers; ++L)
	{
		std::stable_sort(layerNodes[L].begin(), layerNodes[L].end(), [&](int32_t a, int32_t b) {
			const bool aReal = a < nReal;
			const bool bReal = b < nReal;
			if (aReal && bReal)
				return gNodes[nodeIndices[a]]->getPosition().y < gNodes[nodeIndices[b]]->getPosition().y;
			return false;
		});
	}

	AlignedVector< int32_t > position((size_t)nTotal, 0);
	for (int32_t L = 0; L < nLayers; ++L)
		for (int32_t i = 0; i < (int32_t)layerNodes[L].size(); ++i)
			position[layerNodes[L][i]] = i;

	auto sortLayer = [&](int32_t L, bool useIn) {
		auto& nodes = layerNodes[L];
		if (nodes.size() < 2)
			return;
		const int32_t targetLayer = useIn ? L - 1 : L + 1;
		AlignedVector< float > bary(nodes.size());
		for (size_t i = 0; i < nodes.size(); ++i)
		{
			const auto& adj = useIn ? xInAdj[nodes[i]] : xOutAdj[nodes[i]];
			int32_t cnt = 0;
			float sum = 0.0f;
			for (int32_t v : adj)
			{
				if (xLayer[v] == targetLayer)
				{
					sum += (float)position[v];
					cnt++;
				}
			}
			bary[i] = cnt > 0 ? sum / (float)cnt : (float)position[nodes[i]];
		}
		AlignedVector< size_t > idx(nodes.size());
		for (size_t i = 0; i < idx.size(); ++i)
			idx[i] = i;
		std::stable_sort(idx.begin(), idx.end(), [&](size_t a, size_t b) {
			return bary[a] < bary[b];
		});
		AlignedVector< int32_t > newNodes(nodes.size());
		for (size_t i = 0; i < idx.size(); ++i)
			newNodes[i] = nodes[idx[i]];
		nodes = std::move(newNodes);
		for (int32_t i = 0; i < (int32_t)nodes.size(); ++i)
			position[nodes[i]] = i;
	};

	const int32_t sweeps = 24;
	for (int32_t s = 0; s < sweeps; ++s)
	{
		if ((s & 1) == 0)
		{
			for (int32_t L = 1; L < nLayers; ++L)
				sortLayer(L, true);
		}
		else
		{
			for (int32_t L = nLayers - 2; L >= 0; --L)
				sortLayer(L, false);
		}
	}

	// Adjacent-transposition cleanup. The barycenter heuristic is
	// well-known to get stuck in local minima where two neighboring
	// nodes would have fewer crossings if swapped. This pass walks each
	// layer's adjacent pairs and swaps them when doing so strictly
	// reduces the total crossings on edges incident to that pair —
	// counting downstream (toward L+1) and upstream (toward L-1) edges
	// together. Only swaps that improve are accepted, so the total
	// crossing count is monotonically non-increasing.
	auto pairCrossings = [&](const AlignedVector< int32_t >& adjA, const AlignedVector< int32_t >& adjB) -> std::pair< int32_t, int32_t > {
		int32_t crossAB = 0;
		int32_t crossBA = 0;
		for (int32_t va : adjA)
		{
			const int32_t pva = position[va];
			for (int32_t vb : adjB)
			{
				const int32_t pvb = position[vb];
				if (pva > pvb)
					++crossAB;
				else if (pva < pvb)
					++crossBA;
			}
		}
		return { crossAB, crossBA };
	};

	const int32_t maxTransIter = 16;
	for (int32_t iter = 0; iter < maxTransIter; ++iter)
	{
		bool improved = false;
		// Alternate sweep direction across iterations so neither end of
		// the column has a structural advantage.
		const bool reverse = (iter & 1) != 0;
		for (int32_t lIdx = 0; lIdx < nLayers; ++lIdx)
		{
			const int32_t L = reverse ? nLayers - 1 - lIdx : lIdx;
			auto& nodes = layerNodes[L];
			const int32_t sz = (int32_t)nodes.size();
			if (sz < 2)
				continue;
			for (int32_t i = 0; i + 1 < sz; ++i)
			{
				const int32_t a = nodes[i];
				const int32_t b = nodes[i + 1];
				int32_t cur = 0;
				int32_t swp = 0;
				const auto dn = pairCrossings(xOutAdj[a], xOutAdj[b]);
				cur += dn.first;
				swp += dn.second;
				const auto up = pairCrossings(xInAdj[a], xInAdj[b]);
				cur += up.first;
				swp += up.second;
				if (swp < cur)
				{
					nodes[i] = b;
					nodes[i + 1] = a;
					position[a] = i + 1;
					position[b] = i;
					improved = true;
				}
			}
		}
		if (!improved)
			break;
	}

	const Unit columnGap = 90_ut;
	const Unit rowGap = 32_ut;
	const Unit dummyHeight = rowGap; // vertical slot reserved per long-edge crossing

	// Column widths from real nodes only (dummies are zero-width).
	AlignedVector< Unit > colWidth(nLayers, 0_ut);
	for (int32_t L = 0; L < nLayers; ++L)
		for (int32_t u : layerNodes[L])
			if (u < nReal)
				colWidth[L] = std::max(colWidth[L], gNodes[nodeIndices[u]]->calculateRect().getWidth());

	AlignedVector< Unit > colX(nLayers, 0_ut);
	Unit totalW = 0_ut;
	for (int32_t L = 0; L < nLayers; ++L)
	{
		colX[L] = totalW;
		totalW += colWidth[L];
		if (L + 1 < nLayers)
			totalW += columnGap;
	}
	const Unit originX = center.x - totalW / 2_ut;

	auto itemHeight = [&](int32_t u) -> Unit {
		return u < nReal ? gNodes[nodeIndices[u]]->calculateRect().getHeight() : dummyHeight;
	};

	// Seed initial Y for every item (real nodes and dummies) by stacking
	// the column around `center.y`.
	AlignedVector< Unit > itemY((size_t)nTotal, 0_ut);
	for (int32_t L = 0; L < nLayers; ++L)
	{
		const auto& nodes = layerNodes[L];
		Unit totalH = 0_ut;
		for (size_t i = 0; i < nodes.size(); ++i)
		{
			totalH += itemHeight(nodes[i]);
			if (i + 1 < nodes.size())
				totalH += rowGap;
		}
		Unit y = center.y - totalH / 2_ut;
		for (int32_t u : nodes)
		{
			itemY[u] = y;
			y += itemHeight(u) + rowGap;
		}
	}

	// Compaction pass: pull each item toward the vertical center of its
	// connected neighbors (upstream + downstream), constrained by its
	// same-column siblings so within-column order is preserved and no
	// node overlaps another. Dummies on long edges get pulled toward a
	// straight line between their endpoints, which also keeps long edges
	// from drifting across nodes. Each item considers neighbors in both
	// directions, so direct edges become close to horizontal.
	auto centerYOf = [&](int32_t u) -> Unit {
		return itemY[u] + itemHeight(u) / 2_ut;
	};

	const int32_t compactionPasses = 16;
	for (int32_t pass = 0; pass < compactionPasses; ++pass)
	{
		const bool reverse = (pass & 1) != 0;
		for (int32_t lIdx = 0; lIdx < nLayers; ++lIdx)
		{
			const int32_t L = reverse ? nLayers - 1 - lIdx : lIdx;
			const auto& nodes = layerNodes[L];
			const int32_t sz = (int32_t)nodes.size();
			for (int32_t iIdx = 0; iIdx < sz; ++iIdx)
			{
				const int32_t i = reverse ? sz - 1 - iIdx : iIdx;
				const int32_t u = nodes[i];

				int32_t cnt = 0;
				int64_t sum = 0;
				for (int32_t v : xInAdj[u])
				{
					sum += (int64_t)centerYOf(v).get();
					++cnt;
				}
				for (int32_t v : xOutAdj[u])
				{
					sum += (int64_t)centerYOf(v).get();
					++cnt;
				}
				if (cnt == 0)
					continue;

				const Unit targetCenter((int32_t)(sum / cnt));
				Unit newY = targetCenter - itemHeight(u) / 2_ut;

				// Same-column sibling constraints. Note minY <= maxY is
				// guaranteed because each prior update preserved the
				// rowGap separation with its neighbors.
				if (i > 0)
				{
					const int32_t prev = nodes[i - 1];
					const Unit minY = itemY[prev] + itemHeight(prev) + rowGap;
					if (newY < minY)
						newY = minY;
				}
				if (i + 1 < sz)
				{
					const int32_t next = nodes[i + 1];
					const Unit maxY = itemY[next] - itemHeight(u) - rowGap;
					if (newY > maxY)
						newY = maxY;
				}

				itemY[u] = newY;
			}
		}
	}

	// Strict pin-aware alignment. The compaction above lines up nodes by
	// their geometric centers; this stricter pass refines by aligning
	// each real node's *input pin* against the *output pin* on the
	// upstream node that drives it — but only for edges that come from
	// the immediately preceding column (one layer back). Long edges go
	// through dummies and were already handled by the compaction.
	//
	// Because the pass only mutates itemY (never the within-column order
	// in layerNodes/position) it cannot change cross-column edge
	// crossings. A defensive crossing count before/after enforces that
	// invariant: if the count ever increased, the itemY snapshot is
	// restored.
	{
		struct PinIncoming
		{
			int32_t src;     // local index of upstream node
			Unit pinDelta;   // srcPinOffset - dstPinOffset (constant per edge)
		};
		AlignedVector< AlignedVector< PinIncoming > > pinIn((size_t)nReal);

		for (auto edge : gEdges)
		{
			const Pin* sp = edge->getSourcePin();
			const Pin* dp = edge->getDestinationPin();
			if (!sp || !dp)
				continue;
			Node* sn = sp->getNode();
			Node* dn = dp->getNode();
			if (!sn || !dn)
				continue;
			auto si = localIndex.find(sn);
			auto di = localIndex.find(dn);
			if (si == localIndex.end() || di == localIndex.end())
				continue;
			const int32_t u_local = si->second;
			const int32_t v_local = di->second;
			if (u_local == v_local)
				continue;
			// Only edges crossing exactly one column boundary participate
			// in pin alignment. Back-edges and long edges are skipped.
			if (layer[u_local] + 1 != layer[v_local])
				continue;
			const Unit srcPinOffset = sp->getPosition().y - sn->getPosition().y;
			const Unit dstPinOffset = dp->getPosition().y - dn->getPosition().y;
			pinIn[v_local].push_back({ u_local, srcPinOffset - dstPinOffset });
		}

		// Count edge crossings between every adjacent column pair using
		// current position[] indices. Cheap enough at this scale; we run
		// it twice (before and after alignment) only as a safety net.
		auto totalCrossings = [&]() -> int64_t {
			int64_t total = 0;
			for (int32_t L = 0; L + 1 < nLayers; ++L)
			{
				AlignedVector< std::pair< int32_t, int32_t > > es;
				for (int32_t u : layerNodes[L])
				{
					for (int32_t v : xOutAdj[u])
					{
						if (xLayer[v] == L + 1)
							es.push_back({ position[u], position[v] });
					}
				}
				const size_t n = es.size();
				for (size_t i = 0; i < n; ++i)
				{
					for (size_t j = i + 1; j < n; ++j)
					{
						const auto& a = es[i];
						const auto& b = es[j];
						if ((a.first < b.first && a.second > b.second) ||
							(a.first > b.first && a.second < b.second))
							++total;
					}
				}
			}
			return total;
		};

		const int64_t crossingsBefore = totalCrossings();
		AlignedVector< Unit > savedItemY = itemY;

		const int32_t pinPasses = 8;
		for (int32_t pass = 0; pass < pinPasses; ++pass)
		{
			const bool reverse = (pass & 1) != 0;
			for (int32_t lIdx = 0; lIdx < nLayers; ++lIdx)
			{
				const int32_t L = reverse ? nLayers - 1 - lIdx : lIdx;
				const auto& nodes = layerNodes[L];
				const int32_t sz = (int32_t)nodes.size();
				for (int32_t iIdx = 0; iIdx < sz; ++iIdx)
				{
					const int32_t i = reverse ? sz - 1 - iIdx : iIdx;
					const int32_t u = nodes[i];
					if (u >= nReal)
						continue;
					const auto& edges = pinIn[u];
					if (edges.empty())
						continue;

					int64_t sum = 0;
					for (const auto& e : edges)
						sum += (int64_t)itemY[e.src].get() + (int64_t)e.pinDelta.get();
					const Unit desired((int32_t)(sum / (int32_t)edges.size()));
					Unit newY = desired;

					if (i > 0)
					{
						const int32_t prev = nodes[i - 1];
						const Unit minY = itemY[prev] + itemHeight(prev) + rowGap;
						if (newY < minY)
							newY = minY;
					}
					if (i + 1 < sz)
					{
						const int32_t next = nodes[i + 1];
						const Unit maxY = itemY[next] - itemHeight(u) - rowGap;
						if (newY > maxY)
							newY = maxY;
					}

					itemY[u] = newY;
				}
			}
		}

		if (totalCrossings() > crossingsBefore)
			itemY = std::move(savedItemY);
	}

	// Commit real-node positions and compute the content bounds.
	UnitRect placedBounds(
		Unit(std::numeric_limits< int32_t >::max()),
		Unit(std::numeric_limits< int32_t >::max()),
		Unit(-std::numeric_limits< int32_t >::max()),
		Unit(-std::numeric_limits< int32_t >::max()));
	bool anyPlaced = false;

	for (int32_t L = 0; L < nLayers; ++L)
	{
		for (int32_t u : layerNodes[L])
		{
			if (u >= nReal)
				continue;
			const int32_t gi = nodeIndices[u];
			const UnitRect rc = gNodes[gi]->calculateRect();
			const UnitPoint pt(
				originX + colX[L] + (colWidth[L] - rc.getWidth()) / 2_ut,
				itemY[u]);
			outPositions[u] = pt;
			placedBounds.left = std::min(placedBounds.left, pt.x);
			placedBounds.top = std::min(placedBounds.top, pt.y);
			placedBounds.right = std::max(placedBounds.right, pt.x + rc.getWidth());
			placedBounds.bottom = std::max(placedBounds.bottom, pt.y + rc.getHeight());
			anyPlaced = true;
		}
	}

	return anyPlaced ? placedBounds : UnitRect();
}

void PrettifyLayoutOperation::partitionByConnectivity(
	GraphControl* graph,
	const AlignedVector< int32_t >& nodeIndices,
	const SmallMap< const Node*, int32_t >& globalIndex,
	AlignedVector< AlignedVector< int32_t > >& outComponents) const
{
	const auto& gNodes = graph->getNodes();
	const auto& gEdges = graph->getEdges();
	const int32_t nNodes = (int32_t)gNodes.size();

	AlignedVector< uint8_t > isMember((size_t)nNodes, (uint8_t)0);
	for (int32_t i : nodeIndices)
		isMember[i] = 1;

	// Undirected adjacency restricted to member-member edges. Edges with
	// one endpoint outside the input set are ignored — they would just
	// reach beyond the partition we're computing.
	AlignedVector< AlignedVector< int32_t > > adj((size_t)nNodes);
	for (auto edge : gEdges)
	{
		const Pin* src = edge->getSourcePin();
		const Pin* dst = edge->getDestinationPin();
		if (!src || !dst)
			continue;
		auto si = globalIndex.find(src->getNode());
		auto di = globalIndex.find(dst->getNode());
		if (si == globalIndex.end() || di == globalIndex.end())
			continue;
		const int32_t a = si->second;
		const int32_t b = di->second;
		if (a == b)
			continue;
		if (!isMember[a] || !isMember[b])
			continue;
		adj[a].push_back(b);
		adj[b].push_back(a);
	}

	// Iterative DFS. Each unvisited member seeds a new component;
	// isolated nodes form components of size 1.
	AlignedVector< uint8_t > visited((size_t)nNodes, (uint8_t)0);
	AlignedVector< int32_t > stack;
	outComponents.resize(0);
	for (int32_t start : nodeIndices)
	{
		if (visited[start])
			continue;
		AlignedVector< int32_t > component;
		stack.push_back(start);
		visited[start] = 1;
		while (!stack.empty())
		{
			const int32_t u = stack.back();
			stack.pop_back();
			component.push_back(u);
			for (int32_t v : adj[u])
			{
				if (!visited[v])
				{
					visited[v] = 1;
					stack.push_back(v);
				}
			}
		}
		outComponents.push_back(std::move(component));
	}
}

void PrettifyLayoutOperation::apply(GraphControl* graph) const
{
	const auto& gNodes = graph->getNodes();
	const auto& gGroups = graph->getGroups();
	const int32_t nNodes = (int32_t)gNodes.size();
	if (nNodes == 0)
		return;

	// Selection-aware scope. If anything is selected (a group or a node)
	// we restrict the operation to the selection — selected groups are
	// laid out (claiming every node they spatially contain, regardless
	// of that node's individual selection state), and any remaining
	// selected nodes are partitioned and laid out as free components.
	// With no selection anywhere, the whole graph is processed.
	bool selectionMode = false;
	for (auto group : gGroups)
	{
		if (group->isSelected())
		{
			selectionMode = true;
			break;
		}
	}
	if (!selectionMode)
	{
		for (auto node : gNodes)
		{
			if (node->isSelected())
			{
				selectionMode = true;
				break;
			}
		}
	}

	const Unit groupPadding = 64_ut;
	const Unit groupPaddingTop = 96_ut;

	// Each island is laid out independently. After planning the contents
	// of every island we resolve overlaps between island outer rects, so
	// the final commit doesn't put two groups (or a free cluster and a
	// group) on top of each other.
	struct Island
	{
		AlignedVector< int32_t > nodeIndices;
		AlignedVector< UnitPoint > positions; // planned positions, one per real node
		UnitRect contentBounds;               // planned content bounding rect
		UnitRect outerRect;                   // collision rect (content + padding for groups)
		Group* group;                         // null for the free island
		UnitSize delta;                       // accumulated shift from overlap resolution
	};
	AlignedVector< Island > islands;

	AlignedVector< uint8_t > assigned((size_t)nNodes, (uint8_t)0);

	// Global Node* → index lookup, built once and reused for connectivity
	// partitioning of both grouped and free nodes.
	SmallMap< const Node*, int32_t > globalIndex;
	for (int32_t i = 0; i < nNodes; ++i)
		globalIndex.insert(gNodes[i], i);

	// Distance enforced between sub-islands within the same group, to
	// keep disjoint clusters visually separated inside the group's frame.
	const Unit subIslandGap = 32_ut;

	// Per-group islands. First-match wins so a node intersecting multiple
	// groups lands in exactly one island. Within each group the claimed
	// nodes are further partitioned by edge connectivity, so disjoint
	// sub-graphs inside the same group are laid out independently.
	for (auto group : gGroups)
	{
		if (selectionMode && !group->isSelected())
			continue;
		const UnitRect groupRect = group->calculateRect();
		AlignedVector< int32_t > groupNodes;
		for (int32_t i = 0; i < nNodes; ++i)
		{
			if (assigned[i])
				continue;
			const UnitRect rc = gNodes[i]->calculateRect();
			if (groupRect.intersect(rc))
			{
				groupNodes.push_back(i);
				assigned[i] = 1;
			}
		}
		if (groupNodes.empty())
			continue;

		// Split into connectivity components (edges between grouped nodes
		// only — edges leaving the group don't merge sub-islands).
		AlignedVector< AlignedVector< int32_t > > subComponents;
		partitionByConnectivity(graph, groupNodes, globalIndex, subComponents);

		const int32_t numSubs = (int32_t)subComponents.size();
		AlignedVector< AlignedVector< UnitPoint > > subPositions((size_t)numSubs);
		AlignedVector< UnitRect > subBounds((size_t)numSubs);
		AlignedVector< UnitSize > subDeltas((size_t)numSubs, UnitSize(0_ut, 0_ut));
		AlignedVector< uint8_t > subValid((size_t)numSubs, (uint8_t)0);

		for (int32_t s = 0; s < numSubs; ++s)
		{
			const auto& comp = subComponents[s];
			UnitRect b(
				Unit(std::numeric_limits< int32_t >::max()),
				Unit(std::numeric_limits< int32_t >::max()),
				Unit(-std::numeric_limits< int32_t >::max()),
				Unit(-std::numeric_limits< int32_t >::max()));
			for (int32_t i : comp)
			{
				const UnitRect rc = gNodes[i]->calculateRect();
				b.left = std::min(b.left, rc.left);
				b.right = std::max(b.right, rc.right);
				b.top = std::min(b.top, rc.top);
				b.bottom = std::max(b.bottom, rc.bottom);
			}
			subBounds[s] = prettifyIsland(graph, comp, b.getCenter(), subPositions[s]);
			subValid[s] = subBounds[s].getWidth() > 0_ut ? (uint8_t)1 : (uint8_t)0;
		}

		// Local repulsion: arrange sub-islands inside the group so they
		// don't overlap, leaving subIslandGap between them along at
		// least one axis.
		const int32_t maxLocalIter = 64;
		for (int32_t iter = 0; iter < maxLocalIter; ++iter)
		{
			bool anyOverlap = false;
			for (int32_t i = 0; i < numSubs; ++i)
			{
				if (!subValid[i])
					continue;
				for (int32_t j = i + 1; j < numSubs; ++j)
				{
					if (!subValid[j])
						continue;
					const UnitRect ri = subBounds[i].offset(subDeltas[i].cx, subDeltas[i].cy);
					const UnitRect rj = subBounds[j].offset(subDeltas[j].cx, subDeltas[j].cy);
					const Unit ovX = std::min(ri.right, rj.right) - std::max(ri.left, rj.left);
					const Unit ovY = std::min(ri.bottom, rj.bottom) - std::max(ri.top, rj.top);
					if (ovX <= -subIslandGap || ovY <= -subIslandGap)
						continue;

					anyOverlap = true;
					const Unit needX = ovX + subIslandGap;
					const Unit needY = ovY + subIslandGap;
					const UnitPoint ci = ri.getCenter();
					const UnitPoint cj = rj.getCenter();
					if (needX < needY)
					{
						const Unit push = needX / 2_ut + 1_ut;
						if (ci.x < cj.x)
						{
							subDeltas[i].cx -= push;
							subDeltas[j].cx += push;
						}
						else
						{
							subDeltas[i].cx += push;
							subDeltas[j].cx -= push;
						}
					}
					else
					{
						const Unit push = needY / 2_ut + 1_ut;
						if (ci.y < cj.y)
						{
							subDeltas[i].cy -= push;
							subDeltas[j].cy += push;
						}
						else
						{
							subDeltas[i].cy += push;
							subDeltas[j].cy -= push;
						}
					}
				}
			}
			if (!anyOverlap)
				break;
		}

		// Combine the sub-islands into the group's single global island.
		Island island;
		island.group = group;
		island.delta = UnitSize(0_ut, 0_ut);
		UnitRect unionBounds(
			Unit(std::numeric_limits< int32_t >::max()),
			Unit(std::numeric_limits< int32_t >::max()),
			Unit(-std::numeric_limits< int32_t >::max()),
			Unit(-std::numeric_limits< int32_t >::max()));
		bool anyValid = false;
		for (int32_t s = 0; s < numSubs; ++s)
		{
			if (!subValid[s])
				continue;
			const auto& comp = subComponents[s];
			const auto& positions = subPositions[s];
			for (size_t k = 0; k < comp.size(); ++k)
			{
				island.nodeIndices.push_back(comp[k]);
				island.positions.push_back(UnitPoint(
					positions[k].x + subDeltas[s].cx,
					positions[k].y + subDeltas[s].cy));
			}
			const UnitRect shifted = subBounds[s].offset(subDeltas[s].cx, subDeltas[s].cy);
			unionBounds.left = std::min(unionBounds.left, shifted.left);
			unionBounds.right = std::max(unionBounds.right, shifted.right);
			unionBounds.top = std::min(unionBounds.top, shifted.top);
			unionBounds.bottom = std::max(unionBounds.bottom, shifted.bottom);
			anyValid = true;
		}
		if (!anyValid)
			continue;

		island.contentBounds = unionBounds;
		island.outerRect = UnitRect(
			unionBounds.left - groupPadding,
			unionBounds.top - groupPaddingTop,
			unionBounds.right + groupPadding,
			unionBounds.bottom + groupPadding);
		islands.push_back(std::move(island));
	}

	// Free nodes (not in any group) are partitioned into connectivity
	// components — one global island per component.
	{
		AlignedVector< int32_t > freeNodes;
		for (int32_t i = 0; i < nNodes; ++i)
		{
			if (assigned[i])
				continue;
			if (selectionMode && !gNodes[i]->isSelected())
				continue;
			freeNodes.push_back(i);
		}

		if (!freeNodes.empty())
		{
			AlignedVector< AlignedVector< int32_t > > components;
			partitionByConnectivity(graph, freeNodes, globalIndex, components);

			for (auto& component : components)
			{
				UnitRect bounds(
					Unit(std::numeric_limits< int32_t >::max()),
					Unit(std::numeric_limits< int32_t >::max()),
					Unit(-std::numeric_limits< int32_t >::max()),
					Unit(-std::numeric_limits< int32_t >::max()));
				for (int32_t i : component)
				{
					const UnitRect rc = gNodes[i]->calculateRect();
					bounds.left = std::min(bounds.left, rc.left);
					bounds.right = std::max(bounds.right, rc.right);
					bounds.top = std::min(bounds.top, rc.top);
					bounds.bottom = std::max(bounds.bottom, rc.bottom);
				}

				Island island;
				island.group = nullptr;
				island.delta = UnitSize(0_ut, 0_ut);
				island.nodeIndices = std::move(component);
				island.contentBounds = prettifyIsland(graph, island.nodeIndices, bounds.getCenter(), island.positions);
				if (island.contentBounds.getWidth() > 0_ut)
				{
					island.outerRect = island.contentBounds;
					islands.push_back(std::move(island));
				}
			}
		}
	}

	// Iterative pairwise repulsion. Each iteration pushes overlapping
	// outer rects apart along the axis of smallest penetration, splitting
	// the shift evenly between the two islands. Converges in just a few
	// passes for typical graphs; capped at 64 to bound worst-case effort.
	const int32_t maxIter = 64;
	for (int32_t iter = 0; iter < maxIter; ++iter)
	{
		bool anyOverlap = false;
		for (size_t i = 0; i < islands.size(); ++i)
		{
			for (size_t j = i + 1; j < islands.size(); ++j)
			{
				const UnitRect ri = islands[i].outerRect.offset(islands[i].delta.cx, islands[i].delta.cy);
				const UnitRect rj = islands[j].outerRect.offset(islands[j].delta.cx, islands[j].delta.cy);
				const Unit ovX = std::min(ri.right, rj.right) - std::max(ri.left, rj.left);
				const Unit ovY = std::min(ri.bottom, rj.bottom) - std::max(ri.top, rj.top);
				if (ovX <= 0_ut || ovY <= 0_ut)
					continue;

				anyOverlap = true;
				const UnitPoint ci = ri.getCenter();
				const UnitPoint cj = rj.getCenter();
				if (ovX < ovY)
				{
					const Unit push = ovX / 2_ut + 1_ut;
					if (ci.x < cj.x)
					{
						islands[i].delta.cx -= push;
						islands[j].delta.cx += push;
					}
					else
					{
						islands[i].delta.cx += push;
						islands[j].delta.cx -= push;
					}
				}
				else
				{
					const Unit push = ovY / 2_ut + 1_ut;
					if (ci.y < cj.y)
					{
						islands[i].delta.cy -= push;
						islands[j].delta.cy += push;
					}
					else
					{
						islands[i].delta.cy += push;
						islands[j].delta.cy -= push;
					}
				}
			}
		}
		if (!anyOverlap)
			break;
	}

	// Commit. Apply per-island delta to each node, then resize the
	// group (if any) to wrap its shifted content rect.
	for (const auto& island : islands)
	{
		for (size_t k = 0; k < island.nodeIndices.size(); ++k)
		{
			const int32_t gi = island.nodeIndices[k];
			const UnitPoint planned = island.positions[k];
			const UnitPoint pt(planned.x + island.delta.cx, planned.y + island.delta.cy);
			if (pt != gNodes[gi]->getPosition())
			{
				gNodes[gi]->setPosition(pt);
				NodeMovedEvent event(graph, gNodes[gi]);
				graph->raiseEvent(&event);
			}
		}
		if (island.group != nullptr)
		{
			const UnitRect shifted = island.contentBounds.offset(island.delta.cx, island.delta.cy);
			const UnitPoint newPos(shifted.left - groupPadding, shifted.top - groupPaddingTop);
			const UnitSize newSize(
				shifted.getWidth() + groupPadding + groupPadding,
				shifted.getHeight() + groupPaddingTop + groupPadding);
			if (newPos != island.group->getPosition() || newSize != island.group->getSize())
			{
				island.group->setPosition(newPos);
				island.group->setSize(newSize);
				GroupMovedEvent event(graph, island.group);
				graph->raiseEvent(&event);
			}
		}
	}
}

}
