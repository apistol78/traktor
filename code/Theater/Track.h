/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Guid.h"
#include "Core/Object.h"
#include "Core/Math/TransformPath.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_THEATER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::theater
{

/*! Track
 * \ingroup Theater
 */
class T_DLLCLASS Track : public Object
{
	T_RTTI_CLASS;

public:
	explicit Track(
		const Guid& entityId,
		const Guid& lookAtEntityId,
		const TransformPath& path
	);

	const Guid& getEntityId() const;

	const Guid& getLookAtEntityId() const;

	const TransformPath& getPath() const;

	TransformPath& getPath();

private:
	Guid m_entityId;
	Guid m_lookAtEntityId;
	TransformPath m_path;
};

}
