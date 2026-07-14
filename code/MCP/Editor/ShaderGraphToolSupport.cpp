/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "MCP/Editor/ShaderGraphToolSupport.h"

#include "Core/Containers/AlignedVector.h"
#include "Core/Guid.h"
#include "Core/Math/Color4f.h"
#include "Core/Math/Vector4.h"
#include "Core/Misc/String.h"
#include "Core/Reflection/Reflection.h"
#include "Core/Reflection/ReflectionMember.h"
#include "Core/Reflection/RfmArray.h"
#include "Core/Reflection/RfmCompound.h"
#include "Core/Reflection/RfmEnum.h"
#include "Core/Reflection/RfmPrimitive.h"
#include "Core/Rtti/TypeInfo.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "MCP/Editor/NodeEmitterCapture.h"
#include "MCP/Editor/Json.h"
#include "Render/Editor/Edge.h"
#include "Render/Editor/InputPin.h"
#include "Render/Editor/Node.h"
#include "Render/Editor/OutputPin.h"
#include "Render/Editor/Shader/Algorithms/ShaderGraphTypePropagation.h"
#include "Render/Editor/Shader/Algorithms/ShaderGraphValidator.h"
#include "Render/Editor/Shader/External.h"
#include "Render/Editor/Shader/FragmentLinker.h"
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/ParameterLinker.h"
#include "Render/Editor/Shader/PinType.h"
#include "Render/Editor/Shader/Script.h"
#include "Render/Editor/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/UniformLinker.h"
#include "Render/Types.h"

#include <functional>
#include <map>

