/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Svg/PathShape.h"

namespace traktor::svg
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.svg.PathShape", PathShape, Shape)

PathShape::PathShape(const Path& path)
:	m_path(path)
{
}

const Path& PathShape::getPath() const
{
	return m_path;
}

}
