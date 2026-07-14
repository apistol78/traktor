/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "MCP/Editor/Tools/RenderMeshPreviewTool.h"

#include "Core/Containers/AlignedVector.h"
#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Math/Aabb3.h"
#include "Core/Math/Color4f.h"
#include "Core/Misc/Base64.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/ISerializable.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Editor/IEditor.h"
#include "MCP/Editor/McpToolSupport.h"
#include "MCP/Editor/MeshAssetSupport.h"
#include "MCP/Editor/Json.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Mesh/Editor/MeshAssetRasterizer.h"

#include <vector>

namespace traktor::mcp
{
namespace
{

// One render per quadrant of the 2x2 contact sheet, row-major. Angles in
// radians (yaw turns about the vertical axis, pitch tilts the camera down).
struct View
{
	const wchar_t* label;
	float yaw;
	float pitch;
};

const float c_halfPi = 1.5708f;
const int32_t c_cols = 2;
const int32_t c_rows = 2;
const Color4f c_background(0.22f, 0.22f, 0.24f, 1.0f);

// Fallback used when the bounding box is unavailable: classic four views.
const View c_defaultViews[] = {
	{ L"front", 0.0f, 0.0f },
	{ L"right", c_halfPi, 0.0f },
	{ L"top-down", 0.0f, 1.40f },
	{ L"three-quarter", 0.7854f, 0.45f }
};

/*! Pick four camera angles that present the model well for its proportions.
 *
 * The "hero" view looks along the model's thinnest axis, so its largest
 * cross-section faces the camera; the others add a three-quarter, a profile and
 * a top-down. This stops a flat or elongated piece (a disc, a wall, a floor
 * tile) from collapsing into several identical edge-on cells. */
std::vector< View > chooseViews(const Aabb3& bbox)
{
	const Vector4 size = bbox.mx - bbox.mn;
	const float sx = (float)size.x();
	const float sy = (float)size.y();
	const float sz = (float)size.z();

	std::vector< View > views;
	if (sy <= sx && sy <= sz)
	{
		// Thinnest vertically (lies flat: floor tile, rug, debris). The footprint
		// is the hero; tilt the rest up so the thin profile still reads.
		views.push_back({ L"top-down", 0.0f, 1.40f });
		views.push_back({ L"high 3/4", 0.61f, 0.92f });
		views.push_back({ L"high 3/4 (alt)", c_halfPi, 0.92f });
		views.push_back({ L"low profile", 0.0f, 0.26f });
	}
	else
	{
		// Thinnest along X or Z: look along that axis so the broad face fills the
		// frame, then rotate/tilt for depth and footprint.
		const float faceYaw = (sx < sz) ? c_halfPi : 0.0f;
		views.push_back({ L"face", faceYaw, 0.0f });
		views.push_back({ L"three-quarter", faceYaw + 0.61f, 0.42f });
		views.push_back({ L"profile", faceYaw + c_halfPi, 0.0f });
		views.push_back({ L"top-down", faceYaw, 1.40f });
	}
	return views;
}

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.mcp.RenderMeshPreviewTool", RenderMeshPreviewTool, IMcpTool)

RenderMeshPreviewTool::RenderMeshPreviewTool(editor::IEditor* editor)
	: m_editor(editor)
{
}

std::wstring RenderMeshPreviewTool::getName() const
{
	return L"render_mesh_preview";
}

std::wstring RenderMeshPreviewTool::getDescription() const
{
	return L"Render a mesh asset so you can see its shape. Returns a PNG image: a 2x2 contact sheet of the mesh from four camera angles chosen to suit its proportions, plus its source-mesh footprint (dimensions and pivot) in the accompanying text. Use this to understand what a model actually represents - its silhouette, proportions and how it might fit together with other pieces - beyond its name and material slots.";
}

Ref< Json > RenderMeshPreviewTool::getInputSchema() const
{
	Ref< Json > properties = meshTargetProperties();

	Ref< Json > sizeProperty = Json::createObject();
	sizeProperty->setString(L"type", L"integer");
	sizeProperty->setString(L"description", L"Pixel size of each of the four views (clamped to 128..512, default 384). The returned image is twice this in each dimension.");
	properties->set(L"size", sizeProperty);

	Ref< Json > schema = Json::createObject();
	schema->setString(L"type", L"object");
	schema->set(L"properties", properties);
	schema->set(L"required", Json::createArray());
	return schema;
}

Ref< Json > RenderMeshPreviewTool::invoke(const Json* arguments, std::wstring& outError)
{
	Ref< db::Database > database = m_editor->getSourceDatabase();
	if (!database)
	{
		outError = L"No source database is currently open.";
		return nullptr;
	}

	Ref< db::Instance > instance = resolveInstance(database, arguments, outError);
	if (!instance)
	{
		if (outError.empty())
			outError = L"No mesh asset instance found for the given guid/path.";
		return nullptr;
	}

	Ref< ISerializable > object;
	mesh::MeshAsset* meshAsset = loadMeshAsset(instance, object, outError);
	if (!meshAsset)
		return nullptr;

	int32_t cell = 384;
	if (arguments && arguments->getMember(L"size"))
		cell = (int32_t)arguments->getMember(L"size")->getNumber(384);
	if (cell < 128)
		cell = 128;
	if (cell > 512)
		cell = 512;

	// Read the footprint first: it drives both view selection and the reported
	// dimensions/pivot.
	const mesh::MeshAssetRasterizer rasterizer;
	Aabb3 boundingBox;
	const bool haveBox = rasterizer.getBoundingBox(m_editor, meshAsset, boundingBox);

	std::vector< View > views;
	if (haveBox)
		views = chooseViews(boundingBox);
	else
		views.assign(c_defaultViews, c_defaultViews + sizeof(c_defaultViews) / sizeof(c_defaultViews[0]));

	// Composite each view into a quadrant of the contact sheet.
	Ref< drawing::Image > sheet = new drawing::Image(drawing::PixelFormat::getR8G8B8A8(), cell * c_cols, cell * c_rows);
	sheet->clear(c_background);

	int32_t rendered = 0;
	for (int32_t i = 0; i < (int32_t)views.size(); ++i)
	{
		Ref< drawing::Image > view = new drawing::Image(drawing::PixelFormat::getR8G8B8A8(), cell, cell);
		view->clear(c_background);

		if (!rasterizer.generate(m_editor, meshAsset, views[i].yaw, views[i].pitch, view))
			continue;

		const int32_t dx = (i % c_cols) * cell;
		const int32_t dy = (i / c_cols) * cell;
		sheet->copy(view, dx, dy, 0, 0, cell, cell);
		++rendered;
	}

	if (rendered == 0)
	{
		outError = L"Failed to rasterize mesh; the source model file may be missing or unreadable.";
		return nullptr;
	}

	// Encode the contact sheet as PNG, in memory.
	AlignedVector< uint8_t > pngData;
	DynamicMemoryStream pngStream(pngData, false, true);
	if (!sheet->save(&pngStream, L"png"))
	{
		outError = L"Failed to encode preview image as PNG.";
		return nullptr;
	}
	pngStream.close();

	const std::wstring base64 = Base64().encode(pngData);

	// Describe the layout (and footprint) so the model can map quadrants to view
	// directions and reason about scale.
	std::wstring text =
		L"Multi-view render of mesh '" + instance->getName() + L"' (" + instance->getGuid().format() + L"). "
																									   L"2x2 contact sheet, each cell " +
		toString(cell) + L"px. Views, row-major: "
						 L"top-left=" +
		views[0].label + L", top-right=" + views[1].label +
		L", bottom-left=" + views[2].label + L", bottom-right=" + views[3].label + L". ";

	if (haveBox)
	{
		const Vector4 size = boundingBox.mx - boundingBox.mn;
		text +=
			L"Footprint (model units, WxHxD): " +
			toString((float)size.x()) + L" x " + toString((float)size.y()) + L" x " + toString((float)size.z()) +
			L". Pivot: " + pivotLabel(boundingBox) + L". ";
	}

	text += L"The flat gray is the backdrop, not part of the model.";

	Ref< Json > textBlock = Json::createObject();
	textBlock->setString(L"type", L"text");
	textBlock->setString(L"text", text);

	Ref< Json > imageBlock = Json::createObject();
	imageBlock->setString(L"type", L"image");
	imageBlock->setString(L"data", base64);
	imageBlock->setString(L"mimeType", L"image/png");

	Ref< Json > content = Json::createArray();
	content->push(textBlock);
	content->push(imageBlock);
	return content;
}

}
