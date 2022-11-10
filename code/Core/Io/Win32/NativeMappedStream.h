/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Platform.h"
#include "Core/Io/MemoryStream.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class T_DLLCLASS NativeMappedStream : public MemoryStream
{
	T_RTTI_CLASS;

public:
	NativeMappedStream(void* hFile, void* hFileMapping, void* ptr, int64_t size);

	virtual ~NativeMappedStream();

	virtual void close() override final;

private:
	void* m_hFile;
	void* m_hFileMapping;
};

}

