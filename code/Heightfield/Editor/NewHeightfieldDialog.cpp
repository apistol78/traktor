#include "Core/Math/Const.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Heightfield/Editor/NewHeightfieldDialog.h"
#include "I18N/Text.h"
#include "Ui/Edit.h"
#include "Ui/FloodLayout.h"
#include "Ui/NumericEditValidator.h"
#include "Ui/Static.h"
#include "Ui/TableLayout.h"

namespace traktor
{
	namespace hf
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.hf.NewHeightfieldDialog", NewHeightfieldDialog, ui::ConfigDialog)

bool NewHeightfieldDialog::create(ui::Widget* parent)
{
	if (!ui::ConfigDialog::create(parent, i18n::Text(L"NEW_HEIGHTFIELD_TITLE"), 600, 200, ui::ConfigDialog::WsDefaultResizable, new ui::FloodLayout()))
		return false;

	Ref< ui::Container > containerInner = new ui::Container();
	if (!containerInner->create(this, ui::WsNone, new ui::TableLayout(L"*,100%", L"*", 8, 4)))
		return false;

	Ref< ui::Static > staticGridSize = new ui::Static();
	staticGridSize->create(containerInner, i18n::Text(L"NEW_HEIGHTFIELD_GRID_SIZE"));

	m_editGridSize = new ui::Edit();
	m_editGridSize->create(containerInner, L"2048", ui::WsClientBorder, new ui::NumericEditValidator(false, 1, 16*1024, 0));

	Ref< ui::Static > staticExtent = new ui::Static();
	staticExtent->create(containerInner, i18n::Text(L"NEW_HEIGHTFIELD_EXTENT"));

	Ref< ui::Container > containerExtent = new ui::Container();
	containerExtent->create(containerInner, ui::WsNone, new ui::TableLayout(L"*,*,*", L"*", 0, 4));

	m_editExtent[0] = new ui::Edit();
	m_editExtent[0]->create(containerExtent, L"1024", ui::WsClientBorder, new ui::NumericEditValidator(true, 1.0f));

	m_editExtent[1] = new ui::Edit();
	m_editExtent[1]->create(containerExtent, L"512", ui::WsClientBorder, new ui::NumericEditValidator(true, 1.0f));

	m_editExtent[2] = new ui::Edit();
	m_editExtent[2]->create(containerExtent, L"1024", ui::WsClientBorder, new ui::NumericEditValidator(true, 1.0f));

	Ref< ui::Static > staticPlane = new ui::Static();
	staticPlane->create(containerInner, i18n::Text(L"NEW_HEIGHTFIELD_PLANE"));

	Ref< ui::Container > containerPlane = new ui::Container();
	containerPlane->create(containerInner, ui::WsNone, new ui::TableLayout(L"*,*,*", L"*", 0, 4));

	m_editPlane[0] = new ui::Edit();
	m_editPlane[0]->create(containerPlane, L"0", ui::WsClientBorder, new ui::NumericEditValidator(true, -45.0f, 45.0f));

	m_editPlane[1] = new ui::Edit();
	m_editPlane[1]->create(containerPlane, L"0", ui::WsClientBorder, new ui::NumericEditValidator(true, -45.0f, 45.0f));

	m_editPlane[2] = new ui::Edit();
	m_editPlane[2]->create(containerPlane, L"0.5", ui::WsClientBorder, new ui::NumericEditValidator(true));

	return true;
}

int32_t NewHeightfieldDialog::getGridSize()
{
	return parseString< int32_t >(m_editGridSize->getText());
}

Vector4 NewHeightfieldDialog::getWorldExtent()
{
	return Vector4(
		parseString< float >(m_editExtent[0]->getText()),
		parseString< float >(m_editExtent[1]->getText()),
		parseString< float >(m_editExtent[2]->getText())
	);
}

Plane NewHeightfieldDialog::getWorldPlane()
{
	float ax = parseString< float >(m_editPlane[0]->getText());
	float az = parseString< float >(m_editPlane[1]->getText());
	float y = parseString< float >(m_editPlane[2]->getText());

	ax = std::sin(deg2rad(ax));
	az = (1.0f - ax) * std::sin(deg2rad(az));

	float ay = 1.0f - std::sqrt(ax * ax + az * az);

	return Plane(
		Vector4(ax, ay, az, 0.0f),
		Vector4(0.0f, y, 0.0f, 1.0f)
	);
}

	}
}
