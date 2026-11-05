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
#include "Core/Serialization/ISerializable.h"

namespace traktor
{

class ISerializer;

}

namespace traktor::project
{

/*! Project presented by its author.
 * \ingroup Webber
 */
struct Project
{
	std::wstring title;
	std::wstring summary;	//!< Short plain text, shown in project lists.
	std::wstring description;	//!< Markdown.
	std::wstring author;
	uint64_t created;	//!< Seconds since the Epoch.
	uint64_t updated;	//!< Seconds since the Epoch.
	int32_t id = 0;	//!< Unique among projects; never reused.

	void serialize(ISerializer& s);
};

/*! All projects, shared by all sessions; root of the projects data file.
 * \ingroup Webber
 */
class Projects : public ISerializable
{
	T_RTTI_CLASS;

public:
	int32_t nextId = 1;	//!< Id of next new project.
	AlignedVector< Project > projects;

	virtual void serialize(ISerializer& s) override final;
};

/*! Create projects populated with samples. */
Ref< Projects > createSampleProjects();

}
