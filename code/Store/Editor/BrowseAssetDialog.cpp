/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Drawing/Image.h"
#include "Net/Url.h"
#include "Net/Http/HttpClient.h"
#include "Net/Http/HttpClientResult.h"
#include "Store/Editor/BrowseAssetDialog.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/CheckBox.h"
#include "Ui/PreviewList/PreviewItem.h"
#include "Ui/PreviewList/PreviewItems.h"
#include "Ui/PreviewList/PreviewList.h"
#include "Ui/TableLayout.h"
#include "Xml/Attribute.h"
#include "Xml/Document.h"
#include "Xml/Element.h"

namespace traktor
{
	namespace store
	{
		namespace
		{

std::wstring getChildElementValue(xml::Element* elm, const std::wstring& name, const std::wstring& defaultValue)
{
	auto child = elm->getChildElementByName(name);
	return (child != nullptr) ? child->getValue() : defaultValue;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.store.BrowseAssetDialog", BrowseAssetDialog, ui::ConfigDialog)

BrowseAssetDialog::BrowseAssetDialog(const std::wstring& serverHost)
:   m_serverHost(serverHost)
{
}

bool BrowseAssetDialog::create(ui::Widget* parent)
{
	if (!ui::ConfigDialog::create(
		parent,
		L"Browse assets",
		1024_ut,
		600_ut,
		ui::ConfigDialog::WsCenterParent | ui::ConfigDialog::WsDefaultResizable | ui::ConfigDialog::WsOkCancelButtons,
		new ui::TableLayout(L"*,100%", L"100%", 0_ut, 0_ut)
	))
		return false;

	Ref< ui::Container > containerTags = new ui::Container();
	containerTags->create(this, ui::WsNone, new ui::TableLayout(L"100%", L"*", 8_ut, 8_ut));

	m_listAssets = new ui::PreviewList();
	m_listAssets->create(this, ui::PreviewList::WsMultiple | ui::WsDoubleBuffer);
	m_listAssets->setItemSize(ui::PreviewList::IsLarge);

	// Get all unique tags stored on server.
	Ref< net::HttpClient > httpClient = new net::HttpClient();

	auto queryTags = httpClient->get(net::Url(L"http://" + m_serverHost + L"/tags"));
	if (!queryTags->succeeded())
	{
		log::error << L"Unable to query store server \"" << m_serverHost << L"\"; check store server settings." << Endl;
		return false;
	}

	xml::Document tagsDocument;
	if (!tagsDocument.loadFromStream(queryTags->getStream()))
	{
		log::error << L"Unable to parse reply from store server; check store server settings." << Endl;
		return false;
	}

	RefArray< xml::Element > tags;
	tagsDocument.getDocumentElement()->get(L"tag", tags);
	for (auto tag : tags)
	{
		Ref< ui::CheckBox > checkTag = new ui::CheckBox();
		checkTag->create(containerTags, tag->getValue(), false);
		checkTag->addEventHandler< ui::ButtonClickEvent >([&](ui::ButtonClickEvent* event) {
			updatePackages();
		});
		m_checkTags.push_back(checkTag);
	}

	update();
	return true;
}

ui::DialogResult BrowseAssetDialog::showModal(RefArray< net::Url >& outUrls)
{
	if (ui::ConfigDialog::showModal() != ui::DialogResult::Ok)
		return ui::DialogResult::Cancel;

	RefArray< ui::PreviewItem > selectedItems;
	m_listAssets->getSelectedItems(selectedItems);
	
	for (auto selectedItem : selectedItems)
	{
		auto url = selectedItem->getData< net::Url >(L"URL");
		if (url)
			outUrls.push_back(url);
	}

	return ui::DialogResult::Ok;
}

void BrowseAssetDialog::updatePackages()
{
	Ref< ui::PreviewItems > items = new ui::PreviewItems();
	m_listAssets->setItems(items);
	m_listAssets->update();

	std::wstring tags;
	for (auto checkTag : m_checkTags)
	{
		if (!tags.empty())
			tags += L";";
		if (checkTag->isChecked())
			tags += checkTag->getText();
	}

	Ref< net::HttpClient > httpClient = new net::HttpClient();

	auto queryCatalogue = httpClient->get(net::Url(L"http://" + m_serverHost + L"/catalogue?tags=" + tags));
	queryCatalogue->defer([=]() {
		if (!queryCatalogue->succeeded())
			return;

		xml::Document catalogueDocument;
		if (!catalogueDocument.loadFromStream(queryCatalogue->getStream()))
		{
			log::error << L"Unable to parse catalogue." << Endl;
			return;
		}

		RefArray< xml::Element > packageElements;
		catalogueDocument.getDocumentElement()->get(L"package", packageElements);
		for (auto packageElement : packageElements)
		{
			auto id = packageElement->getAttribute(L"id", L"")->getValue();

			Ref< ui::PreviewItem > item = new ui::PreviewItem();
			item->setText(L"Pending...");
			items->add(item);
			m_listAssets->requestUpdate();

			auto queryManifest = httpClient->get(net::Url(L"http://" + m_serverHost + L"/" + id + L"/Manifest.xml"));
			queryManifest->defer([=]() {

				if (!queryManifest->succeeded())
					return;

				xml::Document manifestDocument;
				if (!manifestDocument.loadFromStream(queryManifest->getStream()))
				{
					log::error << L"Unable to parse manifest." << Endl;
					return;
				}

				auto de = manifestDocument.getDocumentElement();

				std::wstring name = getChildElementValue(de, L"name", L"Unnamed");
				std::wstring thumbnailUrl = getChildElementValue(de, L"thumbnail-url", L"");
				std::wstring databaseUrl = getChildElementValue(de, L"database-url", L"Database.compact");

				item->setText(name);
				item->setData(L"URL", new net::Url(L"http://" + m_serverHost + L"/" + id + L"/" + databaseUrl));

				m_listAssets->requestUpdate();

				if (!thumbnailUrl.empty())
				{
					auto queryThumb = httpClient->get(net::Url(L"http://" + m_serverHost + L"/" + id + L"/" + thumbnailUrl));
					queryThumb->defer([=]() {
						
						Ref< drawing::Image > thumbnail = drawing::Image::load(
							queryThumb->getStream(),
							Path(thumbnailUrl).getExtension()
						);
						if (thumbnail)
						{
							item->setImage(new ui::Bitmap(thumbnail));
							m_listAssets->requestUpdate();
						}
					});
				}
			});
		}
	});
}

	}
}