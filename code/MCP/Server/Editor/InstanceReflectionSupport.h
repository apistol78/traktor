/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Containers/AlignedVector.h"
#include "Core/Ref.h"
#include "MCP/Server/Editor/McpToolSupport.h"

namespace traktor
{

class ISerializable;
class ReflectionMember;

}

namespace traktor::mcp
{

class Json;

/*! Default recursion depth when describing reflected members. */
constexpr int32_t c_defaultMaxDepth = 8;

/*! A single member path step: a member name, or an array index. */
struct PathStep
{
	bool index;
	std::wstring name;
	int32_t idx;
};

/*! Recursively describe a reflected member as JSON (name, kind, type, value). */
Ref< Json > describeMember(const ReflectionMember* member, int32_t depth);

/*! Tokenize a dotted/indexed path ("a.b[2].c") into steps. */
bool tokenizePath(const std::wstring& path, AlignedVector< PathStep >& outSteps, std::wstring& outError);

/*! Set the member at \a steps to \a spec, descending through object references
 * and applying bottom-up onto \a object and each crossed nested object. */
bool setMemberThroughPath(db::Database* database, ISerializable* object, const AlignedVector< PathStep >& steps, size_t start, const Json* spec, std::wstring& outError);

/*! Read-only: describe the member at \a steps, descending through object refs. */
Ref< Json > describeMemberAtPath(ISerializable* object, const AlignedVector< PathStep >& steps, size_t start);

}
