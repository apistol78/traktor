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
#include "Core/Ref.h"
#include "Core/RefArray.h"
#include "Core/Io/IStream.h"
#include "Core/Io/Path.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_XML_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::xml
{

class Node;
class Element;

/*! XML DOM Document.
 * \ingroup XML
 */
class T_DLLCLASS Document : public Object
{
	T_RTTI_CLASS;

public:
	bool loadFromFile(const Path& fileName);

	bool loadFromStream(IStream* stream, const std::wstring& name = L"");

	bool loadFromText(const std::wstring& text, const std::wstring& name = L"");

	bool saveAsFile(const Path& fileName);

	bool saveIntoStream(IStream* stream);

	int32_t get(const std::wstring& path, RefArray< Element >& outElements) const;

	Element* getSingle(const std::wstring& path) const;

	void setDocumentElement(Element* docElement);

	Element* getDocumentElement() const;

	Ref< Document > clone() const;

private:
	Ref< Element > m_docElement;
};

}
