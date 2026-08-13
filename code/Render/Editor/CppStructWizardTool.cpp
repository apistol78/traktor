/*
 * TRAKTOR
 * Copyright (c) 2025-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/CppStructWizardTool.h"

#include "Core/Log/Log.h"
#include "Core/Math/MathUtils.h"
#include "Core/Misc/Align.h"
#include "Database/Instance.h"
#include "I18N/Text.h"
#include "Render/Editor/Shader/StructDeclaration.h"

namespace traktor::render
{
namespace
{

/*! std430 layout of a single struct element. */
struct Std430Type
{
	const wchar_t* cppType;	 //!< C++ type of a single component.
	uint32_t componentCount; //!< Number of components; 1 is emitted as a plain scalar.
	uint32_t componentSize;	 //!< Size in bytes of a single component.
	uint32_t alignment;		 //!< std430 base alignment of the element.
	uint32_t size;			 //!< Size in bytes of the element, excluding trailing padding.
};

/*! std430 layout per data type.
 *
 * Must be kept in sync with glsl_storage_type in Render/Editor/Glsl/GlslType.cpp
 * since that determine the GLSL type the shader compiler see.
 *
 * std430 base alignment of a vector is the size of a single component times
 * 1 (scalar), 2 (two components) or 4 (three or four components); note that a
 * three component vector therefore is aligned as if it had four components
 * while still only occupying three.
 */
const Std430Type c_std430Types[] = {
	{ L"float", 1, 4, 4, 4 },	 // DtFloat1   float
	{ L"float", 2, 4, 8, 8 },	 // DtFloat2   vec2
	{ L"float", 3, 4, 16, 12 },	 // DtFloat3   vec3
	{ L"float", 4, 4, 16, 16 },	 // DtFloat4   vec4
	{ L"uint8_t", 4, 1, 4, 4 },	 // DtByte4    u8vec4
	{ L"uint8_t", 4, 1, 4, 4 },	 // DtByte4N   u8vec4
	{ L"int16_t", 2, 2, 4, 4 },	 // DtShort2   i16vec2
	{ L"int16_t", 4, 2, 8, 8 },	 // DtShort4   i16vec4
	{ L"int16_t", 2, 2, 4, 4 },	 // DtShort2N  i16vec2
	{ L"int16_t", 4, 2, 8, 8 },	 // DtShort4N  i16vec4
	{ L"half_t", 2, 2, 4, 4 },	 // DtHalf2    f16vec2
	{ L"half_t", 4, 2, 8, 8 },	 // DtHalf4    f16vec4
	{ L"int32_t", 1, 4, 4, 4 },	 // DtInteger1 int
	{ L"int32_t", 2, 4, 8, 8 },	 // DtInteger2 ivec2
	{ L"int32_t", 3, 4, 16, 12 }, // DtInteger3 ivec3
	{ L"int32_t", 4, 4, 16, 16 }  // DtInteger4 ivec4
};

}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.CppStructWizardTool", 0, CppStructWizardTool, editor::IWizardTool)

bool CppStructWizardTool::create(const PropertyGroup* settings)
{
	return true;
}

std::wstring CppStructWizardTool::getDescription() const
{
	return i18n::Text(L"CPP_STRUCT_WIZARDTOOL_DESCRIPTION");
}

const TypeInfoSet CppStructWizardTool::getSupportedTypes() const
{
	return makeTypeInfoSet< StructDeclaration >();
}

uint32_t CppStructWizardTool::getFlags() const
{
	return editor::IWizardTool::WfInstance;
}

bool CppStructWizardTool::launch(ui::Widget* parent, editor::IEditor* editor, db::Group* group, db::Instance* instance)
{
	Ref< const StructDeclaration > structDecl = instance->getObject< StructDeclaration >();
	if (!structDecl)
		return false;

	const auto& elements = structDecl->getElements();
	if (elements.empty())
	{
		log::error << L"Unable to generate C++ struct; declaration \"" << instance->getName() << L"\" is empty." << Endl;
		return false;
	}

	// A struct is aligned as its most aligned member, and since all types used
	// have an alignment of at least four this also become the minimum alignment.
	uint32_t structAlignment = 4;
	bool useHalf = false;
	for (const auto& elm : elements)
	{
		const Std430Type& t = c_std430Types[(int32_t)elm.type];
		structAlignment = max(structAlignment, t.alignment);
		useHalf |= (bool)(elm.type == DtHalf2 || elm.type == DtHalf4);
	}

	log::info << L"// Generated from \"" << instance->getName() << L"\"; matching GLSL std430 layout." << Endl;
	if (useHalf)
		log::info << L"// Include \"Core/Math/Half.h\" for half_t." << Endl;

	log::info << L"#pragma pack(1)" << Endl;
	log::info << L"struct " << instance->getName() << Endl;
	log::info << L"{" << Endl;
	log::info << IncreaseIndent;

	uint32_t offset = 0;
	for (const auto& elm : elements)
	{
		const Std430Type& t = c_std430Types[(int32_t)elm.type];

		// Pad up to the alignment of this element; for an array it's the alignment
		// of the element type as std430 do not round arrays up to 16 bytes.
		const uint32_t pad = alignUp(offset, t.alignment) - offset;
		if (pad > 0)
			log::info << L"uint8_t __pad__" << elm.name << L"[" << pad << L"]; // " << offset << Endl;
		offset += pad;

		if (elm.length > 0)
		{
			// Array elements are placed at a stride of the element size rounded up
			// to the element alignment; this only differ from the size for three
			// component vectors which thus need a padding component in C++ to keep
			// the stride, hence declaring the stride's worth of components.
			const uint32_t stride = alignUp(t.size, t.alignment);
			const uint32_t strideComponents = stride / t.componentSize;

			if (strideComponents > 1)
			{
				log::info << t.cppType << L" " << elm.name << L"[" << elm.length << L"][" << strideComponents << L"];";
				if (strideComponents > t.componentCount)
					log::info << L" // element stride padded to " << stride << L" bytes";
				log::info << Endl;
			}
			else
				log::info << t.cppType << L" " << elm.name << L"[" << elm.length << L"];" << Endl;

			offset += (uint32_t)elm.length * stride;
		}
		else
		{
			// A lone three component vector occupy three components only; any
			// trailing padding is covered by the alignment of the next element.
			if (t.componentCount > 1)
				log::info << t.cppType << L" " << elm.name << L"[" << t.componentCount << L"];" << Endl;
			else
				log::info << t.cppType << L" " << elm.name << L";" << Endl;

			offset += t.size;
		}
	}

	// Pad the struct up to its own alignment; this is the stride used when it's
	// read as an array of structs, thus the padding must be part of the C++ struct.
	const uint32_t pad = alignUp(offset, structAlignment) - offset;
	if (pad > 0)
		log::info << L"uint8_t __pad__[" << pad << L"]; // " << offset << Endl;

	log::info << DecreaseIndent;
	log::info << L"};" << Endl;
	log::info << L"#pragma pack()" << Endl;
	log::info << Endl;
	log::info << L"static_assert(sizeof(" << instance->getName() << L") == " << (offset + pad) << L");" << Endl;

	return true;
}

}
