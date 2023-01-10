/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Svg/IShapeVisitor.h"
#include "Svg/Shape.h"

namespace traktor
{
	namespace svg
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.svg.Shape", Shape, Object)

void Shape::setId(const std::wstring& id)
{
	m_id = id;
}

const std::wstring& Shape::getId() const
{
	return m_id;
}

void Shape::setStyle(const Style* style)
{
	m_style = style;
}

const Style* Shape::getStyle() const
{
	return m_style;
}

void Shape::setTransform(const Matrix33& transform)
{
	m_transform = transform;
}

const Matrix33& Shape::getTransform() const
{
	return m_transform;
}

Matrix33 Shape::getGlobalTransform() const
{
	Matrix33 transform = m_transform;
	for (Shape* ancestor = m_parent; ancestor != nullptr; ancestor = ancestor->m_parent)
		transform = ancestor->m_transform * transform;
	return transform;
}

Shape* Shape::getParent() const
{
	return m_parent;
}

void Shape::addChild(Shape* shape)
{
	T_FATAL_ASSERT(shape->m_parent == nullptr);
	m_children.push_back(shape);
	shape->m_parent = this;
}

void Shape::visit(IShapeVisitor* shapeVisitor)
{
	shapeVisitor->enter(this);

	for (auto child : m_children)
		child->visit(shapeVisitor);

	shapeVisitor->leave(this);
}

	}
}
