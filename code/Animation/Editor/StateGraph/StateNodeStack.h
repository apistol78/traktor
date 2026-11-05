/*
 * TRAKTOR
 * Copyright (c) 2025-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Animation/Editor/StateGraph/StateNode.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Ref.h"
#include "Resource/Id.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::animation
{

class Animation;
class ITransformTimeData;

/*! Stack state node.
 *
 * \ingroup Animation
 */
class T_DLLCLASS StateNodeStack : public StateNode
{
	T_RTTI_CLASS;

public:
	struct Layer
	{
		std::wstring name;
		resource::Id< Animation > animation;
		Ref< const ITransformTimeData > transformTime;

		void serialize(ISerializer& s);
	};

	virtual void serialize(ISerializer& s) override;

	const AlignedVector< Layer >& getLayers() const { return m_layers; }

private:
	AlignedVector< Layer > m_layers;
};

}
