/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Class/IRuntimeClass.h"
#include "Core/Class/OrderedClassRegistrar.h"

namespace traktor
{

void OrderedClassRegistrar::registerClass(IRuntimeClass* runtimeClass)
{
	m_runtimeClasses.push_back(runtimeClass);
}

void OrderedClassRegistrar::registerClassesInOrder(IRuntimeClassRegistrar* registrar)
{
	while (!m_runtimeClasses.empty())
	{
		for (RefArray< IRuntimeClass >::iterator i = m_runtimeClasses.begin(); i != m_runtimeClasses.end(); )
		{
			const TypeInfo& exportType = (*i)->getExportType();
			const TypeInfo* superExportType = exportType.getSuper();
			if (superExportType)
			{
				bool pending = false;
				for (RefArray< IRuntimeClass >::iterator j = m_runtimeClasses.begin(); j != m_runtimeClasses.end(); ++j)
				{
					if (is_type_a((*j)->getExportType(), *superExportType))
					{
						pending = true;
						break;
					}
				}
				if (!pending)
				{
					registrar->registerClass(*i);
					i = m_runtimeClasses.erase(i);
				}
				else
					++i;
			}
			else
			{
				registrar->registerClass(*i);
				i = m_runtimeClasses.erase(i);
			}
		}
	}
}

}
