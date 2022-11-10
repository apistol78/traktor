/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Vector4.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SHAPE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace model
	{
	
class Model;

	}

	namespace shape
	{
	
/*! Shape interface.
 * \ingroup Shape
 */
class T_DLLCLASS IShape : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual Ref< model::Model > createModel() const = 0;

	virtual void createAnchors(AlignedVector< Vector4 >& outAnchors) const = 0;
};

	}
}
