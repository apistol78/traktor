/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Misc/FourCC.h"
#include "Core/Object.h"
#include "Core/Ref.h"

#include <functional>

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IStream;

}

namespace traktor::render
{

class Mesh;
class MeshFactory;

/*! Render mesh reader.
 * \ingroup Render
 */
class T_DLLCLASS MeshReader : public Object
{
	T_RTTI_CLASS;

public:
	struct AuxPatch
	{
		FourCC id;
		uint32_t elementSize;
		std::function< void(void*) > fn;
	};

	explicit MeshReader(const MeshFactory* meshFactory);

	Ref< Mesh > read(IStream* stream, const AlignedVector< AuxPatch >& patches = {}) const;

private:
	Ref< const MeshFactory > m_meshFactory;
};

}
