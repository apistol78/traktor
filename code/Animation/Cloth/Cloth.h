/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Vector4.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::animation
{

/*!
 * \ingroup Animation
 */
class T_DLLCLASS Cloth : public ISerializable
{
	T_RTTI_CLASS;

public:
	struct Node
	{
		Vector4 position;
		int32_t east;
		int32_t north;

		void serialize(ISerializer& s);
	};

	struct Edge
	{
		int32_t indices[2];
		float length;

		void serialize(ISerializer& s);
	};

	AlignedVector< Node > m_nodes;
	AlignedVector< Edge > m_edges;
	AlignedVector< int32_t > m_triangles;

	virtual void serialize(ISerializer& s);
};

}
