/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Transform.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MODEL_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::model
{

class Model;

/*! Pose
 *         A pose is a set of joint transformations.
 *
 * The transformation stored in Joint is rest/bind pose, aka the T-pose.
 * Transformations in a pose replace each joint's T-pose transformation.
 *
 * \ingroup Model
 */
class T_DLLCLASS Pose : public ISerializable
{
	T_RTTI_CLASS;

public:
	void setJointTransform(uint32_t jointId, const Transform& jointTransform);

	const Transform& getJointTransform(uint32_t jointId) const;

	/*! Evaluate global joint transformation.
	 *         Traverse chain of joints to concate the global joint transform (in object space).
	 *
	 * \param model Model containing base skeleton with joint relationships.
	 * \param jointId Id of joint which we want global transformation calculated.
	 * \return Global joint transformation.
	 */
	Transform getJointGlobalTransform(const Model* model, uint32_t jointId) const;

	virtual void serialize(ISerializer& s) override final;

private:
	AlignedVector< Transform > m_jointTransforms;
};

}
