/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Model/ModelFormat.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MODEL_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::model
{

/*! FBX model format.
 * \ingroup Model
 */
class T_DLLCLASS ModelFormatFbx : public ModelFormat
{
	T_RTTI_CLASS;

public:
	virtual void getExtensions(std::wstring& outDescription, std::vector< std::wstring >& outExtensions) const override final;

	virtual bool supportFormat(const std::wstring& extension) const override final;

	virtual Ref< Model > read(const Path& filePath, const std::wstring& filter) const override final;

	virtual bool write(const Path& filePath, const Model* model) const override final;
};

}
