/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "MCP/Server/Editor/InstanceReflectionSupport.h"

#include "Core/Guid.h"
#include "Core/Io/Path.h"
#include "Core/Math/Color4f.h"
#include "Core/Math/Color4ub.h"
#include "Core/Math/Matrix44.h"
#include "Core/Math/Quaternion.h"
#include "Core/Math/Scalar.h"
#include "Core/Math/Vector2.h"
#include "Core/Math/Vector4.h"
#include "Core/Reflection/Reflection.h"
#include "Core/Reflection/RfmArray.h"
#include "Core/Reflection/RfmCompound.h"
#include "Core/Reflection/RfmEnum.h"
#include "Core/Reflection/RfmObject.h"
#include "Core/Reflection/RfmPrimitive.h"
#include "Core/Rtti/TypeInfo.h"
#include "Core/Serialization/AttributeType.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Serialization/ISerializable.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "MCP/Server/Json.h"

#include <cwchar>

namespace traktor::mcp
{
namespace
{

/*! ASCII-simplified narrow/wide conversion for the rare std::string member. */
std::wstring widen(const std::string& s)
{
	return std::wstring(s.begin(), s.end());
}

std::string narrow(const std::wstring& s)
{
	std::string r;
	r.reserve(s.size());
	for (wchar_t c : s)
		r.push_back((char)c);
	return r;
}

Ref< Json > floatArray(const float* v, int32_t count)
{
	Ref< Json > a = Json::createArray();
	for (int32_t i = 0; i < count; ++i)
		a->push(Json::createReal(v[i]));
	return a;
}

/*! Read \a count floats from a JSON array (missing entries default to 0). */
bool readFloatArray(const Json* value, float* out, int32_t count, std::wstring& outError)
{
	for (int32_t i = 0; i < count; ++i)
		out[i] = 0.0f;
	if (!value || !value->isArray())
	{
		outError = L"Expected an array of numbers.";
		return false;
	}
	const int32_t n = (int32_t)value->size();
	for (int32_t i = 0; i < count && i < n; ++i)
		out[i] = (float)value->at(i)->getReal();
	return true;
}

/*! Set a primitive/enum leaf member from a JSON value. */
bool setLeafFromJson(db::Database* database, ReflectionMember* member, const Json* value, std::wstring& outError)
{
	if (!value)
	{
		outError = L"Missing \"value\".";
		return false;
	}

	// Defense-in-depth: some MCP clients deliver the value as a JSON-encoded
	// string (e.g. "[1,0,0,0]" or "true") for non-string leaves. Re-parse so
	// numeric, boolean and vector members still receive structured data; string
	// family leaves (wide/narrow string, path, enum, guid) keep the literal.
	Ref< Json > reparsed;
	if (value->isString() &&
		!is_a< RfmPrimitiveWideString >(member) &&
		!is_a< RfmPrimitiveString >(member) &&
		!is_a< RfmPrimitivePath >(member) &&
		!is_a< RfmEnum >(member) &&
		!is_a< RfmPrimitiveGuid >(member))
	{
		reparsed = Json::parse(value->getString());
		if (reparsed)
			value = reparsed;
	}

	if (auto m = dynamic_type_cast< RfmPrimitiveBoolean* >(member))
	{
		m->set(value->getBoolean());
		return true;
	}
	if (auto m = dynamic_type_cast< RfmPrimitiveInt8* >(member))
	{
		m->set((int8_t)value->getNumber());
		return true;
	}
	if (auto m = dynamic_type_cast< RfmPrimitiveUInt8* >(member))
	{
		m->set((uint8_t)value->getNumber());
		return true;
	}
	if (auto m = dynamic_type_cast< RfmPrimitiveInt16* >(member))
	{
		m->set((int16_t)value->getNumber());
		return true;
	}
	if (auto m = dynamic_type_cast< RfmPrimitiveUInt16* >(member))
	{
		m->set((uint16_t)value->getNumber());
		return true;
	}
	if (auto m = dynamic_type_cast< RfmPrimitiveInt32* >(member))
	{
		m->set((int32_t)value->getNumber());
		return true;
	}
	if (auto m = dynamic_type_cast< RfmPrimitiveUInt32* >(member))
	{
		m->set((uint32_t)value->getNumber());
		return true;
	}
	if (auto m = dynamic_type_cast< RfmPrimitiveInt64* >(member))
	{
		m->set(value->getNumber());
		return true;
	}
	if (auto m = dynamic_type_cast< RfmPrimitiveUInt64* >(member))
	{
		m->set((uint64_t)value->getNumber());
		return true;
	}
	if (auto m = dynamic_type_cast< RfmPrimitiveFloat* >(member))
	{
		m->set((float)value->getReal());
		return true;
	}
	if (auto m = dynamic_type_cast< RfmPrimitiveDouble* >(member))
	{
		m->set(value->getReal());
		return true;
	}
	if (auto m = dynamic_type_cast< RfmPrimitiveWideString* >(member))
	{
		m->set(value->getString());
		return true;
	}
	if (auto m = dynamic_type_cast< RfmPrimitiveString* >(member))
	{
		m->set(narrow(value->getString()));
		return true;
	}
	if (auto m = dynamic_type_cast< RfmPrimitivePath* >(member))
	{
		m->set(Path(value->getString()));
		return true;
	}
	if (auto m = dynamic_type_cast< RfmPrimitiveScalar* >(member))
	{
		m->set(Scalar((float)value->getReal()));
		return true;
	}
	if (auto m = dynamic_type_cast< RfmEnum* >(member))
	{
		m->set(value->getString());
		return true;
	}
	if (auto m = dynamic_type_cast< RfmPrimitiveGuid* >(member))
	{
		const std::wstring s = value->getString();
		const Guid g(s);
		if (!s.empty() && !g.isValid())
		{
			outError = L"Invalid guid value: " + s;
			return false;
		}
		// Guard resource references. A resource::Id< T > member is reflected as a guid
		// carrying an AttributeType (T, the resource's product type); a plain Guid member
		// (e.g. an entity id) carries none. When such a reference is set to a non-null
		// guid, require it to resolve to an existing source-database instance, so a typo
		// or stale guid is rejected at write time rather than silently dangling.
		if (g.isNotNull() && database != nullptr)
		{
			const Attribute* attributes = m->getAttributes();
			const AttributeType* resourceType = (attributes != nullptr) ? attributes->find< AttributeType >() : nullptr;
			if (resourceType != nullptr && !database->getInstance(g))
			{
				outError = L"Resource reference not found in database: " + s + L" (member \"" + std::wstring(m->getName()) + L"\" expects a " + std::wstring(resourceType->getMemberType().getName()) + L" resource).";
				return false;
			}
		}
		m->set(g);
		return true;
	}
	if (auto m = dynamic_type_cast< RfmPrimitiveVector4* >(member))
	{
		float v[4];
		if (!readFloatArray(value, v, 4, outError))
			return false;
		m->set(Vector4(v[0], v[1], v[2], v[3]));
		return true;
	}
	if (auto m = dynamic_type_cast< RfmPrimitiveVector2* >(member))
	{
		float v[2];
		if (!readFloatArray(value, v, 2, outError))
			return false;
		m->set(Vector2(v[0], v[1]));
		return true;
	}
	if (auto m = dynamic_type_cast< RfmPrimitiveColor4f* >(member))
	{
		float v[4];
		if (!readFloatArray(value, v, 4, outError))
			return false;
		m->set(Color4f(v[0], v[1], v[2], v[3]));
		return true;
	}
	if (auto m = dynamic_type_cast< RfmPrimitiveColor4ub* >(member))
	{
		float v[4];
		if (!readFloatArray(value, v, 4, outError))
			return false;
		m->set(Color4ub((uint8_t)v[0], (uint8_t)v[1], (uint8_t)v[2], (uint8_t)v[3]));
		return true;
	}
	if (auto m = dynamic_type_cast< RfmPrimitiveQuaternion* >(member))
	{
		float v[4];
		if (!readFloatArray(value, v, 4, outError))
			return false;
		m->set(Quaternion(v[0], v[1], v[2], v[3]));
		return true;
	}

	if (is_a< RfmCompound >(member))
	{
		outError = L"Target member is a compound/array; navigate to a primitive or enum leaf (e.g. \"a.b[0].c\") to set a value.";
		return false;
	}
	if (is_a< RfmObject >(member))
	{
		outError = L"Target member is a nested object; setting whole objects is not supported (set their primitive leaves individually).";
		return false;
	}
	outError = L"Unsupported member type for set.";
	return false;
}

/*! Resolve \a steps within a single reflection tree.
 *
 * Returns the reached member. If the path continues past an object reference,
 * stops there: \a outDescend is set to that RfmObject and \a outNext to the
 * index of the first not-yet-consumed step, so the caller can descend into the
 * nested object's own (shallow) reflection.
 */
ReflectionMember* resolveSteps(RfmCompound* root, const AlignedVector< PathStep >& steps, size_t start, RfmObject*& outDescend, size_t& outNext, std::wstring& outError)
{
	outDescend = nullptr;
	RfmCompound* current = root;
	ReflectionMember* member = nullptr;
	for (size_t i = start; i < steps.size(); ++i)
	{
		const PathStep& s = steps[i];
		if (s.index)
		{
			RfmArray* arr = dynamic_type_cast< RfmArray* >(current);
			if (!arr)
			{
				outError = L"Path index [" + std::to_wstring(s.idx) + L"] applied to a non-array member.";
				return nullptr;
			}
			if (s.idx < 0 || (uint32_t)s.idx >= arr->getMemberCount())
			{
				outError = L"Array index [" + std::to_wstring(s.idx) + L"] out of range (size " + std::to_wstring(arr->getMemberCount()) + L").";
				return nullptr;
			}
			member = arr->getMember((uint32_t)s.idx);
		}
		else
		{
			if (!current)
			{
				outError = L"Cannot resolve '" + s.name + L"'; parent is not a navigable compound.";
				return nullptr;
			}
			member = nullptr;
			for (uint32_t k = 0; k < current->getMemberCount(); ++k)
			{
				ReflectionMember* m = current->getMember(k);
				if (m->getName() && s.name == m->getName())
				{
					member = m;
					break;
				}
			}
			if (!member)
			{
				outError = L"Member '" + s.name + L"' not found.";
				return nullptr;
			}
		}

		// If more steps remain and this member is an object reference, stop so
		// the caller can recurse into the nested object's reflection.
		if (i + 1 < steps.size())
		{
			if (RfmObject* om = dynamic_type_cast< RfmObject* >(member))
			{
				outDescend = om;
				outNext = i + 1;
				return member;
			}
		}
		current = dynamic_type_cast< RfmCompound* >(member);
	}
	outNext = steps.size();
	return member;
}

/*! Build an object value from a JSON spec.
 *
 * Accepts { "$clone": "<guid|path>" } (deep clone of an existing instance) or
 * { "$type": "<typename>" } (a new instance), each with an optional
 * "set": { memberPath: value, ... } applied afterwards.
 */
Ref< ISerializable > buildObjectFromSpec(db::Database* database, const Json* spec, std::wstring& outError)
{
	if (!spec || !spec->isObject())
	{
		outError = L"Object value must be a JSON object with \"$clone\" or \"$type\".";
		return nullptr;
	}

	Ref< ISerializable > object;
	if (const Json* cloneRef = spec->getMember(L"$clone"))
	{
		const std::wstring ref = cloneRef->getString();
		const Guid g(ref);
		Ref< db::Instance > src = g.isValid() ? database->getInstance(g) : database->getInstance(ref);
		if (!src)
		{
			outError = L"$clone source not found: " + ref;
			return nullptr;
		}
		Ref< ISerializable > srcObject = src->getObject();
		if (!srcObject)
		{
			outError = L"Failed to read $clone source object: " + ref;
			return nullptr;
		}
		object = DeepClone(srcObject).create();
		if (!object)
		{
			outError = L"Failed to deep-clone source object: " + ref;
			return nullptr;
		}
	}
	else if (const Json* typeRef = spec->getMember(L"$type"))
	{
		const std::wstring typeName = typeRef->getString();
		const TypeInfo* type = TypeInfo::find(typeName.c_str());
		if (!type)
		{
			outError = L"Unknown type: " + typeName;
			return nullptr;
		}
		if (!type->isInstantiable())
		{
			outError = L"Type is not instantiable: " + typeName;
			return nullptr;
		}
		object = dynamic_type_cast< ISerializable* >(type->createInstance());
		if (!object)
		{
			outError = L"Type is not a serializable instance type: " + typeName;
			return nullptr;
		}
	}
	else
	{
		outError = L"Object value requires \"$clone\" or \"$type\".";
		return nullptr;
	}

	if (const Json* setMembers = spec->getMember(L"set"); setMembers && setMembers->isObject())
	{
		for (uint32_t i = 0; i < setMembers->getMemberCount(); ++i)
		{
			const std::wstring memberPath = setMembers->getMemberName(i);
			AlignedVector< PathStep > steps;
			if (!tokenizePath(memberPath, steps, outError))
				return nullptr;
			if (!setMemberThroughPath(database, object, steps, 0, setMembers->getMemberValue(i), outError))
				return nullptr;
		}
	}

	return object;
}

/*! Assign a JSON value to a reflected member.
 *
 * Primitive/enum leaves take raw JSON; object members take an object spec (see
 * buildObjectFromSpec) or null; array members take a JSON array whose elements
 * are object specs (object-element arrays).
 */
bool assignValue(db::Database* database, ReflectionMember* target, const Json* spec, std::wstring& outError, bool append)
{
	// Defense-in-depth: some MCP clients deliver the value as a JSON-encoded
	// string (the "value" parameter has no single JSON type). Array, map and
	// object members never take a bare string at the top level, so a string
	// here is a mis-encoded array/object spec; re-parse it into real JSON.
	// (Primitive string leaves are routed to setLeafFromJson instead, so they
	// are never reached here and keep their literal value.)
	Ref< Json > reparsed;
	if (spec && spec->isString() && (is_a< RfmArray >(target) || is_a< RfmObject >(target)))
	{
		reparsed = Json::parse(spec->getString());
		if (reparsed)
			spec = reparsed;
	}

	// RfmArray derives from RfmCompound, so test it first.
	if (auto arr = dynamic_type_cast< RfmArray* >(target))
	{
		// Grow mode: { "$grow": N } inserts N default-typed elements (for struct/enum/primitive vectors; set them afterwards by index path).
		if (spec && spec->isObject() && spec->getMember(L"$grow"))
		{
			const int32_t n = (int32_t)spec->getMember(L"$grow")->getNumber();
			if (!append)
				while (arr->getMemberCount() > 0)
					arr->removeMember(arr->getMember(0));
			for (int32_t i = 0; i < n; ++i)
				arr->insertDefault();
			return true;
		}
		// Map mode: a JSON object { key: valueSpec, ... } builds first/second pair elements (SmallMap members).
		if (spec && spec->isObject())
		{
			if (!append)
				while (arr->getMemberCount() > 0)
					arr->removeMember(arr->getMember(0));
			for (uint32_t i = 0; i < spec->getMemberCount(); ++i)
			{
				const std::wstring key = spec->getMemberName(i);
				const Json* valueSpec = spec->getMemberValue(i);
				RfmCompound* pair = new RfmCompound(L"item", nullptr);
				pair->addMember(new RfmPrimitiveWideString(L"first", key, nullptr));
				if (valueSpec && valueSpec->isObject() && (valueSpec->getMember(L"$type") || valueSpec->getMember(L"$clone")))
				{
					Ref< ISerializable > object = buildObjectFromSpec(database, valueSpec, outError);
					if (!object)
						return false;
					pair->addMember(new RfmObject(L"second", object, nullptr));
				}
				else if (valueSpec && valueSpec->isString() && Guid(valueSpec->getString()).isValid())
					pair->addMember(new RfmPrimitiveGuid(L"second", Guid(valueSpec->getString()), nullptr));
				else if (!valueSpec || valueSpec->isNull())
					pair->addMember(new RfmObject(L"second", (ISerializable*)nullptr, nullptr));
				else
				{
					outError = L"Map value for key '" + key + L"' must be an object spec ($type/$clone), guid string, or null.";
					return false;
				}
				arr->addMember(pair);
			}
			return true;
		}
		if (!spec || !spec->isArray())
		{
			outError = L"Array member '" + std::wstring(target->getName() ? target->getName() : L"") + L"' requires a JSON array value.";
			return false;
		}
		if (!append)
			while (arr->getMemberCount() > 0)
				arr->removeMember(arr->getMember(0));
		for (uint32_t i = 0; i < spec->size(); ++i)
		{
			const Json* element = spec->at(i);
			if (element && element->isObject() && (element->getMember(L"$type") || element->getMember(L"$clone")))
			{
				Ref< ISerializable > object = buildObjectFromSpec(database, element, outError);
				if (!object)
					return false;
				arr->addMember(new RfmObject(L"item", object, nullptr));
			}
			else if (element && element->isString() && Guid(element->getString()).isValid())
			{
				// Guid-string element, e.g. a resource::Id set such as physics collision groups.
				arr->addMember(new RfmPrimitiveGuid(L"item", Guid(element->getString()), nullptr));
			}
			else
			{
				outError = L"Array elements must be object specs (\"$type\"/\"$clone\") or guid strings.";
				return false;
			}
		}
		return true;
	}
	if (auto objectMember = dynamic_type_cast< RfmObject* >(target))
	{
		if (!spec || spec->isNull())
		{
			objectMember->set(nullptr);
			return true;
		}
		Ref< ISerializable > object = buildObjectFromSpec(database, spec, outError);
		if (!object)
			return false;
		objectMember->set(object);
		return true;
	}
	if (is_a< RfmCompound >(target))
	{
		outError = L"Target member is a compound (struct); set its leaf members individually (e.g. \"" + std::wstring(target->getName() ? target->getName() : L"x") + L".field\").";
		return false;
	}
	return setLeafFromJson(database, target, spec, outError);
}

}

Ref< Json > describeMember(const ReflectionMember* member, int32_t depth)
{
	Ref< Json > j = Json::createObject();
	if (member->getName())
		j->setString(L"name", member->getName());

	// RfmArray derives from RfmCompound, so it must be tested first.
	if (auto arr = dynamic_type_cast< const RfmArray* >(member))
	{
		j->setString(L"kind", L"array");
		Ref< Json > elements = Json::createArray();
		if (depth > 0)
			for (uint32_t i = 0; i < arr->getMemberCount(); ++i)
				elements->push(describeMember(arr->getMember(i), depth - 1));
		j->set(L"elements", elements);
		return j;
	}
	if (auto cmp = dynamic_type_cast< const RfmCompound* >(member))
	{
		j->setString(L"kind", L"compound");
		Ref< Json > members = Json::createArray();
		if (depth > 0)
			for (uint32_t i = 0; i < cmp->getMemberCount(); ++i)
				members->push(describeMember(cmp->getMember(i), depth - 1));
		j->set(L"members", members);
		return j;
	}
	if (auto obj = dynamic_type_cast< const RfmObject* >(member))
	{
		j->setString(L"kind", L"object");
		ISerializable* o = obj->get();
		if (!o)
		{
			j->set(L"value", Json::createNull());
			return j;
		}
		j->setString(L"type", type_name(o));
		// Reflection is shallow; recurse explicitly into the nested object.
		if (depth > 0)
		{
			Ref< Reflection > sub = Reflection::create(o);
			Ref< Json > members = Json::createArray();
			if (sub)
				for (uint32_t i = 0; i < sub->getMemberCount(); ++i)
					members->push(describeMember(sub->getMember(i), depth - 1));
			j->set(L"members", members);
		}
		return j;
	}
	if (auto e = dynamic_type_cast< const RfmEnum* >(member))
	{
		j->setString(L"kind", L"enum");
		j->set(L"value", Json::createString(e->get()));
		return j;
	}

	j->setString(L"kind", L"primitive");
	if (auto m = dynamic_type_cast< const RfmPrimitiveBoolean* >(member))
	{
		j->setString(L"type", L"bool");
		j->set(L"value", Json::createBoolean(m->get()));
	}
	else if (auto m = dynamic_type_cast< const RfmPrimitiveInt8* >(member))
	{
		j->setString(L"type", L"int8");
		j->set(L"value", Json::createNumber(m->get()));
	}
	else if (auto m = dynamic_type_cast< const RfmPrimitiveUInt8* >(member))
	{
		j->setString(L"type", L"uint8");
		j->set(L"value", Json::createNumber(m->get()));
	}
	else if (auto m = dynamic_type_cast< const RfmPrimitiveInt16* >(member))
	{
		j->setString(L"type", L"int16");
		j->set(L"value", Json::createNumber(m->get()));
	}
	else if (auto m = dynamic_type_cast< const RfmPrimitiveUInt16* >(member))
	{
		j->setString(L"type", L"uint16");
		j->set(L"value", Json::createNumber(m->get()));
	}
	else if (auto m = dynamic_type_cast< const RfmPrimitiveInt32* >(member))
	{
		j->setString(L"type", L"int32");
		j->set(L"value", Json::createNumber(m->get()));
	}
	else if (auto m = dynamic_type_cast< const RfmPrimitiveUInt32* >(member))
	{
		j->setString(L"type", L"uint32");
		j->set(L"value", Json::createNumber((int64_t)m->get()));
	}
	else if (auto m = dynamic_type_cast< const RfmPrimitiveInt64* >(member))
	{
		j->setString(L"type", L"int64");
		j->set(L"value", Json::createNumber(m->get()));
	}
	else if (auto m = dynamic_type_cast< const RfmPrimitiveUInt64* >(member))
	{
		j->setString(L"type", L"uint64");
		j->set(L"value", Json::createNumber((int64_t)m->get()));
	}
	else if (auto m = dynamic_type_cast< const RfmPrimitiveFloat* >(member))
	{
		j->setString(L"type", L"float");
		j->set(L"value", Json::createReal(m->get()));
	}
	else if (auto m = dynamic_type_cast< const RfmPrimitiveDouble* >(member))
	{
		j->setString(L"type", L"double");
		j->set(L"value", Json::createReal(m->get()));
	}
	else if (auto m = dynamic_type_cast< const RfmPrimitiveWideString* >(member))
	{
		j->setString(L"type", L"wstring");
		j->set(L"value", Json::createString(m->get()));
	}
	else if (auto m = dynamic_type_cast< const RfmPrimitiveString* >(member))
	{
		j->setString(L"type", L"string");
		j->set(L"value", Json::createString(widen(m->get())));
	}
	else if (auto m = dynamic_type_cast< const RfmPrimitiveGuid* >(member))
	{
		j->setString(L"type", L"guid");
		j->set(L"value", Json::createString(m->get().format()));
	}
	else if (auto m = dynamic_type_cast< const RfmPrimitivePath* >(member))
	{
		j->setString(L"type", L"path");
		j->set(L"value", Json::createString(m->get().getOriginal()));
	}
	else if (auto m = dynamic_type_cast< const RfmPrimitiveScalar* >(member))
	{
		j->setString(L"type", L"scalar");
		j->set(L"value", Json::createReal((float)m->get()));
	}
	else if (auto m = dynamic_type_cast< const RfmPrimitiveColor4ub* >(member))
	{
		j->setString(L"type", L"color4ub");
		const Color4ub c = m->get();
		const float v[4] = { (float)c.r, (float)c.g, (float)c.b, (float)c.a };
		j->set(L"value", floatArray(v, 4));
	}
	else if (auto m = dynamic_type_cast< const RfmPrimitiveColor4f* >(member))
	{
		j->setString(L"type", L"color4f");
		float v[4];
		m->get().storeUnaligned(v);
		j->set(L"value", floatArray(v, 4));
	}
	else if (auto m = dynamic_type_cast< const RfmPrimitiveVector2* >(member))
	{
		j->setString(L"type", L"vector2");
		const Vector2 v = m->get();
		const float a[2] = { v.x, v.y };
		j->set(L"value", floatArray(a, 2));
	}
	else if (auto m = dynamic_type_cast< const RfmPrimitiveVector4* >(member))
	{
		j->setString(L"type", L"vector4");
		float v[4];
		m->get().storeUnaligned(v);
		j->set(L"value", floatArray(v, 4));
	}
	else if (auto m = dynamic_type_cast< const RfmPrimitiveQuaternion* >(member))
	{
		j->setString(L"type", L"quaternion");
		float v[4];
		m->get().e.storeUnaligned(v);
		j->set(L"value", floatArray(v, 4));
	}
	else if (auto m = dynamic_type_cast< const RfmPrimitiveMatrix44* >(member))
	{
		j->setString(L"type", L"matrix44");
		float v[16];
		m->get().storeUnaligned(v);
		j->set(L"value", floatArray(v, 16));
	}
	else
	{
		j->setString(L"type", type_name(member));
		j->set(L"value", Json::createNull());
	}
	return j;
}

bool tokenizePath(const std::wstring& path, AlignedVector< PathStep >& outSteps, std::wstring& outError)
{
	const size_t n = path.length();
	size_t i = 0;
	while (i < n)
	{
		std::wstring name;
		while (i < n && path[i] != L'.' && path[i] != L'[')
			name += path[i++];
		if (!name.empty())
		{
			PathStep s;
			s.index = false;
			s.name = name;
			s.idx = 0;
			outSteps.push_back(s);
		}
		while (i < n && path[i] == L'[')
		{
			++i;
			std::wstring idxText;
			while (i < n && path[i] != L']')
				idxText += path[i++];
			if (i >= n || path[i] != L']')
			{
				outError = L"Malformed index in path.";
				return false;
			}
			++i;
			PathStep s;
			s.index = true;
			s.idx = (int32_t)wcstol(idxText.c_str(), nullptr, 10);
			outSteps.push_back(s);
		}
		if (i < n && path[i] == L'.')
			++i;
	}
	if (outSteps.empty())
	{
		outError = L"Empty member path.";
		return false;
	}
	return true;
}

bool setMemberThroughPath(db::Database* database, ISerializable* object, const AlignedVector< PathStep >& steps, size_t start, const Json* spec, std::wstring& outError, bool append)
{
	Ref< Reflection > reflection = Reflection::create(object);
	if (!reflection)
	{
		outError = L"Failed to create reflection of object.";
		return false;
	}

	RfmObject* descend = nullptr;
	size_t next = 0;
	ReflectionMember* member = resolveSteps(reflection, steps, start, descend, next, outError);
	if (!member)
		return false;

	if (descend)
	{
		ISerializable* nested = descend->get();
		if (!nested)
		{
			outError = L"Cannot descend into a null object reference along the path.";
			return false;
		}
		if (!setMemberThroughPath(database, nested, steps, next, spec, outError, append))
			return false;
		descend->set(nested);
		return reflection->apply(object);
	}

	if (!assignValue(database, member, spec, outError, append))
		return false;
	return reflection->apply(object);
}

Ref< Json > describeMemberAtPath(ISerializable* object, const AlignedVector< PathStep >& steps, size_t start)
{
	Ref< Reflection > reflection = Reflection::create(object);
	if (!reflection)
		return nullptr;
	RfmObject* descend = nullptr;
	size_t next = 0;
	std::wstring ignore;
	ReflectionMember* member = resolveSteps(reflection, steps, start, descend, next, ignore);
	if (!member)
		return nullptr;
	if (descend)
	{
		ISerializable* nested = descend->get();
		if (!nested)
			return nullptr;
		return describeMemberAtPath(nested, steps, next);
	}
	return describeMember(member, c_defaultMaxDepth);
}

}
