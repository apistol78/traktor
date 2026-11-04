/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include "Core/Date/DateTime.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Forum/ForumData.h"

namespace traktor::forum
{
	namespace
	{

uint64_t at(uint8_t day, uint8_t hour, uint8_t minute)
{
	return DateTime(2026, 10, day, hour, minute, 0).getSecondsSinceEpoch();
}

void assignIds(Forum& forum)
{
	for (auto& board : forum.boards)
	{
		for (auto& thread : board.threads)
		{
			thread.id = forum.nextId++;
			for (auto& post : thread.posts)
				post.id = forum.nextId++;
		}
	}
}

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.forum.Forum", 1, Forum, ISerializable)

void Post::serialize(ISerializer& s)
{
	if (s.getVersion< Forum >() >= 1)
		s >> Member< int32_t >(L"id", id);

	s >> Member< std::wstring >(L"author", author);
	s >> Member< std::wstring >(L"text", text);
	s >> Member< uint64_t >(L"time", time);

	// Version 0 kept deleted posts; mark them for removal.
	if (s.getVersion< Forum >() < 1)
	{
		bool deleted = false;
		s >> Member< bool >(L"deleted", deleted);
		id = deleted ? -1 : 0;
	}
}

void Thread::serialize(ISerializer& s)
{
	if (s.getVersion< Forum >() >= 1)
		s >> Member< int32_t >(L"id", id);

	s >> Member< std::wstring >(L"title", title);
	s >> MemberAlignedVector< Post, MemberComposite< Post > >(L"posts", posts);
}

void Board::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", name);
	s >> Member< std::wstring >(L"description", description);
	s >> MemberAlignedVector< Thread, MemberComposite< Thread > >(L"threads", threads);
}

void Forum::serialize(ISerializer& s)
{
	if (s.getVersion() >= 1)
		s >> Member< int32_t >(L"nextId", nextId);

	s >> Member< std::wstring >(L"name", name);
	s >> MemberAlignedVector< Board, MemberComposite< Board > >(L"boards", boards);

	// Version 0 had no ids; drop deleted posts, then threads left empty, and number the rest.
	if (s.getVersion() < 1)
	{
		for (auto& board : boards)
		{
			for (auto& thread : board.threads)
				thread.posts.erase(std::remove_if(thread.posts.begin(), thread.posts.end(), [](const Post& post) { return post.id < 0; }), thread.posts.end());
			board.threads.erase(std::remove_if(board.threads.begin(), board.threads.end(), [](const Thread& thread) { return thread.posts.empty(); }), board.threads.end());
		}
		assignIds(*this);
	}
}

Ref< Forum > createSampleForum()
{
	Ref< Forum > forum = new Forum();
	forum->name = L"Webber Forum";

	Board& general = forum->boards.push_back();
	general.name = L"General";
	general.description = L"Announcements and anything that doesn't fit elsewhere.";
	general.threads.push_back({ L"Welcome to the forum", {
		{ L"apistol", L"Welcome! This forum is the Webber demo application.\nEvery page is built from server-side widgets; clicks are sent to the server, which replies with the parts of the page that changed.", at(20, 9, 0) },
		{ L"Alice", L"Neat, the page never reloads.", at(20, 10, 12) },
		{ L"Bob", L"Testing special characters: <b>not bold</b> & \"quotes\".", at(21, 8, 45) }
	} });
	general.threads.push_back({ L"Introduce yourself", {
		{ L"Alice", L"I'm Alice, mostly working on tools.", at(22, 14, 30) },
		{ L"klara", L"Hi, I'm Klara. Mostly here to read about the engine.", at(23, 9, 15) }
	} });

	Board& engine = forum->boards.push_back();
	engine.name = L"Engine";
	engine.description = L"Traktor engine development.";
	engine.threads.push_back({ L"Webber is back", {
		{ L"apistol", L"The old Webber library was restored as a standalone tool in tools/Webber.", at(28, 16, 5) },
		{ L"Bob", L"Does it still need the editor?", at(28, 16, 40) },
		{ L"apistol", L"No, it only links Core, Html and Net.", at(28, 17, 2) }
	} });
	engine.threads.push_back({ L"Vulkan validation layers", {
		{ L"Bob", L"Any tips for keeping the validation layers quiet?", at(25, 11, 20) }
	} });

	Board& offtopic = forum->boards.push_back();
	offtopic.name = L"Off-topic";
	offtopic.description = L"Everything else.";

	assignIds(*forum);
	return forum;
}

}
