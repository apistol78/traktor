/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Date/DateTime.h"
#include "Forum/ForumData.h"

namespace traktor::forum
{
	namespace
	{

uint64_t at(uint8_t day, uint8_t hour, uint8_t minute)
{
	return DateTime(2026, 10, day, hour, minute, 0).getSecondsSinceEpoch();
}

	}

Forum createSampleForum()
{
	Forum forum;
	forum.name = L"Webber Forum";

	Board& general = forum.boards.push_back();
	general.name = L"General";
	general.description = L"Announcements and anything that doesn't fit elsewhere.";
	general.threads.push_back({ L"Welcome to the forum", {
		{ L"Admin", L"Welcome! This forum is the Webber demo application.\nEvery page is built from server-side widgets; clicks are sent to the server, which replies with the parts of the page that changed.", at(20, 9, 0) },
		{ L"Alice", L"Neat, the page never reloads.", at(20, 10, 12) },
		{ L"Bob", L"Testing special characters: <b>not bold</b> & \"quotes\".", at(21, 8, 45) }
	} });
	general.threads.push_back({ L"Introduce yourself", {
		{ L"Alice", L"I'm Alice, mostly working on tools.", at(22, 14, 30) }
	} });

	Board& engine = forum.boards.push_back();
	engine.name = L"Engine";
	engine.description = L"Traktor engine development.";
	engine.threads.push_back({ L"Webber is back", {
		{ L"Admin", L"The old Webber library was restored as a standalone tool in tools/Webber.", at(28, 16, 5) },
		{ L"Bob", L"Does it still need the editor?", at(28, 16, 40) },
		{ L"Admin", L"No, it only links Core, Html and Net.", at(28, 17, 2) }
	} });
	engine.threads.push_back({ L"Vulkan validation layers", {
		{ L"Bob", L"Any tips for keeping the validation layers quiet?", at(25, 11, 20) }
	} });

	Board& offtopic = forum.boards.push_back();
	offtopic.name = L"Off-topic";
	offtopic.description = L"Everything else.";

	return forum;
}

}
