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

/*! Instance created event.
 * \ingroup Database
 */
class T_DLLCLASS EvtInstanceCreated : public EvtInstance
{
	T_RTTI_CLASS;

public:
	EvtInstanceCreated() = default;

	explicit EvtInstanceCreated(const std::wstring& groupPath, const Guid& instanceGuid);

	/*! Path to group which contain new instance.
	 *
	 * \return Group path.
	 */
	const std::wstring& getGroupPath() const;

	virtual void serialize(ISerializer& s) override final;

private:
	std::wstring m_groupPath;
};

}
