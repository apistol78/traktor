/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#define _WIN32_LEAN_AND_MEAN
#define WINVER 0x0A00
#define _WIN32_WINNT 0x0A00
#include <windows.h>
#if defined(T_USE_GDI_PLUS)
// \hack Define min/max only while including gdiplus.h.
#	define max(a,b) (((a) > (b)) ? (a) : (b))
#	define min(a,b) (((a) < (b)) ? (a) : (b))
#	include <gdiplus.h>
#	undef min
#	undef max
#endif
#include "Core/IRefCount.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Ref.h"
#include "Core/Thread/SpinLock.h"
#include "Ui/Itf/ISystemBitmap.h"

namespace traktor::ui
{

/*! Win32 system bitmap.
 * \ingroup UIW32
 */
class BitmapWin32 : public ISystemBitmap
{
public:
	/*! Lifetime of a bitmap; outlives the bitmap and is no longer alive when bitmap is destroyed. */
	class Lifetime : public RefCountImpl< IRefCount >
	{
	public:
		bool alive() const { return m_alive; }

	private:
		friend class BitmapWin32;
		std::atomic< bool > m_alive = true;
	};

	BitmapWin32();

	virtual ~BitmapWin32();

	virtual bool create(uint32_t width, uint32_t height);

	virtual void destroy();

	virtual void copySubImage(const drawing::Image* image, const Rect& srcRect, const Point& destPos);

	virtual Ref< drawing::Image > getImage() const;

	virtual Size getSize() const;

	const void* getBits() const { return m_bits.c_ptr(); }

	const void* getBitsPreMulAlpha() const { return m_bitsPreMulAlpha.c_ptr(); }

	bool haveAlpha() const { return m_haveAlpha; }

	HICON createIcon() const;

#if defined(T_USE_GDI)
	HBITMAP getHBitmap() const { return m_hBitmap; }

	HBITMAP getHBitmapPreMulAlpha() const { return m_hBitmapPreMulAlpha; }
#endif

	int32_t getTag() const { return m_tag; }

	int32_t getRevision() const { return m_revision; }

	/*! Get area modified after revision, entire bitmap if no longer recorded; returns current revision. */
	int32_t getModified(int32_t revision, Rect& outRect) const;

	const Lifetime* getLifetime() const { return m_lifetime; }

	/*! Get number of destroyed bitmaps. */
	static int32_t getDestroyedCount() { return ms_destroyedCount; }

private:
	struct Modification
	{
		int32_t revision = 0;
		Rect rect;
	};

	static std::atomic< int32_t > ms_nextTag;
	static std::atomic< int32_t > ms_destroyedCount;
	Ref< Lifetime > m_lifetime;
	int32_t m_tag = 0;
	std::atomic< int32_t > m_revision = 0;
	mutable SpinLock m_modificationsLock;
	Modification m_modifications[8];
	AutoArrayPtr< uint32_t > m_bits;
	AutoArrayPtr< uint32_t > m_bitsPreMulAlpha;
	uint32_t m_width = 0;
	uint32_t m_height = 0;
	bool m_haveAlpha = false;

#if defined(T_USE_GDI)
	HBITMAP m_hBitmap = NULL;
	HBITMAP m_hBitmapPreMulAlpha = NULL;
#endif
};

}
