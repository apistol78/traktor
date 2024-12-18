/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Ref.h"
#include "Core/Containers/SmallMap.h"
#include "Ui/Associative.h"

namespace traktor::ui
{

class Associative::Impl
{
public:
	SmallMap< std::wstring, Ref< Object > > m_data;

	void setData(const std::wstring& key, Object* data)
	{
		if (data)
			m_data[key] = data;
		else
		{
			auto it = m_data.find(key);
			if (it != m_data.end())
				m_data.erase(it);
		}
	}

	Object* getData(const std::wstring& key) const
	{
		auto it = m_data.find(key);
		return (it != m_data.end()) ? it->second.ptr() : nullptr;
	}

	void copyData(const Associative* source)
	{
		for (const auto& kv : source->m_impl->m_data)
			setData(kv.first, kv.second);
	}
};

Associative::~Associative()
{
	delete m_impl;
}

void Associative::removeAllData()
{
	delete m_impl;
	m_impl = nullptr;
}

void Associative::setData(const std::wstring& key, Object* data)
{
	if (!m_impl)
		m_impl = new Impl();

	m_impl->setData(key, data);
}

Object* Associative::getData(const std::wstring& key) const
{
	if (m_impl)
		return m_impl->getData(key);
	else
		return nullptr;
}

void Associative::copyData(const Associative* source)
{
	if (!m_impl)
		m_impl = new Impl();

	m_impl->copyData(source);
}

}