namespace traktor::mcp
{
namespace
{

/*! Name of a resolved pin type. */
std::wstring pinTypeName(render::PinType type)
{
	switch (type)
	{
	case render::PinType::Void:
		return L"Void";
	case render::PinType::Scalar1:
		return L"Scalar1";
	case render::PinType::Scalar2:
		return L"Scalar2";
	case render::PinType::Scalar3:
		return L"Scalar3";
	case render::PinType::Scalar4:
		return L"Scalar4";
	case render::PinType::Matrix:
		return L"Matrix";
	case render::PinType::Texture2D:
		return L"Texture2D";
	case render::PinType::Texture3D:
		return L"Texture3D";
	case render::PinType::TextureCube:
		return L"TextureCube";
	case render::PinType::StructBuffer:
		return L"StructBuffer";
	case render::PinType::Image2D:
		return L"Image2D";
	case render::PinType::Image3D:
		return L"Image3D";
	case render::PinType::ImageCube:
		return L"ImageCube";
	case render::PinType::State:
		return L"State";
	case render::PinType::Bundle:
		return L"Bundle";
	case render::PinType::Array:
		return L"Array";
	case render::PinType::Struct:
		return L"Struct";
	default:
		return L"Void";
	}
}

/*! Map a Script output pin's ParameterType to/from its IR string name. */
render::ParameterType parseParameterType(const std::wstring& name, render::ParameterType defaultType)
{
	if (name == L"Scalar")
		return render::ParameterType::Scalar;
	if (name == L"Vector")
		return render::ParameterType::Vector;
	if (name == L"Matrix")
		return render::ParameterType::Matrix;
	if (name == L"Texture2D")
		return render::ParameterType::Texture2D;
	if (name == L"Texture3D")
		return render::ParameterType::Texture3D;
	if (name == L"TextureCube")
		return render::ParameterType::TextureCube;
	if (name == L"StructBuffer")
		return render::ParameterType::StructBuffer;
	if (name == L"Image2D")
		return render::ParameterType::Image2D;
	if (name == L"Image3D")
		return render::ParameterType::Image3D;
	if (name == L"ImageCube")
		return render::ParameterType::ImageCube;
	if (name == L"AccelerationStructure")
		return render::ParameterType::AccelerationStructure;
	return defaultType;
}

std::wstring parameterTypeName(render::ParameterType type)
{
	switch (type)
	{
	case render::ParameterType::Vector:
		return L"Vector";
	case render::ParameterType::Matrix:
		return L"Matrix";
	case render::ParameterType::Texture2D:
		return L"Texture2D";
	case render::ParameterType::Texture3D:
		return L"Texture3D";
	case render::ParameterType::TextureCube:
		return L"TextureCube";
	case render::ParameterType::StructBuffer:
		return L"StructBuffer";
	case render::ParameterType::Image2D:
		return L"Image2D";
	case render::ParameterType::Image3D:
		return L"Image3D";
	case render::ParameterType::ImageCube:
		return L"ImageCube";
	case render::ParameterType::AccelerationStructure:
		return L"AccelerationStructure";
	default:
		return L"Scalar";
	}
}

/*! Map a Script domain to/from its IR string name. */
render::Script::Domain parseScriptDomain(const std::wstring& name)
{
	if (name == L"Vertex")
		return render::Script::Vertex;
	if (name == L"Pixel")
		return render::Script::Pixel;
	if (name == L"Compute")
		return render::Script::Compute;
	return render::Script::Undefined;
}

std::wstring scriptDomainName(render::Script::Domain domain)
{
	switch (domain)
	{
	case render::Script::Vertex:
		return L"Vertex";
	case render::Script::Pixel:
		return L"Pixel";
	case render::Script::Compute:
		return L"Compute";
	default:
		return L"Undefined";
	}
}

/*! Base Node members that are surfaced as first-class IR fields, not properties. */
bool isReservedMember(const std::wstring& name)
{
	return name == L"id" || name == L"comment" || name == L"position" || name == L"fragmentGuid";
}

/*! Convert a primitive or enum reflection member to a JSON value (null if unsupported). */
Ref< Json > primitiveMemberToJson(const ReflectionMember* member)
{
	if (auto m = dynamic_type_cast< const RfmPrimitiveBoolean* >(member))
		return Json::createBoolean(m->get());
	if (auto m = dynamic_type_cast< const RfmPrimitiveInt8* >(member))
		return Json::createNumber(m->get());
	if (auto m = dynamic_type_cast< const RfmPrimitiveUInt8* >(member))
		return Json::createNumber(m->get());
	if (auto m = dynamic_type_cast< const RfmPrimitiveInt16* >(member))
		return Json::createNumber(m->get());
	if (auto m = dynamic_type_cast< const RfmPrimitiveUInt16* >(member))
		return Json::createNumber(m->get());
	if (auto m = dynamic_type_cast< const RfmPrimitiveInt32* >(member))
		return Json::createNumber(m->get());
	if (auto m = dynamic_type_cast< const RfmPrimitiveUInt32* >(member))
		return Json::createNumber(m->get());
	if (auto m = dynamic_type_cast< const RfmPrimitiveInt64* >(member))
		return Json::createNumber(m->get());
	if (auto m = dynamic_type_cast< const RfmPrimitiveUInt64* >(member))
		return Json::createNumber((int64_t)m->get());
	if (auto m = dynamic_type_cast< const RfmPrimitiveFloat* >(member))
		return Json::createReal(m->get());
	if (auto m = dynamic_type_cast< const RfmPrimitiveDouble* >(member))
		return Json::createReal(m->get());
	if (auto m = dynamic_type_cast< const RfmPrimitiveWideString* >(member))
		return Json::createString(m->get());
	if (auto m = dynamic_type_cast< const RfmPrimitiveGuid* >(member))
		return Json::createString(m->get().format());
	// Enums (incl. bitmasks) reflect as RfmEnum; emit the key string so render/
	// sampler state (blend, depth, cull, filters, ...) survives a round-trip.
	if (auto m = dynamic_type_cast< const RfmEnum* >(member))
		return Json::createString(m->get());
	return nullptr;
}

/*! Convert a reflection member to JSON: primitive, enum, or nested compound
 * (render/sampler state, bitmasks). Arrays are left to node-specific paths; null if unsupported. */
Ref< Json > memberToJson(const ReflectionMember* member)
{
	// RfmArray derives from RfmCompound, so exclude arrays before the compound case.
	if (dynamic_type_cast< const RfmArray* >(member))
		return nullptr;
	if (auto compound = dynamic_type_cast< const RfmCompound* >(member))
	{
		Ref< Json > object = Json::createObject();
		for (uint32_t i = 0; i < compound->getMemberCount(); ++i)
		{
			const ReflectionMember* sub = compound->getMember(i);
			if (sub->getName() == nullptr)
				continue;
			Ref< Json > value = memberToJson(sub);
			if (value)
				object->set(sub->getName(), value);
		}
		return object;
	}
	return primitiveMemberToJson(member);
}

/*! Reflect a node's primitive and enum value members into a JSON "properties" object. */
Ref< Json > reflectNodeProperties(const render::Node* node)
{
	Ref< Reflection > reflection = Reflection::create(node);
	if (!reflection)
		return nullptr;

	Ref< Json > properties = Json::createObject();
	for (uint32_t i = 0; i < reflection->getMemberCount(); ++i)
	{
		const ReflectionMember* member = reflection->getMember(i);
		const std::wstring name = member->getName();
		if (isReservedMember(name))
			continue;
		Ref< Json > value = memberToJson(member);
		if (value)
			properties->set(name, value);
	}
	return properties;
}

/*! Set a primitive or enum reflection member from a JSON value; false if type unsupported. */
bool setPrimitiveMemberFromJson(ReflectionMember* member, const Json* value)
{
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
	if (auto m = dynamic_type_cast< RfmPrimitiveGuid* >(member))
	{
		m->set(Guid(value->getString()));
		return true;
	}
	if (auto m = dynamic_type_cast< RfmEnum* >(member))
	{
		m->set(value->getString());
		return true;
	}
	return false;
}

/*! Apply a JSON value to a reflection member, recursing into nested compounds
 * (render/sampler state, bitmasks). Arrays are left to node-specific paths. */
bool applyMemberFromJson(ReflectionMember* member, const Json* value)
{
	if (dynamic_type_cast< RfmArray* >(member))
		return false;
	if (auto compound = dynamic_type_cast< RfmCompound* >(member))
	{
		if (!value || !value->isObject())
			return false;
		for (uint32_t i = 0; i < compound->getMemberCount(); ++i)
		{
			ReflectionMember* sub = compound->getMember(i);
			if (sub->getName() == nullptr)
				continue;
			if (const Json* subValue = value->getMember(sub->getName()))
				applyMemberFromJson(sub, subValue);
		}
		return true;
	}
	return setPrimitiveMemberFromJson(member, value);
}

/*! Apply a JSON "properties" object onto a node via reflection. */
void applyNodeProperties(render::Node* node, const Json* properties, Json* warnings)
{
	if (!properties || !properties->isObject())
		return;

	Ref< Reflection > reflection = Reflection::create(node);
	if (!reflection)
		return;

	for (uint32_t i = 0; i < properties->getMemberCount(); ++i)
	{
		const std::wstring name = properties->getMemberName(i);
		const Json* value = properties->getMemberValue(i);
		if (isReservedMember(name))
			continue;

		ReflectionMember* member = nullptr;
		for (uint32_t j = 0; j < reflection->getMemberCount(); ++j)
		{
			if (name == reflection->getMember(j)->getName())
			{
				member = reflection->getMember(j);
				break;
			}
		}

		if (!member)
			warnings->push(Json::createString(L"Unknown property '" + name + L"' on node type " + shortTypeName(type_name(node))));
		else if (!applyMemberFromJson(member, value))
			warnings->push(Json::createString(L"Unsupported property type for '" + name + L"' (only scalars, strings and guids are settable)."));
	}

	reflection->apply(node);
}

/*! Read a pin name from an IR pin entry (a string, or a { "name": ... } object). */
std::wstring pinName(const Json* pin)
{
	if (!pin)
		return L"";
	if (pin->isString())
		return pin->getString();
	if (pin->isObject() && pin->getMember(L"name"))
		return pin->getMember(L"name")->getString();
	return L"";
}

/*! Read an optional pin type from an IR pin entry ({ "type": ... } object). */
std::wstring pinTypeName(const Json* pin)
{
	if (pin && pin->isObject() && pin->getMember(L"type"))
		return pin->getMember(L"type")->getString();
	return L"";
}

/*! Instantiate and configure a node from its IR description. */
Ref< render::Node > nodeFromIr(const Json* nodeJson, std::wstring& outError, Json* warnings)
{
	const std::wstring type = nodeJson->getMember(L"type") ? nodeJson->getMember(L"type")->getString() : L"";
	if (type.empty())
	{
		outError = L"Node is missing a \"type\".";
		return nullptr;
	}

	const std::wstring fullType = L"traktor.render." + type;
	Ref< render::Node > node = dynamic_type_cast< render::Node* >(TypeInfo::createInstance(fullType.c_str()));
	if (!node)
	{
		outError = L"Unknown or non-instantiable node type: " + type;
		return nullptr;
	}

	if (isDeprecatedNode(type))
		warnings->push(Json::createString(L"Node type '" + type + L"' is deprecated and should not be used in new graphs."));

	Guid id;
	if (nodeJson->getMember(L"guid"))
	{
		const Guid g(nodeJson->getMember(L"guid")->getString());
		if (g.isValid())
			id = g;
	}
	node->setId(id.isValid() ? id : Guid::create());

	if (const Json* position = nodeJson->getMember(L"position"); position && position->isArray() && position->size() >= 2)
		node->setPosition(std::make_pair((int)position->at(0)->getNumber(), (int)position->at(1)->getNumber()));

	if (nodeJson->getMember(L"comment"))
		node->setComment(nodeJson->getMember(L"comment")->getString());

	applyNodeProperties(node, nodeJson->getMember(L"properties"), warnings);

	// External nodes have dynamic pins and a fragment reference; both come from
	// the IR (the pins should match the referenced fragment's ports).
	if (auto external = dynamic_type_cast< render::External* >(node))
	{
		if (nodeJson->getMember(L"fragment"))
			external->setFragmentGuid(Guid(nodeJson->getMember(L"fragment")->getString()));

		if (const Json* inputs = nodeJson->getMember(L"inputs"); inputs && inputs->isArray())
		{
			for (uint32_t i = 0; i < inputs->size(); ++i)
			{
				const Json* pin = inputs->at(i);
				const std::wstring name = pinName(pin);
				const bool optional = pin && pin->isObject() && pin->getMember(L"optional") && pin->getMember(L"optional")->getBoolean();
				if (!name.empty())
					external->createInputPin(Guid::create(), name, optional);
			}
		}
		if (const Json* outputs = nodeJson->getMember(L"outputs"); outputs && outputs->isArray())
		{
			for (uint32_t i = 0; i < outputs->size(); ++i)
			{
				const std::wstring name = pinName(outputs->at(i));
				if (!name.empty())
					external->createOutputPin(Guid::create(), name);
			}
		}
	}
	// Script nodes also carry dynamic pins, but their output pins are typed
	// (ParameterType) and they have a domain; create both from the IR.
	else if (auto script = dynamic_type_cast< render::Script* >(node))
	{
		if (const Json* domain = nodeJson->getMember(L"domain"))
			script->setDomain(parseScriptDomain(domain->getString()));

		// Restore #include references (guids of ShaderModule instances); not a
		// primitive member, so they are carried in a dedicated "includes" field.
		if (const Json* includes = nodeJson->getMember(L"includes"); includes && includes->isArray())
		{
			AlignedVector< Guid > includeIds;
			for (uint32_t i = 0; i < includes->size(); ++i)
			{
				const Guid g(includes->at(i)->getString());
				if (g.isValid())
					includeIds.push_back(g);
			}
			script->setIncludes(includeIds);
		}

		if (const Json* inputs = nodeJson->getMember(L"inputs"); inputs && inputs->isArray())
		{
			for (uint32_t i = 0; i < inputs->size(); ++i)
			{
				const std::wstring name = pinName(inputs->at(i));
				if (!name.empty())
					script->addInputPin(Guid::create(), name);
			}
		}
		if (const Json* outputs = nodeJson->getMember(L"outputs"); outputs && outputs->isArray())
		{
			for (uint32_t i = 0; i < outputs->size(); ++i)
			{
				const Json* pin = outputs->at(i);
				const std::wstring name = pinName(pin);
				if (!name.empty())
					script->addOutputPin(Guid::create(), name, parseParameterType(pinTypeName(pin), render::ParameterType::Scalar));
			}
		}
	}
	// BundleSplit derives its output pins from a serialized channel-name list
	// (its single "Input" pin is fixed). The names are not reflected as a
	// primitive property, so restore them from the IR's output pins to keep
	// edges that reference them alive through a round-trip.
	else if (auto bundleSplit = dynamic_type_cast< render::BundleSplit* >(node))
	{
		AlignedVector< std::wstring > names;
		if (const Json* outputs = nodeJson->getMember(L"outputs"); outputs && outputs->isArray())
		{
			for (uint32_t i = 0; i < outputs->size(); ++i)
			{
				const std::wstring name = pinName(outputs->at(i));
				if (!name.empty())
					names.push_back(name);
			}
		}
		bundleSplit->setNames(names);
	}
	// BundleUnite's named input pins are its channel list; the leading "Input"
	// pin and the single "Output" pin are fixed, so restore everything else.
	else if (auto bundleUnite = dynamic_type_cast< render::BundleUnite* >(node))
	{
		AlignedVector< std::wstring > names;
		if (const Json* inputs = nodeJson->getMember(L"inputs"); inputs && inputs->isArray())
		{
			for (uint32_t i = 0; i < inputs->size(); ++i)
			{
				const std::wstring name = pinName(inputs->at(i));
				if (!name.empty() && name != L"Input")
					names.push_back(name);
			}
		}
		bundleUnite->setNames(names);
	}
	// Color/Vector node values are composites (Color4f/Vector4) that reflection
	// cannot set; restore the components from the IR's top-level array (the
	// "linear" flag on Color is already applied via reflected properties above).
	else if (auto colorNode = dynamic_type_cast< render::Color* >(node))
	{
		if (const Json* color = nodeJson->getMember(L"color"); color && color->isArray() && color->size() >= 4)
			colorNode->setColor(Color4f(
				(float)color->at(0)->getReal(),
				(float)color->at(1)->getReal(),
				(float)color->at(2)->getReal(),
				(float)color->at(3)->getReal()));
	}
	else if (auto vectorNode = dynamic_type_cast< render::Vector* >(node))
	{
		if (const Json* value = nodeJson->getMember(L"value"); value && value->isArray() && value->size() >= 4)
			vectorNode->set(Vector4(
				(float)value->at(0)->getReal(),
				(float)value->at(1)->getReal(),
				(float)value->at(2)->getReal(),
				(float)value->at(3)->getReal()));
	}

	return node;
}

}

std::wstring shortTypeName(const std::wstring& fullName)
{
	const size_t p = fullName.find_last_of(L'.');
	return (p == std::wstring::npos) ? fullName : fullName.substr(p + 1);
}

bool isDeprecatedNode(const std::wstring& shortType)
{
	const std::set< std::wstring >& deprecated = nodeDeprecatedTypes();
	return deprecated.find(shortType) != deprecated.end();
}

Ref< render::ShaderGraph > loadShaderGraph(db::Database* database, const Json* arguments, Guid& outGuid, std::wstring& outName, std::wstring& outError)
{
	if (!arguments)
	{
		outError = L"Missing arguments; expected \"guid\" or \"path\".";
		return nullptr;
	}

	Ref< db::Instance > instance;
	const std::wstring guidText = arguments->getMember(L"guid") ? arguments->getMember(L"guid")->getString() : L"";
	if (!guidText.empty())
	{
		const Guid guid(guidText);
		if (!guid.isValid())
		{
			outError = L"Invalid guid: " + guidText;
			return nullptr;
		}
		instance = database->getInstance(guid);
	}
	else if (arguments->getMember(L"path"))
		instance = database->getInstance(arguments->getMember(L"path")->getString());

	if (!instance)
	{
		outError = L"No instance found for the given guid/path.";
		return nullptr;
	}

	outGuid = instance->getGuid();
	outName = instance->getName();

	Ref< render::ShaderGraph > shaderGraph = instance->getObject< render::ShaderGraph >();
	if (!shaderGraph)
	{
		outError = L"Instance is not a ShaderGraph (primary type: " + instance->getPrimaryTypeName() + L").";
		return nullptr;
	}

	// Link parameter declarations in resolved shader graph.
	const auto parameterDeclarationReader = [&](const Guid& declarationId) -> render::ParameterLinker::named_decl_t {
		Ref< db::Instance > declarationInstance = database->getInstance(declarationId);
		if (declarationInstance != nullptr)
			return { declarationInstance->getName(), declarationInstance->getObject() };
		else
			return { L"", nullptr };
	};
	render::ParameterLinker(parameterDeclarationReader).resolve(shaderGraph);

	const auto uniformDeclarationReader = [&](const Guid& declarationId) -> render::UniformLinker::named_decl_t {
		Ref< db::Instance > declarationInstance = database->getInstance(declarationId);
		if (declarationInstance != nullptr)
			return { declarationInstance->getName(), declarationInstance->getObject() };
		else
			return { L"", nullptr };
	};
	render::UniformLinker(uniformDeclarationReader).resolve(shaderGraph);

	return shaderGraph;
}

Ref< render::ShaderGraph > resolveShaderGraph(db::Database* database, const render::ShaderGraph* shaderGraph, const Guid& guid)
{
	render::FragmentLinker linker([database](const Guid& fragmentGuid) -> Ref< const render::ShaderGraph > {
		return database->getObjectReadOnly< render::ShaderGraph >(fragmentGuid);
	});
	Ref< render::ShaderGraph > mutableShaderGraph = linker.resolve(shaderGraph, true, &guid);
	if (!mutableShaderGraph)
		return nullptr;

	// Link parameter declarations in resolved shader graph.
	const auto parameterDeclarationReader = [&](const Guid& declarationId) -> render::ParameterLinker::named_decl_t {
		Ref< db::Instance > declarationInstance = database->getInstance(declarationId);
		if (declarationInstance != nullptr)
			return { declarationInstance->getName(), declarationInstance->getObject() };
		else
			return { L"", nullptr };
	};
	render::ParameterLinker(parameterDeclarationReader).resolve(mutableShaderGraph);

	const auto uniformDeclarationReader = [&](const Guid& declarationId) -> render::UniformLinker::named_decl_t {
		Ref< db::Instance > declarationInstance = database->getInstance(declarationId);
		if (declarationInstance != nullptr)
			return { declarationInstance->getName(), declarationInstance->getObject() };
		else
			return { L"", nullptr };
	};
	render::UniformLinker(uniformDeclarationReader).resolve(mutableShaderGraph);

	return mutableShaderGraph;
}

Ref< Json > buildGraphIr(const render::ShaderGraph* shaderGraph, db::Database* database, const render::ShaderGraphTypePropagation* typePropagation)
{
	const RefArray< render::Node >& nodes = shaderGraph->getNodes();

	std::map< const render::Node*, int32_t > nodeIndex;
	Ref< Json > nodesJson = Json::createArray();

	for (int32_t i = 0; i < (int32_t)nodes.size(); ++i)
	{
		const render::Node* node = nodes[i];
		nodeIndex[node] = i;

		Ref< Json > nodeJson = Json::createObject();
		nodeJson->setString(L"id", L"n" + toString(i));
		nodeJson->setString(L"guid", node->getId().format());
		const std::wstring typeName = shortTypeName(type_name(node));
		nodeJson->setString(L"type", typeName);
		if (isDeprecatedNode(typeName))
			nodeJson->setBoolean(L"deprecated", true);

		const std::wstring info = node->getInformation();
		if (!info.empty())
			nodeJson->setString(L"info", info);

		const std::wstring comment = node->getComment();
		if (!comment.empty())
			nodeJson->setString(L"comment", comment);

		Ref< Json > position = Json::createArray();
		position->push(Json::createNumber(node->getPosition().first));
		position->push(Json::createNumber(node->getPosition().second));
		nodeJson->set(L"position", position);

		Ref< Json > inputs = Json::createArray();
		for (int32_t p = 0; p < node->getInputPinCount(); ++p)
		{
			const render::InputPin* inputPin = node->getInputPin(p);
			Ref< Json > pin = Json::createObject();
			pin->setString(L"name", inputPin->getName());
			if (inputPin->isOptional())
				pin->setBoolean(L"optional", true);
			if (typePropagation && typePropagation->valid())
				pin->setString(L"type", pinTypeName(typePropagation->evaluate(inputPin)));
			inputs->push(pin);
		}
		nodeJson->set(L"inputs", inputs);

		Ref< Json > outputs = Json::createArray();
		for (int32_t p = 0; p < node->getOutputPinCount(); ++p)
		{
			const render::OutputPin* outputPin = node->getOutputPin(p);
			Ref< Json > pin = Json::createObject();
			pin->setString(L"name", outputPin->getName());
			if (typePropagation && typePropagation->valid())
				pin->setString(L"type", pinTypeName(typePropagation->evaluate(outputPin)));
			outputs->push(pin);
		}
		nodeJson->set(L"outputs", outputs);

		if (auto external = dynamic_type_cast< const render::External* >(node))
		{
			const Guid& fragmentGuid = external->getFragmentGuid();
			nodeJson->setString(L"fragment", fragmentGuid.format());
			if (database)
			{
				Ref< db::Instance > fragmentInstance = database->getInstance(fragmentGuid);
				if (fragmentInstance)
					nodeJson->setString(L"fragmentName", fragmentInstance->getName());
			}
		}

		// Script nodes have dynamic, typed output pins and a domain; surface the
		// per-output type and the domain so the graph round-trips through the IR.
		if (auto script = dynamic_type_cast< const render::Script* >(node))
		{
			Ref< Json > typedOutputs = Json::createArray();
			for (int32_t p = 0; p < node->getOutputPinCount(); ++p)
			{
				Ref< Json > pin = Json::createObject();
				pin->setString(L"name", node->getOutputPin(p)->getName());
				pin->setString(L"type", parameterTypeName(script->getOutputPinType(p)));
				typedOutputs->push(pin);
			}
			nodeJson->set(L"outputs", typedOutputs);
			nodeJson->setString(L"domain", scriptDomainName(script->getDomain()));

			// #include references are guids of ShaderModule instances; not a
			// primitive member, so surface them explicitly to survive a round-trip.
			const AlignedVector< Guid >& includes = script->getIncludes();
			if (!includes.empty())
			{
				Ref< Json > includesJson = Json::createArray();
				for (const auto& include : includes)
					includesJson->push(Json::createString(include.format()));
				nodeJson->set(L"includes", includesJson);
			}
		}

		// Color/Vector nodes store their value in a composite (Color4f/Vector4)
		// that reflection does not surface as a primitive; emit the components as
		// a top-level array so the value survives a round-trip (the boolean
		// "linear" flag on Color still travels through reflected properties).
		if (auto colorNode = dynamic_type_cast< const render::Color* >(node))
		{
			float c[4];
			colorNode->getColor().storeUnaligned(c);
			Ref< Json > color = Json::createArray();
			for (int32_t i = 0; i < 4; ++i)
				color->push(Json::createReal(c[i]));
			nodeJson->set(L"color", color);
		}
		else if (auto vectorNode = dynamic_type_cast< const render::Vector* >(node))
		{
			float v[4];
			vectorNode->get().storeUnaligned(v);
			Ref< Json > value = Json::createArray();
			for (int32_t i = 0; i < 4; ++i)
				value->push(Json::createReal(v[i]));
			nodeJson->set(L"value", value);
		}

		Ref< Json > properties = reflectNodeProperties(node);
		if (properties && properties->getMemberCount() > 0)
			nodeJson->set(L"properties", properties);

		nodesJson->push(nodeJson);
	}

	Ref< Json > edgesJson = Json::createArray();
	for (auto edge : shaderGraph->getEdges())
	{
		const render::OutputPin* source = edge->getSource();
		const render::InputPin* destination = edge->getDestination();
		if (!source || !destination)
			continue;

		const auto si = nodeIndex.find(source->getNode());
		const auto di = nodeIndex.find(destination->getNode());
		if (si == nodeIndex.end() || di == nodeIndex.end())
			continue;

		Ref< Json > from = Json::createObject();
		from->setString(L"node", L"n" + toString(si->second));
		from->setString(L"pin", source->getName());

		Ref< Json > to = Json::createObject();
		to->setString(L"node", L"n" + toString(di->second));
		to->setString(L"pin", destination->getName());

		Ref< Json > edgeJson = Json::createObject();
		edgeJson->set(L"from", from);
		edgeJson->set(L"to", to);
		edgesJson->push(edgeJson);
	}

	Ref< Json > ir = Json::createObject();
	ir->set(L"nodes", nodesJson);
	ir->set(L"edges", edgesJson);
	return ir;
}

Ref< Json > targetSchema(bool includeResolve)
{
	Ref< Json > guidProperty = Json::createObject();
	guidProperty->setString(L"type", L"string");
	guidProperty->setString(L"description", L"Guid of the shader graph instance (preferred).");

	Ref< Json > pathProperty = Json::createObject();
	pathProperty->setString(L"type", L"string");
	pathProperty->setString(L"description", L"Database path of the shader graph instance (alternative to guid).");

	Ref< Json > properties = Json::createObject();
	properties->set(L"guid", guidProperty);
	properties->set(L"path", pathProperty);

	if (includeResolve)
	{
		Ref< Json > resolveProperty = Json::createObject();
		resolveProperty->setString(L"type", L"boolean");
		resolveProperty->setString(L"description", L"If true, inline (resolve) all referenced External fragments before returning.");
		properties->set(L"resolve", resolveProperty);
	}

	Ref< Json > schema = Json::createObject();
	schema->setString(L"type", L"object");
	schema->set(L"properties", properties);
	schema->set(L"required", Json::createArray());
	return schema;
}

Ref< render::ShaderGraph > graphFromIr(db::Database* database, const Json* graphJson, std::wstring& outError, Json* warnings)
{
	if (!graphJson || !graphJson->isObject())
	{
		outError = L"Missing \"graph\" object.";
		return nullptr;
	}

	const Json* nodes = graphJson->getMember(L"nodes");
	if (!nodes || !nodes->isArray())
	{
		outError = L"\"graph.nodes\" must be an array.";
		return nullptr;
	}

	Ref< render::ShaderGraph > graph = new render::ShaderGraph();
	std::map< std::wstring, render::Node* > idMap;

	for (uint32_t i = 0; i < nodes->size(); ++i)
	{
		const Json* nodeJson = nodes->at(i);
		std::wstring error;
		Ref< render::Node > node = nodeFromIr(nodeJson, error, warnings);
		if (!node)
		{
			outError = error;
			return nullptr;
		}
		const std::wstring id = nodeJson->getMember(L"id") ? nodeJson->getMember(L"id")->getString() : (L"n" + toString(i));
		idMap[id] = node;
		graph->addNode(node);
	}

	if (const Json* edges = graphJson->getMember(L"edges"); edges && edges->isArray())
	{
		for (uint32_t i = 0; i < edges->size(); ++i)
		{
			const Json* edge = edges->at(i);
			const Json* from = edge->getMember(L"from");
			const Json* to = edge->getMember(L"to");
			if (!from || !to)
			{
				warnings->push(Json::createString(L"Edge missing \"from\"/\"to\"; skipped."));
				continue;
			}

			const std::wstring fromId = from->getMember(L"node") ? from->getMember(L"node")->getString() : L"";
			const std::wstring fromPin = from->getMember(L"pin") ? from->getMember(L"pin")->getString() : L"";
			const std::wstring toId = to->getMember(L"node") ? to->getMember(L"node")->getString() : L"";
			const std::wstring toPin = to->getMember(L"pin") ? to->getMember(L"pin")->getString() : L"";

			const auto fromIt = idMap.find(fromId);
			const auto toIt = idMap.find(toId);
			if (fromIt == idMap.end() || toIt == idMap.end())
			{
				warnings->push(Json::createString(L"Edge references unknown node id (" + fromId + L" -> " + toId + L"); skipped."));
				continue;
			}

			const render::OutputPin* outputPin = fromIt->second->findOutputPin(fromPin);
			const render::InputPin* inputPin = toIt->second->findInputPin(toPin);
			if (!outputPin)
			{
				warnings->push(Json::createString(L"Unknown output pin \"" + fromPin + L"\" on node " + fromId + L"; edge skipped."));
				continue;
			}
			if (!inputPin)
			{
				warnings->push(Json::createString(L"Unknown input pin \"" + toPin + L"\" on node " + toId + L"; edge skipped."));
				continue;
			}

			graph->addEdge(new render::Edge(outputPin, inputPin));
		}
	}

	// Link parameter declarations in resolved shader graph.
	const auto parameterDeclarationReader = [&](const Guid& declarationId) -> render::ParameterLinker::named_decl_t {
		Ref< db::Instance > declarationInstance = database->getInstance(declarationId);
		if (declarationInstance != nullptr)
			return { declarationInstance->getName(), declarationInstance->getObject() };
		else
			return { L"", nullptr };
	};
	render::ParameterLinker(parameterDeclarationReader).resolve(graph);

	const auto uniformDeclarationReader = [&](const Guid& declarationId) -> render::UniformLinker::named_decl_t {
		Ref< db::Instance > declarationInstance = database->getInstance(declarationId);
		if (declarationInstance != nullptr)
			return { declarationInstance->getName(), declarationInstance->getObject() };
		else
			return { L"", nullptr };
	};
	render::UniformLinker(uniformDeclarationReader).resolve(graph);

	return graph;
}

Ref< Json > validateGraph(db::Database* database, const render::ShaderGraph* shaderGraph, const Guid& guid, bool& outValid)
{
	outValid = false;
	Ref< Json > result = Json::createObject();

	Ref< render::ShaderGraph > resolved = resolveShaderGraph(database, shaderGraph, guid);
	if (!resolved)
	{
		result->setBoolean(L"valid", false);
		result->setString(L"message", L"Failed to resolve shader graph fragments (a referenced fragment may be missing).");
		result->set(L"errorNodes", Json::createArray());
		return result;
	}

	const render::ShaderGraphValidator validator(resolved, guid);
	const render::ShaderGraphValidator::ShaderGraphType type = validator.estimateType();

	AlignedVector< const render::Node* > errorNodes;
	const bool ok = validator.validate(type, &errorNodes);
	const bool integrity = validator.validateIntegrity();
	outValid = ok && integrity;

	Ref< Json > errors = Json::createArray();
	for (const render::Node* node : errorNodes)
	{
		Ref< Json > entry = Json::createObject();
		entry->setString(L"type", shortTypeName(type_name(node)));
		entry->setString(L"guid", node->getId().format());
		const std::wstring info = node->getInformation();
		if (!info.empty())
			entry->setString(L"info", info);
		errors->push(entry);
	}

	result->setBoolean(L"valid", outValid);
	result->setBoolean(L"integrity", integrity);
	result->setString(L"graphType", type == render::ShaderGraphValidator::SgtFragment ? L"fragment" : L"program");
	result->set(L"errorNodes", errors);
	return result;
}

}
