/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Heightfield/HeightfieldClassFactory.h"

#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/Boxes/BoxedVector4.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Heightfield/Heightfield.h"

namespace traktor::hf
{
namespace
{

Vector4 Heightfield_gridToWorld_Vector4(Heightfield* self, float gridX, float gridZ)
{
	return self->gridToWorld(gridX, gridZ);
}

}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.hf.HeightfieldClassFactory", 0, HeightfieldClassFactory, IRuntimeClassFactory)

void HeightfieldClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	auto classHeightfield = new AutoRuntimeClass< Heightfield >();
	classHeightfield->addConstructor< int32_t, const Vector4& >();
	classHeightfield->addProperty("size", &Heightfield::getSize);
	classHeightfield->addProperty("worldExtent", &Heightfield::getWorldExtent);
	registrar->registerClass(classHeightfield);
}

}
