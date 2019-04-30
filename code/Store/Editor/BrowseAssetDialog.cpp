#include "Core/Log/Log.h"
#include "Net/Url.h"
#include "Net/Http/HttpClient.h"
#include "Net/Http/HttpClientResult.h"
#include "Store/Editor/BrowseAssetDialog.h"
#include "Ui/Application.h"
#include "Ui/CheckBox.h"
#include "Ui/FloodLayout.h"
#include "Ui/TableLayout.h"
#include "Ui/GridView/GridColumn.h"
#include "Ui/GridView/GridItem.h"
#include "Ui/GridView/GridRow.h"
#include "Ui/GridView/GridView.h"
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
        ui::dpi96(1024),
        ui::dpi96(600),
		ui::ConfigDialog::WsDefaultResizable | ui::ConfigDialog::WsOkCancelButtons,
        new ui::TableLayout(L"*,100%", L"100%", 0, 0)
    ))
        return false;

    Ref< ui::Container > containerTags = new ui::Container();
    containerTags->create(this, ui::WsNone, new ui::TableLayout(L"100%", L"*", ui::dpi96(8), ui::dpi96(8)));

    m_gridAssets = new ui::GridView();
    m_gridAssets->create(this, ui::GridView::WsMultiSelect | ui::WsDoubleBuffer);
    m_gridAssets->addColumn(new ui::GridColumn(L"Thumbnail", ui::dpi96(100)));
    m_gridAssets->addColumn(new ui::GridColumn(L"Name", ui::dpi96(600)));

    // Get all unique tags stored on server.
    Ref< net::HttpClient > httpClient = new net::HttpClient();

    auto queryTags = httpClient->get(net::Url(L"http://" + m_serverHost + L"/tags"));
    if (!queryTags->succeeded())
        return false;

    xml::Document tagsDocument;
    if (!tagsDocument.loadFromStream(queryTags->getStream()))
        return false;

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

bool BrowseAssetDialog::showModal(RefArray< net::Url >& outUrls)
{
	if (ui::ConfigDialog::showModal() != ui::DrOk)
		return false;

    RefArray< ui::GridRow > selectedRows;
    m_gridAssets->getRows(selectedRows, ui::GridView::GfDescendants | ui::GridView::GfSelectedOnly);
	
    for (auto selectedRow : selectedRows)
    {
        auto url = selectedRow->getData< net::Url >(L"URL");
        if (url)
            outUrls.push_back(url);
    }

	return true;
}

void BrowseAssetDialog::updatePackages()
{
    m_gridAssets->removeAllRows();
    m_gridAssets->update();

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
            
            Ref< ui::GridRow > rowPackage = new ui::GridRow();
            rowPackage->add(new ui::GridItem(L""));
            rowPackage->add(new ui::GridItem(L"Pending..."));
            m_gridAssets->addRow(rowPackage);
            m_gridAssets->requestUpdate();

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
                std::wstring databaseUrl = getChildElementValue(de, L"database-url", L"Database.compact");

                rowPackage->get(1)->setText(name);
                rowPackage->setData(L"URL", new net::Url(L"http://" + m_serverHost + L"/" + id + L"/" + databaseUrl));
                m_gridAssets->requestUpdate();
            });
        }
    });
}

    }
}