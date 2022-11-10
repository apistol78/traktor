/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/SmallMap.h"
#include "Render/Editor/Glsl/GlslType.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class TypeInfo;

}

namespace traktor::render
{

class GlslContext;
class Node;

struct Emitter;

/*!
 * \ingroup Render
 */
class T_DLLCLASS GlslEmitter
{
public:
	explicit GlslEmitter(GlslDialect dialect);

	virtual ~GlslEmitter();

	bool emit(GlslContext& c, Node* node);

private:
	SmallMap< const TypeInfo*, Emitter* > m_emitters;
};

}
