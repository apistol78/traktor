/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Containers/AlignedVector.h"
#include "Webber/Event.h"
#include "Webber/Widget.h"

namespace traktor::wbr
{

/*! Button which uploads a file chosen by the user.
 * \ingroup Webber
 *
 * Uploaded file is only available while the uploaded event is raised.
 */
class FileUpload : public Widget
{
	T_RTTI_CLASS;

public:
	/*! \param accept File types the browser offers, e.g. "image/*". */
	explicit FileUpload(const std::wstring& text, const std::wstring& accept);

	const std::wstring& getFileName() const;

	const AlignedVector< uint8_t >& getData() const;

	virtual void build(html::Element* parent) const override final;

	virtual void consumeValues(const std::map< int32_t, std::wstring >& values) override final;

	virtual void consume(int32_t senderId, int32_t action) override final;

	Event& uploaded();

private:
	std::wstring m_text;
	std::wstring m_accept;
	std::wstring m_fileName;
	AlignedVector< uint8_t > m_data;
	Event m_uploaded;
};

}
