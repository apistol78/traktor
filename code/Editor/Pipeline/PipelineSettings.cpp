/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/String.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Settings/IPropertyValue.h"
#include "Core/Settings/PropertyGroup.h"
#include "Editor/Pipeline/PipelineSettings.h"

namespace traktor::editor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.PipelineSettings", PipelineSettings, IPipelineSettings)

PipelineSettings::PipelineSettings(const PropertyGroup* settings)
:	m_settings(settings)
,	m_hash(0)
{
}

Ref< const IPropertyValue > PipelineSettings::getProperty(const std::wstring& propertyName, bool includeInHash, const IPropertyValue* defaultValue) const
{
	Ref< const IPropertyValue > prop = m_settings->getProperty(propertyName);
	if (includeInHash)
	{
		m_log << L"\"" << propertyName << L"\"";
		if (prop)
		{
			const uint32_t hash = DeepHash(prop).get();
			m_log << L" " << str(L"0x%08x", hash);
			m_hash += hash;
		}
		else if(defaultValue)
		{
			const uint32_t hash = DeepHash(defaultValue).get();
			m_log << L" " << str(L"0x%08x", hash) << L" (default)";
			m_hash += hash;
		}
		else
			m_log << L" 0x00000000 (missing)";
		m_log << Endl;
	}
	if (prop)
		return prop;
	else
		return defaultValue;
}

uint32_t PipelineSettings::getHash() const
{
	return m_hash;
}

std::wstring PipelineSettings::getLog() const
{
	return m_log.str();
}

}
