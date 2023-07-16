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
#include "Animation/Pose.h"
#include "Core/Serialization/ISerializable.h"
#include "Resource/IdProxy.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::animation
{

class Animation;
class StateContext;

/*! Animation state node.
 * \ingroup Animation
 */
class T_DLLCLASS StateNode : public ISerializable
{
	T_RTTI_CLASS;

public:
	StateNode() = default;

	explicit StateNode(const std::wstring& name, const resource::IdProxy< Animation >& animation);

	const std::wstring& getName() const;

	void setPosition(const std::pair< int, int >& position);

	const std::pair< int, int >& getPosition() const;

	bool bind(resource::IResourceManager* resourceManager);

	bool prepareContext(StateContext& outContext) const;

	void evaluate(StateContext& context, Pose& outPose) const;

	virtual void serialize(ISerializer& s) override;

	const resource::IdProxy< Animation >& getAnimation() const { return m_animation; }

private:
	std::wstring m_name;
	std::pair< int, int > m_position = { 0, 0 };
	resource::IdProxy< Animation > m_animation;
};

}
