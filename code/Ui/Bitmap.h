/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/IBitmap.h"
#include "Ui/Rect.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Color4ub;

}

namespace traktor::drawing
{

class Image;

}

namespace traktor::ui
{

/*! Bitmap.
 * \ingroup UI
 *
 * The Bitmap class is a wrapper for system
 * dependent IBitmap instances.
 */
class T_DLLCLASS Bitmap : public IBitmap
{
	T_RTTI_CLASS;

public:
	Bitmap() = default;

	explicit Bitmap(uint32_t width, uint32_t height);

	explicit Bitmap(const drawing::Image* image);

	explicit Bitmap(const drawing::Image* image, const ui::Rect& srcRect);

	virtual ~Bitmap();

	/*! Create empty bitmap.
	 *
	 * \param width Width of bitmap.
	 * \param height Height of bitmap.
	 * \return True if successful.
	 */
	bool create(uint32_t width, uint32_t height);

	/*! Create a copy of an image.
	 *
	 * \param image Source image.
	 * \return True if successful.
	 */
	bool create(const drawing::Image* image);

	/*! Create a sub-copy of an image.
	 *
	 * \param image Source image.
	 * \param srcRect Source rectangle from image.
	 * \return True if successful.
	 */
	bool create(const drawing::Image* image, const ui::Rect& srcRect);

	/*! Destroy bitmap. */
	virtual void destroy() override final;

	/*! Copy image into bitmap. */
	void copyImage(const drawing::Image* image);

	/*! Copy sub-rectangle from image into bitmap.
	 *
	 * \param image Source image.
	 * \param srcRect Source rectangle.
	 * \param destPos Destination position.
	 */
	void copySubImage(const drawing::Image* image, const ui::Rect& srcRect, const ui::Point& destPos);

	/*! Get image from bitmap. */
	virtual Ref< drawing::Image > getImage(const Widget* reference) const override final;

	/*! Get size of bitmap in pixels. */
	virtual Size getSize(const Widget* reference) const override final;

	/*! Get system bitmap. */
	virtual ISystemBitmap* getSystemBitmap(const Widget* reference) const override final;

	/*! Load bitmap from file. */
	static Ref< Bitmap > load(const std::wstring& fileName);

	/*! Load bitmap from resource. */
	static Ref< Bitmap > load(const void* resource, uint32_t size, const std::wstring& extension);

private:
	ISystemBitmap* m_bitmap = nullptr;
};

}
