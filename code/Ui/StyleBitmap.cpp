/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/SafeDestroy.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/StyleBitmap.h"
#include "Ui/StyleSheet.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.StyleBitmap", StyleBitmap, IBitmap)

StyleBitmap::StyleBitmap(const wchar_t* const name)
:	m_name(name)
,	m_ownDefaultBitmap(false)
{
}

StyleBitmap::StyleBitmap(const wchar_t* const name, IBitmap* defaultBitmap)
:	m_name(name)
,	m_defaultBitmap(defaultBitmap)
,	m_ownDefaultBitmap(false)
{
	T_FATAL_ASSERT_M(m_defaultBitmap, L"Default bitmap is null.");
}

// StyleBitmap::StyleBitmap(const wchar_t* const name, const void* defaultBitmapResource, uint32_t defaultBitmapResourceSize)
// :	m_name(name)
// ,	m_defaultBitmap(Bitmap::load(defaultBitmapResource, defaultBitmapResourceSize, L"image"))
// ,	m_ownDefaultBitmap(true)
// {
// 	T_FATAL_ASSERT_M(m_defaultBitmap, L"Unable to load default bitmap resource.");
// }

StyleBitmap::~StyleBitmap()
{
	destroy();
}

void StyleBitmap::destroy()
{
	if (m_ownDefaultBitmap)
	{
		safeDestroy(m_defaultBitmap);
		m_ownDefaultBitmap = false;
	}
	safeDestroy(m_bitmap);
}

Size StyleBitmap::getSize(int32_t dpi) const
{
	return resolve(dpi) ? m_bitmap->getSize(dpi) : Size();
}

Ref< drawing::Image > StyleBitmap::getImage(int32_t dpi) const
{
	return resolve(dpi) ? m_bitmap->getImage(dpi) : nullptr;
}

ISystemBitmap* StyleBitmap::getSystemBitmap(int32_t dpi) const
{
	return resolve(dpi) ? m_bitmap->getSystemBitmap(dpi) : nullptr;
}

bool StyleBitmap::resolve(int32_t dpi) const
{
	const StyleSheet* ss = Application::getInstance()->getStyleSheet();
	if (!ss)
	{
		m_bitmap = m_defaultBitmap;
		return (bool)(m_bitmap != nullptr);
	}

	std::wstring bmp = ss->getValue(m_name);
	if (!bmp.empty() && bmp == m_path)
		return (bool)(m_bitmap != nullptr);

	safeDestroy(m_bitmap);

	if ((m_bitmap = Bitmap::load(bmp, dpi)) == nullptr)
		m_bitmap = m_defaultBitmap;

	m_path = bmp;
	return (bool)(m_bitmap != nullptr);
}

	}
}
