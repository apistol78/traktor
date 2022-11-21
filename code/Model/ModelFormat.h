/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <vector>
#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Io/Path.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MODEL_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IStream;

}

namespace traktor::model
{

class Model;

/*! Model importer format.
 * \ingroup Model
 *
 * ModelFormat is used to import geometry and material
 * information from proprietary 3rd-party formats.
 */
class T_DLLCLASS ModelFormat : public Object
{
	T_RTTI_CLASS;

public:
	/*! Get list of common extensions of implemented format.
	 *
	 * \param outDescription Human readable description of format.
	 * \param outExtensions List of file extensions commonly used for format.
	 */
	virtual void getExtensions(std::wstring& outDescription, std::vector< std::wstring >& outExtensions) const = 0;

	/*! Determine if format support parsing file.
	 *
	 * \param extension Model extension.
	 * \return True if format is supported.
	 */
	virtual bool supportFormat(const std::wstring& extension) const = 0;

	/*! Read model.
	 *
	 * \param filePath Path to model file.
	 * \return Read model.
	 */
	virtual Ref< Model > read(const Path& filePath, const std::wstring& filter) const = 0;

	/*! Write model.
	 *
	 * \param filePath Path to model file.
	 * \param model Output model.
	 * \return True if model written successfully.
	 */
	virtual bool write(const Path& filePath, const Model* model) const = 0;

	/*! Automatically read model using appropriate format.
	 *
	 * \param filePath Path to model file.
	 * \param filter Import filter.
	 * \return Read model.
	 */
	static Ref< Model > readAny(const Path& filePath, const std::wstring& filter = L"");

	/*! Automatically write model using format based on filename extension.
	 *
	 * \param filePath Path to new model file.
	 * \param model Output model.
	 * \return True if model written successfully.
	 */
	static bool writeAny(const Path& filePath, const Model* model);
};

}
