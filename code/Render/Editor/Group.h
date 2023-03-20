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
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class T_DLLCLASS Group : public ISerializable
{
	T_RTTI_CLASS;

public:
	/*!
	 */
	const std::wstring& getTitle() const;

	/*! Set position.
	 *
	 * \param position New position.
	 */
	void setPosition(const std::pair< int, int >& position);

	/*! Get position.
	 *
	 * \return Position.
	 */
	const std::pair< int, int >& getPosition() const;

	/*! Set size.
	 *
	 * \param size New size.
	 */
	void setSize(const std::pair< int, int >& size);

	/*! Get size.
	 *
	 * \return Size.
	 */
	const std::pair< int, int >& getSize() const;

	virtual void serialize(ISerializer& s) override final;

private:
	std::wstring m_title;
	std::wstring m_comment;
	std::pair< int32_t, int32_t > m_position = { 0, 0 };
	std::pair< int32_t, int32_t > m_size = { 0, 0 };
};

}
