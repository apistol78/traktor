/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <map>
#include <set>
#include <string>

namespace traktor::mcp
{

/*! Captured GLSL emitter source per shader node type.
 * \ingroup MCP
 *
 * Maps a node type's short name (e.g. L"Add") to the source text of the GLSL
 * emitter function that generates code for it. Generated at build time from
 * code/Render/Editor/Glsl/GlslEmitter.cpp by
 * scripts/generate-node-emitter-capture.py so the running server can describe
 * node functionality without the Traktor source tree being present.
 */
const std::map< std::wstring, std::wstring >& nodeEmitterCapture();

/*! One-line GLSL expression summary per shader node type.
 * \ingroup MCP
 *
 * A best-effort, boilerplate-stripped expression (e.g. L"Output = Input1 + Input2")
 * for nodes whose emitter is a single straightforward assignment. Absent for
 * nodes whose emission is non-trivial (use nodeEmitterCapture() for those).
 */
const std::map< std::wstring, std::wstring >& nodeEmitterSummary();

/*! Set of shader node type short names tagged T_DEPRECATED in their headers.
 * \ingroup MCP
 *
 * Derived from the [[deprecated]] tag so the server can steer clients away from
 * nodes that should no longer be used.
 */
const std::set< std::wstring >& nodeDeprecatedTypes();

}
