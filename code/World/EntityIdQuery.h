/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/RefArray.h"
#include "Core/Math/Vector2.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class Buffer;

}

namespace traktor::world
{

class Entity;

/*! Query which entity is rendered at a position in view.
 * \ingroup World
 *
 * Given to a world renderer through WorldCreateDesc; the world renderer then renders
 * the id of every entity the frame after a request and reads back the id at the
 * requested position. Intended for picking in the editor, the entity id technique
 * is only built into shaders for the editor.
 */
class T_DLLCLASS EntityIdQuery : public Object
{
	T_RTTI_CLASS;

public:
	/*! Check if query is supported by the world renderer. */
	bool isSupported() const { return m_supported; }

	/*! Request entity rendered at a position, replaces any pending request.
	 *
	 * \param position Position in view, normalized [0, 1] with origin at top left.
	 */
	void request(const Vector2& position);

	/*! Poll result of request.
	 *
	 * The id is read back from the GPU thus the result is available a few frames after the request.
	 *
	 * \param outEntity Entity rendered at requested position, null if none.
	 * \return True when request has been resolved.
	 */
	bool poll(Ref< Entity >& outEntity);

private:
	friend class EntityIdPass;

	enum class State
	{
		Idle,
		Requested,
		Rendered
	};

	State m_state = State::Idle;
	Vector2 m_position = Vector2(0.0f, 0.0f);
	bool m_supported = false;
	Ref< render::Buffer > m_readBackBuffer;
	RefArray< Entity > m_entities;			//!< Entity of each id, first id is 1.
	RefArray< Entity > m_instanceEntities;	//!< Entity of each culling instance, first id is m_instanceBase.
	uint32_t m_instanceBase = 0;
};

}
