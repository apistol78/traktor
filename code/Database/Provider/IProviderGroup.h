/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Guid.h"
#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/RefArray.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DATABASE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{

class IProviderInstance;

/*! Provider group interface.
 * \ingroup Database
 */
class T_DLLCLASS IProviderGroup : public Object
{
	T_RTTI_CLASS;

public:
	/*! Get name of group.
	 *
	 * \return Group name.
	 */
	virtual std::wstring getName() const = 0;

	/*! Rename group.
	 *
	 * \param name New name of group.
	 * \return True if group was renamed successfully.
	 */
	virtual bool rename(const std::wstring& name) = 0;

	/*! Remove group.
	 *
	 * \return True if group was removed.
	 */
	virtual bool remove() = 0;

	/*! Create new child group.
	 *
	 * \param groupName Name of child group.
	 * \return Group object.
	 */
	virtual Ref< IProviderGroup > createGroup(const std::wstring& groupName) = 0;

	/*! Create new child instance.
	 *
	 * \param instanceName Name of child instance.
	 * \param instanceGuid Guid of child instance.
	 * \return Instance object.
	 */
	virtual Ref< IProviderInstance > createInstance(const std::wstring& instanceName, const Guid& instanceGuid) = 0;

	/*! Get all children.
	 *
	 * \param outChildGroups Returning array of all child groups.
	 * \param outChildInstances Returning array of all child instances.
	 * \return True if children was returned.
	 */
	virtual bool getChildren(RefArray< IProviderGroup >& outChildGroups, RefArray< IProviderInstance >& outChildInstances) = 0;
};

	}
}

