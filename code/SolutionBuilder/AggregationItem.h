/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOLUTIONBUILDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::sb
{

class T_DLLCLASS AggregationItem : public ISerializable
{
	T_RTTI_CLASS;

public:
	/*! Set source file path.
	 *
	 * \param sourceFile Source path.
	 */
	void setSourceFile(const std::wstring& sourceFile);

	/*! Get source file path.
	 */
	const std::wstring& getSourceFile() const;

	/*! Set target path.
	 *
	 * Target path is relative to aggregation root;
	 * for instance the Xcode generated aggregation
	 * is the bundle root.
	 *
	 * \param targetPath Target path.
	 */
	void setTargetPath(const std::wstring& targetPath);

	/*! Get target file path.
	 */
	const std::wstring& getTargetPath() const;

	virtual void serialize(ISerializer& s) override final;

private:
	std::wstring m_sourceFile;
	std::wstring m_targetPath;
};

}
