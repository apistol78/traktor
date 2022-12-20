/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Settings/PropertyGroup.h"
#include "Net/Discovery/IService.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_NET_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::net
{

/*! Generic network service.
 * \ingroup Net
 */
class T_DLLCLASS NetworkService : public IService
{
	T_RTTI_CLASS;

public:
	NetworkService() = default;

	explicit NetworkService(const std::wstring& type, const PropertyGroup* properties);

	const std::wstring& getType() const;

	const PropertyGroup* getProperties() const;

	virtual void serialize(ISerializer& s) override final;

private:
	std::wstring m_type;
	Ref< const PropertyGroup > m_properties;
};

}
