/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Io/IOutputStreamBuffer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! Buffer used by output streams.
 * \ingroup Core
 */
class T_DLLCLASS OutputStreamBuffer : public IOutputStreamBuffer
{
	T_RTTI_CLASS;

public:
	virtual int32_t getIndent() const override final;

	virtual void setIndent(int32_t indent) override final;

	virtual int32_t getDecimals() const override final;

	virtual void setDecimals(int32_t decimals) override final;

	virtual bool getPushIndent() const override final;

	virtual void setPushIndent(bool pushIndent) override final;

private:
	int32_t m_indent = 0;
	int32_t m_decimals = 6;
	bool m_pushIndent = false;
};

}

