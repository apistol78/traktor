/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Serialization/ISerializable.h"

namespace traktor
{
	namespace sb
	{

class SolutionBuilderMsvcVCXProj;

/*! Visual Studio solution settings. */
class SolutionBuilderMsvcSettings : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s) override final;

	inline const std::wstring& getSLNVersion() const { return m_slnVersion; }

	inline const std::wstring& getVSVersion() const { return m_vsVersion; }

	inline SolutionBuilderMsvcVCXProj* getProject() const { return m_project; }

private:
	std::wstring m_slnVersion;
	std::wstring m_vsVersion;
	Ref< SolutionBuilderMsvcVCXProj > m_project;
};

	}
}

