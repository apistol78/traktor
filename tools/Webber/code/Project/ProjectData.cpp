/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Date/DateTime.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Project/ProjectData.h"

namespace traktor::project
{
	namespace
	{

uint64_t at(uint8_t month, uint8_t day, uint8_t hour, uint8_t minute)
{
	return DateTime(2026, month, day, hour, minute, 0).getSecondsSinceEpoch();
}

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.project.Projects", 0, Projects, ISerializable)

void Project::serialize(ISerializer& s)
{
	s >> Member< int32_t >(L"id", id);
	s >> Member< std::wstring >(L"title", title);
	s >> Member< std::wstring >(L"summary", summary);
	s >> Member< std::wstring >(L"description", description);
	s >> Member< std::wstring >(L"author", author);
	s >> Member< uint64_t >(L"created", created);
	s >> Member< uint64_t >(L"updated", updated);
}

void Projects::serialize(ISerializer& s)
{
	s >> Member< int32_t >(L"nextId", nextId);
	s >> MemberAlignedVector< Project, MemberComposite< Project > >(L"projects", projects);
}

Ref< Projects > createSampleProjects()
{
	Ref< Projects > projects = new Projects();

	projects->projects.push_back({
		L"Traktor",
		L"Spelmotor och editor i C++20 med Vulkan-rendering, Lua-skript och en egen databas för assets.",
		L"Traktor är en komplett spelmotor med editor, skriven i C++20.\n"
		L"\n"
		L"## Delar\n"
		L"\n"
		L"- **Rendering** med Vulkan, deferred och forward world renderers.\n"
		L"- **Editor** med egen widget-uppsättning för Windows och Linux.\n"
		L"- **Pipeline** som bygger assets inkrementellt från en källdatabas.\n"
		L"- **Skript** i Lua.\n"
		L"\n"
		L"## Just nu\n"
		L"\n"
		L"Arbetar med asynkron compute i render graph och mer animation.",
		L"apistol", at(9, 1, 10, 0), at(11, 3, 21, 15)
	});

	projects->projects.push_back({
		L"Webber",
		L"Webbapplikationer helt på servern: widgets i C++, HTML i webbläsaren och bara ändringarna skickas.",
		L"Webber är biblioteket som hela den här sajten är byggd med.\n"
		L"\n"
		L"Varje session har ett träd av widgets på servern. Ett klick skickas till servern, sidan byggs om och bara de delar som ändrats skickas tillbaka.\n"
		L"\n"
		L"```\n"
		L"auto save = new wbr::Button(L\"Spara\");\n"
		L"save->clicked() += [&]() { showProject(id); };\n"
		L"```",
		L"apistol", at(10, 20, 9, 0), at(11, 5, 8, 30)
	});

	projects->projects.push_back({
		L"Väderstation på balkongen",
		L"ESP32 med sensorer för temperatur, luftfuktighet och lufttryck som rapporterar varje minut.",
		L"En liten väderstation som sitter på balkongen.\n"
		L"\n"
		L"## Hårdvara\n"
		L"\n"
		L"1. ESP32-kort\n"
		L"2. BME280 för temperatur, luftfuktighet och tryck\n"
		L"3. Solcell och ett litet batteri\n"
		L"\n"
		L"> Nästa steg är ett regnmätare som räknar tippningar.",
		L"klara", at(10, 4, 18, 0), at(10, 30, 20, 45)
	});

	projects->projects.push_back({
		L"Z80-emulator",
		L"Cykelexakt emulator för Z80 med stöd för ZX Spectrum-ROM och en enkel debugger.",
		L"En emulator för Z80-processorn, skriven för att förstå hur gamla datorer fungerade.\n"
		L"\n"
		L"- Alla dokumenterade och de flesta odokumenterade instruktioner\n"
		L"- Startar ZX Spectrum 48K-ROM\n"
		L"- Debugger med brytpunkter och stegning",
		L"Bob", at(8, 15, 12, 0), at(10, 12, 16, 20)
	});

	for (auto& project : projects->projects)
		project.id = projects->nextId++;

	return projects;
}

}
