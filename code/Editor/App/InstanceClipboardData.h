/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <list>
#include "Core/Ref.h"
#include "Core/Serialization/ISerializable.h"

namespace traktor
{
	namespace editor
	{

class InstanceClipboardData : public ISerializable
{
	T_RTTI_CLASS;

public:
	struct Instance
	{
		std::wstring name;
		Ref< ISerializable > object;
		Guid originalId;
		Guid pasteId;

		void serialize(ISerializer& s);
	};

	InstanceClipboardData();

	void addInstance(const std::wstring& name, ISerializable* object, const Guid& id = Guid());

	virtual void serialize(ISerializer& s) override final;

	const std::list< Instance >& getInstances() const { return m_instances; }

private:
	std::list< Instance > m_instances;
};

	}
}

