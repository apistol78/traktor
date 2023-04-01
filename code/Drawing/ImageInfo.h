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
#if defined(T_DRAWING_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::drawing
{

/*! Image information.
 * \ingroup Drawing
 *
 * Additional information about the image.
 * Some image formats contain additional information about
 * author, copyright notes and such.
 */
class T_DLLCLASS ImageInfo : public ISerializable
{
	T_RTTI_CLASS;

public:
	void setAuthor(const std::wstring& author);

	const std::wstring& getAuthor() const;

	void setCopyright(const std::wstring& copyright);

	const std::wstring& getCopyright() const;

	void setFormat(const std::wstring& format);

	const std::wstring& getFormat() const;

	void setGamma(float gamma);

	float getGamma() const;

	virtual void serialize(ISerializer& s) override;

private:
	std::wstring m_author;
	std::wstring m_copyright;
	std::wstring m_format;
	float m_gamma = 0.0f;
};

}
