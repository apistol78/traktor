/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Editor/StateGraph.h"

#include "Animation/Editor/StateNode.h"
#include "Animation/Editor/StateTransition.h"
#include "Animation/Skeleton.h"
#include "Core/Serialization/AttributePrivate.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Mesh/Skinned/SkinnedMesh.h"
#include "Resource/Member.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.animation.StateGraph", 0, StateGraph, ISerializable)

void StateGraph::addState(StateNode* state)
{
	m_states.push_back(state);
}

void StateGraph::removeState(StateNode* state)
{
	m_states.remove(state);

	// Unbind root state also if we're removing root.
	if (m_rootState == state)
		m_rootState = nullptr;
}

const RefArray< StateNode >& StateGraph::getStates() const
{
	return m_states;
}

void StateGraph::addTransition(StateTransition* transition)
{
	m_transitions.push_back(transition);
}

void StateGraph::removeTransition(StateTransition* transition)
{
	m_transitions.remove(transition);
}

const RefArray< StateTransition >& StateGraph::getTransitions() const
{
	return m_transitions;
}

void StateGraph::setRootState(StateNode* rootState)
{
	m_rootState = rootState;
}

Ref< StateNode > StateGraph::getRootState() const
{
	return m_rootState;
}

void StateGraph::setPreviewPosition(const Vector4& position)
{
	m_previewPosition = position;
}

void StateGraph::setPreviewAngles(const Vector4& angles)
{
	m_previewAngles = angles;
}

void StateGraph::serialize(ISerializer& s)
{
	s >> MemberRefArray< StateNode >(L"states", m_states, AttributePrivate());
	s >> MemberRefArray< StateTransition >(L"transitions", m_transitions, AttributePrivate());
	s >> MemberRef< StateNode >(L"rootState", m_rootState, AttributePrivate());
	s >> resource::Member< Skeleton >(L"previewSkeleton", m_previewSkeleton);
	s >> resource::Member< mesh::SkinnedMesh >(L"previewMesh", m_previewMesh);
	s >> Member< Vector4 >(L"previewPosition", m_previewPosition, AttributePrivate());
	s >> Member< Vector4 >(L"previewAngles", m_previewAngles, AttributePrivate());
}

}
