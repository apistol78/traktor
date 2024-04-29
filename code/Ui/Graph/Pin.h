/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Guid.h"
#include "Core/Object.h"
#include "Ui/Associative.h"
#include "Ui/Point.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

class Node;

/*! Graph node pin.
 * \ingroup UI
 */
class T_DLLCLASS Pin
:	public Object
,	public Associative
{
	T_RTTI_CLASS;

public:
	enum Direction
	{
		DrInput,
		DrOutput
	};

	explicit Pin(Node* node, const std::wstring& name, const Guid& id, Direction direction, bool mandatory, bool bold);

	Node* getNode() const;

	void setName(const std::wstring& name);

	const std::wstring& getName() const;

	const Guid& getId() const;

	Direction getDirection() const;

	bool isMandatory() const;

	bool isBold() const;

	UnitPoint getPosition() const;

private:
	Node* m_node;
	std::wstring m_name;
	Guid m_id;
	Direction m_direction;
	bool m_mandatory;
	bool m_bold;
};

}
