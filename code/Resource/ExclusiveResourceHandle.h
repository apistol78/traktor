/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Resource/ResourceHandle.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RESOURCE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::resource
{

/*! Exclusive resource handle.
 * \ingroup Resource
 *
 * Exclusive resource handles automatically
 * release contained object when last
 * external reference is released.
 */
class T_DLLCLASS ExclusiveResourceHandle : public ResourceHandle
{
	T_RTTI_CLASS;

public:
	explicit ExclusiveResourceHandle(const TypeInfo& type);

	virtual void release(void* owner) const override final;

	/*! Get the type of resource, ie product type. */
	const TypeInfo& getProductType() const { return m_resourceType; }

private:
	const TypeInfo& m_resourceType;
};

}
