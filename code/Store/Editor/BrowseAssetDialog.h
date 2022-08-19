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

class CheckBox;
class PreviewList;

    }

    namespace store
    {

class BrowseAssetDialog : public ui::ConfigDialog
{
    T_RTTI_CLASS;

public:
    BrowseAssetDialog(const std::wstring& serverHost);

    bool create(ui::Widget* parent);

    ui::DialogResult showModal(RefArray< net::Url >& outUrls);

private:
    std::wstring m_serverHost;
    RefArray< ui::CheckBox > m_checkTags;
    Ref< ui::PreviewList > m_listAssets;

    void updatePackages();
};

    }
}