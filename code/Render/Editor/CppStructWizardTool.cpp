/*
 * TRAKTOR
 * Copyright (c) 2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/CppStructWizardTool.h"

#include "Core/Log/Log.h"
#include "Core/Misc/Align.h"
#include "Database/Instance.h"
#include "I18N/Text.h"
#include "Render/Editor/Shader/StructDeclaration.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.CppStructWizardTool", 0, CppStructWizardTool, editor::IWizardTool)

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

	// std430

	log::info << L"#pragma pack(1)" << Endl;
	log::info << L"struct " << instance->getName() << Endl;
	log::info << L"{" << Endl;
	log::info << IncreaseIndent;

	uint32_t offset = 0;
	for (const auto& elm : structDecl->getElements())
	{
		switch (elm.type)
		{
		case DtFloat1:
			{
				const uint32_t pad = alignUp(offset, 4) - offset;
				if (pad > 0)
					log::info << L"uint8_t __pad__" << elm.name << L"[" << pad << L"]; // " << offset << Endl;
				log::info << L"float " << elm.name << L";" << Endl;
				offset += pad + 4;
			}
			break;

		case DtFloat2:
			{
				const uint32_t pad = alignUp(offset, 8) - offset;
				if (pad > 0)
					log::info << L"uint8_t __pad__" << elm.name << L"[" << pad << L"]; // " << offset << Endl;
				log::info << L"float " << elm.name << L"[2];" << Endl;
				offset += pad + 8;
			}
			break;

		case DtFloat3:
			{
				const uint32_t pad = alignUp(offset, 16) - offset;
				if (pad > 0)
					log::info << L"uint8_t __pad__" << elm.name << L"[" << pad << L"]; // " << offset << Endl;
				log::info << L"float " << elm.name << L"[3];" << Endl;
				offset += pad + 12;
			}
			break;

		case DtFloat4:
			{
				const uint32_t pad = alignUp(offset, 16) - offset;
				if (pad > 0)
					log::info << L"uint8_t __pad__" << elm.name << L"[" << pad << L"]; // " << offset << Endl;
				log::info << L"float " << elm.name << L"[4];" << Endl;
				offset += pad + 16;
			}
			break;

		case DtByte4:
		case DtByte4N:
		case DtShort2:
		case DtShort4:
		case DtShort2N:
		case DtShort4N:
		case DtHalf2:
		case DtHalf4:
			break;

		case DtInteger1:
			{
				const uint32_t pad = alignUp(offset, 4) - offset;
				if (pad > 0)
					log::info << L"uint8_t __pad__" << elm.name << L"[" << pad << L"]; // " << offset << Endl;
				log::info << L"int32_t " << elm.name << L";" << Endl;
				offset += pad + 4;
			}
			break;

		case DtInteger2:
			{
				const uint32_t pad = alignUp(offset, 8) - offset;
				if (pad > 0)
					log::info << L"uint8_t __pad__" << elm.name << L"[" << pad << L"]; // " << offset << Endl;
				log::info << L"int32_t " << elm.name << L"[2];" << Endl;
				offset += pad + 8;
			}
			break;

		case DtInteger3:
			{
				const uint32_t pad = alignUp(offset, 16) - offset;
				if (pad > 0)
					log::info << L"uint8_t __pad__" << elm.name << L"[" << pad << L"]; // " << offset << Endl;
				log::info << L"int32_t " << elm.name << L"[3];" << Endl;
				offset += pad + 12;
			}
			break;

		case DtInteger4:
			{
				const uint32_t pad = alignUp(offset, 16) - offset;
				if (pad > 0)
					log::info << L"uint8_t __pad__" << elm.name << L"[" << pad << L"]; // " << offset << Endl;
				log::info << L"int32_t " << elm.name << L"[4];" << Endl;
				offset += pad + 16;
			}
			break;
		}
	}

	const uint32_t pad = alignUp(offset, 16) - offset;
	if (pad > 0)
		log::info << L"uint8_t __pad__[" << pad << L"]; // " << offset << Endl;

	log::info << DecreaseIndent;
	log::info << L"};" << Endl;
	log::info << L"#pragma pack()" << Endl;

	return true;
}

}
