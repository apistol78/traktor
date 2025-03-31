/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Core/RefArray.h"
#include "Core/Serialization/ISerializable.h"
#include "Resource/Id.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::mesh
{

class SkinnedMesh;

}

namespace traktor::animation
{

class StateNode;
class StateTransition;
class Skeleton;

/*! Animation state graph.
 * \ingroup Animation
 */
class T_DLLCLASS StateGraph : public ISerializable
{
	T_RTTI_CLASS;

public:
	void addState(StateNode* state);

	void removeState(StateNode* state);

	const RefArray< StateNode >& getStates() const;

	void addTransition(StateTransition* transition);

	void removeTransition(StateTransition* transition);

	const RefArray< StateTransition >& getTransitions() const;

	void setRootState(StateNode* rootState);

	Ref< StateNode > getRootState() const;

	const resource::Id< Skeleton >& getPreviewSkeleton() const { return m_previewSkeleton; }

	const resource::Id< mesh::SkinnedMesh >& getPreviewMesh() const { return m_previewMesh; }

	virtual void serialize(ISerializer& s) override final;

private:
	RefArray< StateNode > m_states;
	RefArray< StateTransition > m_transitions;
	Ref< StateNode > m_rootState;
	resource::Id< Skeleton > m_previewSkeleton;
	resource::Id< mesh::SkinnedMesh > m_previewMesh;
};

}
