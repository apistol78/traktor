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
#include "Core/Math/Matrix33.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SVG_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::svg
{

class IShapeVisitor;
class Style;

/*! SVG shape.
 * \ingroup SVG
 */
class T_DLLCLASS Shape : public Object
{
	T_RTTI_CLASS;

public:
	void setId(const std::wstring& id);

	const std::wstring& getId() const;

	void setStyle(const Style* style);

	const Style* getStyle() const;

	void setTransform(const Matrix33& transform);

	const Matrix33& getTransform() const;

	Matrix33 getGlobalTransform() const;

	Shape* getParent() const;

	void addChild(Shape* shape);

	virtual void visit(IShapeVisitor* shapeVisitor);

private:
	std::wstring m_id;
	Ref< const Style > m_style;
	Matrix33 m_transform = Matrix33::identity();
	Shape* m_parent = nullptr;
	RefArray< Shape > m_children;
};

}
