/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include <initializer_list>
#include <vector>
#include "Core/Date/DateTime.h"
#include "Core/Misc/String.h"
#include "Blog/BlogData.h"
#include "Demo/DemoApplication.h"
#include "Demo/HomePage.h"
#include "Forum/ForumData.h"
#include "Webber/Button.h"
#include "Webber/Link.h"
#include "Webber/Markdown.h"
#include "Webber/Static.h"

namespace traktor::demo
{
	namespace
	{

const int32_t c_featuredPosts = 3;
const int32_t c_latestThreads = 4;

/*! Code token; style is null for plain text. */
struct Token
{
	const wchar_t* text;
	const wchar_t* style;
};

struct LatestThread
{
	int32_t board;
	const forum::Thread* thread;
	const forum::Post* last;
};

wbr::Container* addContainer(wbr::Container* parent, const std::wstring& styleClass)
{
	auto container = parent->addChild(new wbr::Container());
	container->setStyleClass(styleClass);
	return container;
}

wbr::Static* addStatic(wbr::Container* parent, const std::wstring& text, const std::wstring& styleClass)
{
	auto st = parent->addChild(new wbr::Static(text));
	st->setStyleClass(styleClass);
	return st;
}

wbr::Container* addCodeWindow(wbr::Container* parent)
{
	auto window = addContainer(parent, L"home_code");
	auto bar = addContainer(window, L"home_code_bar");
	for (int32_t i = 0; i < 3; ++i)
		addStatic(bar, L"", L"home_code_dot");
	return addContainer(window, L"home_code_body");
}

void addCodeLine(wbr::Container* code, int32_t number, std::initializer_list< Token > tokens)
{
	auto line = addContainer(code, L"home_code_line");
	addStatic(line, toString(number), L"home_code_number");
	for (const auto& token : tokens)
		addStatic(line, token.text, token.style ? token.style : L"");
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.demo.HomePage", HomePage, wbr::Container)

HomePage::HomePage(DemoApplication* app, const blog::Blog& blog, const forum::Forum& forum)
{
	setStyleClass(L"home");

	// Hero.
	auto hero = addContainer(addContainer(this, L"home_hero"), L"home_inner home_hero_inner");
	auto heroText = addContainer(hero, L"home_hero_text");
	addStatic(heroText, L"KOD  /  HANTVERK  /  MÄNNISKA", L"home_eyebrow");
	addStatic(heroText, L"Kod som ett hantverk", L"home_title");
	addStatic(heroText, L"Veterankod är för de personer som älskar att lösa problem, som ser skönhet i lösningar och som, helt enkelt, älskar att koda.", L"home_lead");
	auto actions = addContainer(heroText, L"home_actions");
	
	auto readBlog = actions->addChild(new wbr::Button(L"Läs en blogg →"));
	readBlog->setStyleClass(L"home_primary");
	readBlog->clicked() += [app]() { app->showBlog(); };

	auto visitForum = actions->addChild(new wbr::Button(L"Gå till forumet"));
	visitForum->setStyleClass(L"home_secondary");
	visitForum->clicked() += [app]() { app->showForum(); };

	auto heroCode = addCodeWindow(hero);
	addCodeLine(heroCode, 1, { { L"auto", L"home_kw" }, { L" save = ", nullptr }, { L"new", L"home_kw" }, { L" wbr::", nullptr }, { L"Button", L"home_type" }, { L"(", nullptr }, { L"L\"Save\"", L"home_str" }, { L");", nullptr } });
	addCodeLine(heroCode, 2, { { L"save->", nullptr }, { L"clicked", L"home_fn" }, { L"() += [&]() {", nullptr } });
	addCodeLine(heroCode, 3, { { L"    post.text = text->", nullptr }, { L"getText", L"home_fn" }, { L"();", nullptr } });
	addCodeLine(heroCode, 4, { { L"    ", nullptr }, { L"showPost", L"home_fn" }, { L"(post);", nullptr } });
	addCodeLine(heroCode, 5, { { L"};", nullptr } });
	addCodeLine(heroCode, 6, { { L"// Only what changed reaches the browser.", L"home_comment" } });

	// Introduction.
	auto intro = addContainer(addContainer(this, L"home_intro"), L"home_inner home_intro_inner");
	addStatic(intro, L"Webber", L"home_brand");
	addStatic(intro, L"A small library from the Traktor engine for people who like to know exactly what their web page is doing; widgets on the server, HTML in the browser and nothing in between.", L"home_intro_text");
	const wchar_t* features[][3] = {
		{ L"{ }", L"Widgets", L"Pages are trees of widgets, built and kept on the server." },
		{ L"Δ", L"Small updates", L"Only the parts of the page that changed are sent." },
		{ L"</>", L"Plain C++", L"No client framework and no build step for the browser." }
	};
	for (const auto& feature : features)
	{
		auto item = addContainer(intro, L"home_feature");
		addStatic(item, feature[0], L"home_feature_icon");
		auto text = addContainer(item, L"home_feature_text");
		addStatic(text, feature[1], L"home_feature_title");
		addStatic(text, feature[2], L"home_feature_body");
	}

	// Featured blog posts and latest forum threads.
	auto featured = addContainer(addContainer(this, L"home_featured"), L"home_inner");
	addStatic(featured, L"Featured", L"home_section_title");
	auto grid = addContainer(featured, L"home_grid");

	std::vector< const blog::Post* > posts;
	for (const auto& post : blog.posts)
		posts.push_back(&post);
	std::stable_sort(posts.begin(), posts.end(), [](const blog::Post* lh, const blog::Post* rh) {
		return lh->time > rh->time;
	});
	if ((int32_t)posts.size() > c_featuredPosts)
		posts.resize(c_featuredPosts);

	for (int32_t i = 0; i < (int32_t)posts.size(); ++i)
	{
		const blog::Post& p = *posts[i];
		const int32_t post = p.id;
		auto card = addContainer(grid, L"home_card home_card_" + toString(i));
		addStatic(card, L"BLOG  ·  " + DateTime(p.time).format(L"%Y-%m-%d"), L"home_card_label");
		auto title = card->addChild(new wbr::Link(p.title));
		title->setStyleClass(L"home_card_title");
		title->clicked() += [app, post]() { app->showBlogPost(post); };
		card->addChild(new wbr::Markdown(p.text.substr(0, p.text.find(L"\n\n"))))->setStyleClass(L"home_card_text");
	}

	std::vector< LatestThread > threads;
	for (int32_t b = 0; b < (int32_t)forum.boards.size(); ++b)
	{
		for (const auto& thread : forum.boards[b].threads)
		{
			if (!thread.posts.empty())
				threads.push_back({ b, &thread, &thread.posts.back() });
		}
	}
	std::stable_sort(threads.begin(), threads.end(), [](const LatestThread& lh, const LatestThread& rh) {
		return lh.last->time > rh.last->time;
	});
	if ((int32_t)threads.size() > c_latestThreads)
		threads.resize(c_latestThreads);

	auto panel = addContainer(grid, L"home_panel");
	addStatic(panel, L"Latest in the forum", L"home_panel_title");
	for (const auto& latest : threads)
	{
		auto item = addContainer(panel, L"home_panel_item");
		const int32_t board = latest.board;
		const int32_t thread = latest.thread->id;
		item->addChild(new wbr::Link(latest.thread->title))->clicked() += [app, board, thread]() { app->showForumThread(board, thread); };
		addStatic(item, latest.last->author + L"  ·  " + DateTime(latest.last->time).format(L"%Y-%m-%d"), L"home_panel_meta");
	}

	// Closing band.
	auto bottom = addContainer(addContainer(this, L"home_bottom"), L"home_inner home_bottom_inner");
	auto craft = addContainer(bottom, L"home_bottom_text");
	addStatic(craft, L"Hantverket", L"home_section_title");
	addStatic(craft, L"Every click goes to the server and every page is built by the same code. What you write in C++ is what the browser shows.", L"home_bottom_body");

	auto bottomCode = addCodeWindow(bottom);
	addCodeLine(bottomCode, 1, { { L"auto", L"home_kw" }, { L" md = ", nullptr }, { L"new", L"home_kw" }, { L" wbr::", nullptr }, { L"Markdown", L"home_type" }, { L"(text);", nullptr } });
	addCodeLine(bottomCode, 2, { { L"page->", nullptr }, { L"addChild", L"home_fn" }, { L"(md);", nullptr } });
	addCodeLine(bottomCode, 3, { { L"return", L"home_kw" }, { L" page;", nullptr } });

	auto care = addContainer(bottom, L"home_bottom_text");
	addStatic(care, L"För utvecklare som bryr sig", L"home_section_title");
	addStatic(care, L"Read about how Webber works on the blog, or ask questions and share ideas in the forum.", L"home_bottom_body");

	auto checks = addContainer(bottom, L"home_checks");
	for (auto check : { L"Server-side widgets in C++", L"Blog posts written in Markdown", L"Forum with user accounts", L"Only changes are sent to the browser" })
		addStatic(checks, check, L"home_check");
}

}
