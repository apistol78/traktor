#include "Core/Log/Log.h"
#include "Net/Url.h"
#include "Net/Http/HttpClient.h"
#include "Net/Http/HttpClientResult.h"
#include "Store/Editor/BrowseAssetDialog.h"
#include "Ui/Application.h"
#include "Ui/FloodLayout.h"
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
        new ui::FloodLayout()
    ))
        return false;

    m_gridAssets = new ui::GridView();
    m_gridAssets->create(this, ui::GridView::WsMultiSelect | ui::WsDoubleBuffer);
    m_gridAssets->addColumn(new ui::GridColumn(L"Thumbnail", ui::dpi96(100)));
    m_gridAssets->addColumn(new ui::GridColumn(L"Name", ui::dpi96(600)));

    // Get categories.
    /*
        textures = {0516941b-6d18-418d-95bc-f76d64bf8ed9}
        Example = {5680d18e-2346-41e6-b385-99382e213002}
    */
    Ref< net::HttpClient > httpClient = new net::HttpClient();
    
    auto queryCatalogue = httpClient->get(net::Url(L"http://" + m_serverHost + L"/catalogue?category=Texture"));
    queryCatalogue->defer([=]() {

        if (!queryCatalogue->succeeded())
            return;

        xml::Document xc;
        if (!xc.loadFromStream(queryCatalogue->getStream()))
        {
            log::error << L"Unable to parse catalogue." << Endl;
            return;
        }

        Ref< ui::GridRow > rowCategory = new ui::GridRow();
        rowCategory->add(new ui::GridItem(L""));
        rowCategory->add(new ui::GridItem(L"Texture"));
        m_gridAssets->addRow(rowCategory);
        m_gridAssets->requestUpdate();

        RefArray< xml::Element > xps;
        xc.getDocumentElement()->get(L"package", xps);

        for (auto xp : xps)
        {
            auto id = xp->getAttribute(L"id", L"")->getValue();
            
            Ref< ui::GridRow > rowPackage = new ui::GridRow();
            rowPackage->add(new ui::GridItem(L""));
            rowPackage->add(new ui::GridItem(L"Pending..."));
            rowCategory->addChild(rowPackage);
            m_gridAssets->requestUpdate();

            auto queryManifest = httpClient->get(net::Url(L"http://" + m_serverHost + L"/Texture/" + id + L"/Manifest.xml"));
            queryManifest->defer([=]() {

                if (!queryManifest->succeeded())
                    return;

                Ref< xml::Document > xm = new xml::Document();
                if (!xm->loadFromStream(queryManifest->getStream()))
                {
                    log::error << L"Unable to parse manifest." << Endl;
                    return;
                }

                std::wstring name = getChildElementValue(xm->getDocumentElement(), L"name", L"Unnamed");
                std::wstring databaseUrl = getChildElementValue(xm->getDocumentElement(), L"database-url", L"Database.compact");

                rowPackage->get(1)->setText(name);
                rowPackage->setData(L"URL", new net::Url(L"http://" + m_serverHost + L"/Texture/" + id + L"/" + databaseUrl));
                m_gridAssets->requestUpdate();
            });
        }
    });

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

    }
}