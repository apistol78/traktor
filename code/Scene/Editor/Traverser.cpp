/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Reflection/Reflection.h"
#include "Core/Reflection/RfmObject.h"
#include "Core/Reflection/RfpMemberType.h"
#include "Scene/Editor/Traverser.h"
#include "World/EntityData.h"

namespace traktor::scene
{

bool Traverser::visit(const ISerializable* object, const std::function< Result (const world::EntityData*) >& visitor)
{
	if (!object)
		return true;

	Ref< Reflection > reflection = Reflection::create(object);
	if (!reflection)
		return false;

 	RefArray< ReflectionMember > objectMembers;
 	reflection->findMembers(RfpMemberType(type_of< RfmObject >()), objectMembers);
	for (auto member : objectMembers)
	{
		RfmObject* objectMember = dynamic_type_cast< RfmObject* >(member);
		if (!objectMember->get())
			continue;

		if (auto entityData = dynamic_type_cast< const world::EntityData* >(objectMember->get()))
		{
			const Result result = visitor(entityData);
			if (result == Result::Continue)
			{
				if (!Traverser::visit(entityData, visitor))
					return false;
			}
			else if (result == Result::Failed)
				return false;
		}
		else if (objectMember->get())
		{
			if (!Traverser::visit((const ISerializable*)objectMember->get(), visitor))
				return false;
		}
	}

	return true;
}

bool Traverser::visit(ISerializable* object, const std::function< Result (Ref< world::EntityData >&) >& visitor)
{
	if (!object)
		return true;

	Ref< Reflection > reflection = Reflection::create(object);
	if (!reflection)
		return false;

	bool needToApply = false;

 	RefArray< ReflectionMember > objectMembers;
 	reflection->findMembers(RfpMemberType(type_of< RfmObject >()), objectMembers);
	for (auto member : objectMembers)
	{
		RfmObject* objectMember = dynamic_type_cast< RfmObject* >(member);
		if (!objectMember->get())
			continue;

		Ref< world::EntityData > entityData = dynamic_type_cast< world::EntityData* >(objectMember->get());
		if (entityData)
		{
			const Result result = visitor(entityData);
			if (result == Result::Continue)
			{
				if (!Traverser::visit(entityData, visitor))
					return false;
			}
			else if (result == Result::Failed)
				return false;

			if (entityData != objectMember->get())
			{
				objectMember->set(entityData);
				needToApply = true;
			}
		}
		else if (objectMember->get())
		{
			if (!Traverser::visit(objectMember->get(), visitor))
				return false;
		}
	}

	if (needToApply)
		reflection->apply(object);

	return true;
}

}
