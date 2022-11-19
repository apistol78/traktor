/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Database/Events/EvtInstance.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DATABASE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::db
{

/*! Instance renamed event.
 * \ingroup Database
 */
class T_DLLCLASS EvtInstanceRenamed : public EvtInstance
{
	T_RTTI_CLASS;

public:
	EvtInstanceRenamed() = default;

	explicit EvtInstanceRenamed(const Guid& instanceGuid, const std::wstring& previousName);

	/*! Instance's previous name.
	 *
	 * \return Previous name of instance.
	 */
	const std::wstring& getPreviousName() const;

	virtual void serialize(ISerializer& s) override final;

private:
	std::wstring m_previousName;
};

}
