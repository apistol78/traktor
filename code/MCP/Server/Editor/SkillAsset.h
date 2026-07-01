/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Core/Serialization/ISerializable.h"

#include <string>

namespace traktor::mcp
{

/*! A single named, substitutable parameter of a skill.
 * \ingroup MCP
 */
class SkillParameter : public ISerializable
{
	T_RTTI_CLASS;

public:
	std::wstring m_name;
	std::wstring m_description;
	std::wstring m_defaultValue;

	virtual void serialize(ISerializer& s) override final;
};

/*! An authorable, publishable AI skill, stored as a database instance.
 * \ingroup MCP
 *
 * A skill captures a reusable procedure (e.g. "create an animated character
 * entity") as editable data: a markdown \a body plus typed \a parameters. When
 * \a published it is surfaced over MCP "prompts/list"/"prompts/get" (and so
 * appears as a slash command in the client), with \c {{parameter}} tokens in the
 * body substituted from the supplied arguments.
 */
class SkillAsset : public ISerializable
{
	T_RTTI_CLASS;

public:
	std::wstring m_name;
	std::wstring m_description;
	std::wstring m_whenToUse;
	std::wstring m_body;
	std::wstring m_engineVersion;
	bool m_published = false;
	RefArray< SkillParameter > m_parameters;

	virtual void serialize(ISerializer& s) override final;
};

}
