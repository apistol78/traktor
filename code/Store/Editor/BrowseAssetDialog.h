#pragma once

#include "Core/RefArray.h"
#include "Ui/ConfigDialog.h"

namespace traktor
{
    namespace net
    {

class Url;

    }

    namespace ui
    {

class GridView;
    }

    namespace store
    {

class BrowseAssetDialog : public ui::ConfigDialog
{
    T_RTTI_CLASS;

public:
    BrowseAssetDialog(const std::wstring& serverHost);

    bool create(ui::Widget* parent);

    bool showModal(RefArray< net::Url >& outUrls);

private:
    std::wstring m_serverHost;
    Ref< ui::GridView > m_gridAssets;
};

    }
}