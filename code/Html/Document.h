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
#include "Core/Object.h"
#include "Core/Io/IStream.h"
#include "Core/Io/OutputStream.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_HTML_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IEncoding;

	namespace html
	{

class Element;

/*! HTML DOM Document.
 * \ingroup HTML
 */
class T_DLLCLASS Document : public Object
{
	T_RTTI_CLASS;

public:
	Document() = default;

	explicit Document(bool parseComments);

	bool loadFromFile(const std::wstring& filename, const IEncoding* encoding);

	bool loadFromStream(IStream* stream, const IEncoding* encoding);

	bool loadFromText(const std::wstring& text);

	bool saveAsFile(const std::wstring& filename);

	bool saveIntoStream(IStream* stream);

	void setDocumentElement(Element* docElement);

	Element* getDocumentElement() const;

	void toString(OutputStream& os) const;

private:
	bool m_parseComments = false;
	Ref< Element > m_docElement;
};

	}
}

