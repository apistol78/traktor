/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include <cwctype>
#include <vector>
#include "Core/Date/DateTime.h"
#include "Core/Misc/String.h"
#include "Project/ProjectData.h"
#include "Project/ProjectPage.h"
#include "Webber/Button.h"
#include "Webber/Edit.h"
#include "Webber/FileUpload.h"
#include "Webber/Link.h"
#include "Webber/Markdown.h"
#include "Webber/Static.h"
#include "Webber/TextArea.h"

namespace traktor::project
{
	namespace
	{

const size_t c_maxSummaryLength = 300;

/*! Project id from location segment, -1 unless only digits. */
int32_t parseId(const std::wstring& text)
{
	if (text.empty() || text.size() > 9 || text.find_first_not_of(L"0123456789") != text.npos)
		return -1;
	return parseString< int32_t >(text);
}

Project* findProject(Projects& projects, int32_t id)
{
	auto it = std::find_if(projects.projects.begin(), projects.projects.end(), [=](const Project& project) { return project.id == id; });
	return it != projects.projects.end() ? &(*it) : nullptr;
}

std::wstring formatDate(uint64_t time)
{
	return DateTime(time).format(L"%Y-%m-%d");
}

/*! Placeholder image; project's initial on a color chosen by id. */
std::wstring tileStyle(const Project& project)
{
	return L"project_tile project_tile_" + toString(project.id % 4);
}

std::wstring tileText(const Project& project)
{
	return project.title.empty() ? L"?" : std::wstring(1, (wchar_t)std::towupper(project.title[0]));
}

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

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.project.ProjectPage", ProjectPage, wbr::Container)

ProjectPage::ProjectPage(Projects& projects, const std::function< void() >& changed, const std::function< std::wstring(const AlignedVector< uint8_t >&) >& storeImage)
:	m_projects(projects)
,	m_changed(changed)
,	m_storeImage(storeImage)
{
	setStyleClass(L"project");
	showProjects();
}

void ProjectPage::setUser(const std::wstring& user, bool admin)
{
	m_user = user;
	m_admin = admin;
	if (m_project >= 0)
		showProject(m_project);
	else
		showProjects();
}

std::wstring ProjectPage::getLocation() const
{
	if (m_editing)
		return m_project >= 0 ? toString(m_project) + L"/edit" : L"new";
	else
		return m_project >= 0 ? toString(m_project) : L"";
}

void ProjectPage::navigate(const std::wstring& location)
{
	const size_t p = location.find(L'/');
	const std::wstring first = location.substr(0, p);
	const std::wstring rest = (p != location.npos) ? location.substr(p + 1) : L"";
	const Project* project = findProject(m_projects, parseId(first));

	if (first == L"new" && !m_user.empty())
		showEditor(-1);
	else if (!project)
		showProjects();
	else if (rest == L"edit" && canEdit(*project))
		showEditor(project->id);
	else
		showProject(project->id);
}

void ProjectPage::showProjects()
{
	m_project = -1;
	m_editing = false;
	removeAllChildren();

	auto header = addContainer(this, L"project_header");
	addStatic(header, L"Projekt", L"project_heading");
	addStatic(header, L"Pågående projekt från medlemmarna. Välj ett projekt för att läsa hela beskrivningen.", L"project_intro");
	if (!m_user.empty())
		header->addChild(new wbr::Button(L"Nytt projekt"))->clicked() += [this]() { showEditor(-1); };

	// Most recently updated first.
	std::vector< const Project* > order;
	for (const auto& project : m_projects.projects)
		order.push_back(&project);
	std::stable_sort(order.begin(), order.end(), [](const Project* lh, const Project* rh) {
		return lh->updated > rh->updated;
	});

	if (order.empty())
		addStatic(this, L"Inga projekt ännu.", L"project_empty");

	auto grid = addContainer(this, L"project_grid");
	for (const Project* project : order)
	{
		const int32_t id = project->id;
		auto card = addContainer(grid, L"project_card");
		addStatic(card, tileText(*project), tileStyle(*project));
		auto body = addContainer(card, L"project_card_body");
		auto title = body->addChild(new wbr::Link(project->title));
		title->setStyleClass(L"project_card_title");
		title->clicked() += [this, id]() { showProject(id); };
		addStatic(body, project->summary, L"project_card_summary");
		addStatic(body, project->author + L"  ·  uppdaterat " + formatDate(project->updated), L"project_card_meta");
	}
}

void ProjectPage::showProject(int32_t project)
{
	const Project* p = findProject(m_projects, project);
	if (!p)
	{
		showProjects();
		return;
	}

	m_project = project;
	m_editing = false;
	removeAllChildren();

	auto back = addChild(new wbr::Link(L"‹ Alla projekt"));
	back->setStyleClass(L"project_back");
	back->clicked() += [this]() { showProjects(); };

	auto header = addContainer(this, L"project_detail");
	addStatic(header, tileText(*p), tileStyle(*p));
	auto info = addContainer(header, L"project_detail_info");
	addStatic(info, p->title, L"project_title");
	addStatic(info, p->summary, L"project_summary");
	addStatic(info, L"av " + p->author + L"  ·  startat " + formatDate(p->created) + L"  ·  uppdaterat " + formatDate(p->updated), L"project_meta");

	auto article = addContainer(this, L"project_article");
	article->addChild(new wbr::Markdown(p->description))->setStyleClass(L"project_text");

	if (canEdit(*p))
	{
		article->addChild(new wbr::Button(L"Redigera"))->clicked() += [this, project]() { showEditor(project); };
		article->addChild(new wbr::Button(L"Ta bort"))->clicked() += [this, project]() {
			auto& projects = m_projects.projects;
			projects.erase(std::remove_if(projects.begin(), projects.end(), [=](const Project& p) { return p.id == project; }), projects.end());
			m_changed();
			showProjects();
		};
	}
}

void ProjectPage::showEditor(int32_t project)
{
	const Project* p = project >= 0 ? findProject(m_projects, project) : nullptr;
	if (project >= 0 && !p)
	{
		showProjects();
		return;
	}

	m_project = project;
	m_editing = true;
	removeAllChildren();
	addStatic(this, project >= 0 ? L"Redigera projekt" : L"Nytt projekt", L"project_heading");

	auto form = addContainer(this, L"project_form");
	addStatic(form, L"Titel", L"project_label");
	auto title = form->addChild(new wbr::Edit());
	addStatic(form, L"Sammanfattning, visas i projektlistan", L"project_label");
	auto summary = form->addChild(new wbr::TextArea());
	summary->setStyleClass(L"project_summary_input");
	addStatic(form, L"Beskrivning (Markdown)", L"project_label");
	auto description = form->addChild(new wbr::TextArea());
	description->setStyleClass(L"project_description_input");
	auto upload = form->addChild(new wbr::FileUpload(L"Infoga bild", L"image/png,image/jpeg,image/gif,image/webp"));
	auto error = addStatic(form, L"", L"project_error");

	// Uploaded image is referenced at the end of the description.
	upload->uploaded() += [this, upload, description, error]() {
		const std::wstring uri = m_storeImage(upload->getData());
		if (uri.empty())
		{
			error->setText(L"Bara bilder i PNG, JPEG, GIF eller WebP kan infogas.");
			return;
		}
		const std::wstring& fileName = upload->getFileName();
		const std::wstring before = rtrim(description->getText());
		error->setText(L"");
		description->setText(before + (before.empty() ? L"" : L"\n\n") + L"![" + fileName.substr(0, fileName.find_last_of(L'.')) + L"](" + uri + L")\n");
	};

	if (p)
	{
		title->setText(p->title);
		summary->setText(p->summary);
		description->setText(p->description);
	}

	auto buttons = addContainer(form, L"project_buttons");
	buttons->addChild(new wbr::Button(L"Spara"))->clicked() += [this, project, title, summary, description, error]() {
		if (trim(title->getText()).empty() || trim(summary->getText()).empty() || trim(description->getText()).empty())
		{
			error->setText(L"Titel, sammanfattning och beskrivning måste fyllas i.");
			return;
		}
		if (trim(summary->getText()).size() > c_maxSummaryLength)
		{
			error->setText(L"Sammanfattningen får vara högst " + toString((int32_t)c_maxSummaryLength) + L" tecken.");
			return;
		}

		const uint64_t now = DateTime::now().getSecondsSinceEpoch();
		if (project >= 0)
		{
			// Project might have been deleted by someone else meanwhile.
			Project* edited = findProject(m_projects, project);
			if (edited && canEdit(*edited))
			{
				edited->title = trim(title->getText());
				edited->summary = trim(summary->getText());
				edited->description = description->getText();
				edited->updated = now;
				m_changed();
			}
			showProject(project);
		}
		else
		{
			const int32_t id = m_projects.nextId++;
			m_projects.projects.push_back({ trim(title->getText()), trim(summary->getText()), description->getText(), m_user, now, now, id });
			m_changed();
			showProject(id);
		}
	};
	buttons->addChild(new wbr::Button(L"Avbryt"))->clicked() += [this, project]() {
		if (project >= 0)
			showProject(project);
		else
			showProjects();
	};
}

bool ProjectPage::canEdit(const Project& project) const
{
	return m_admin || (!m_user.empty() && project.author == m_user);
}

}
