/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Database/Remote/IMessage.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DATABASE_REMOTE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::db
{

/*! Read object result.
 * \ingroup Database
 */
class T_DLLCLASS DbmReadObjectResult : public IMessage
{
	T_RTTI_CLASS;

public:
	explicit DbmReadObjectResult(uint32_t streamId = 0, const std::wstring_view& serializerTypeName = L"");

	uint32_t getStreamId() const { return m_streamId; }

	const std::wstring& getSerializerTypeName() const { return m_serializerTypeName; }

	virtual void serialize(ISerializer& s) override final;

private:
	uint32_t m_streamId;
	std::wstring m_serializerTypeName;
};

}
