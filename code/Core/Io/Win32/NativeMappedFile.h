/*
 * TRAKTOR
 * Copyright (c) 2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Io/IMappedFile.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*!
 * \ingroup Core
 */
class T_DLLCLASS NativeMappedFile : public IMappedFile
{
	T_RTTI_CLASS;

public:
	explicit NativeMappedFile(void* hFile, void* hFileMapping, void* base, int64_t size);

	virtual ~NativeMappedFile();

	virtual void* getBase() const override final;

	virtual int64_t getSize() const override final;

private:
	void* m_hFile;
	void* m_hFileMapping;
	void* m_base;
	int64_t m_size;
};

}
