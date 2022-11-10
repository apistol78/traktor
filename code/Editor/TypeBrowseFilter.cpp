/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Editor/TypeBrowseFilter.h"
#include "Database/Instance.h"

namespace traktor::editor
{

TypeBrowseFilter::TypeBrowseFilter(const TypeInfoSet& typeSet)
:	m_typeSet(typeSet)
{
}

TypeBrowseFilter::TypeBrowseFilter(const TypeInfo& typeInfo)
{
	m_typeSet.insert(&typeInfo);
}

bool TypeBrowseFilter::acceptable(db::Instance* instance) const
{
	// This could happen if database contains instances which
	// can't be loaded with current set of loaded modules.
	if (!instance || !instance->getPrimaryType())
		return false;

	for (TypeInfoSet::const_iterator i = m_typeSet.begin(); i != m_typeSet.end(); ++i)
	{
		if (is_type_of(*(*i), *instance->getPrimaryType()))
			return true;
	}

	return false;
}

}
