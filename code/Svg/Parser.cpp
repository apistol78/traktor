/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <locale>
#include <sstream>
#include "Core/Containers/StaticVector.h"
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Misc/Base64.h"
#include "Core/Misc/Split.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Drawing/Image.h"
#include "Svg/Document.h"
#include "Svg/Gradient.h"
#include "Svg/ImageShape.h"
#include "Svg/Parser.h"
#include "Svg/Path.h"
#include "Svg/PathShape.h"
#include "Svg/Shape.h"
#include "Svg/Style.h"
#include "Svg/TextShape.h"
#include "Xml/Attribute.h"
#include "Xml/Document.h"
#include "Xml/Element.h"

namespace traktor::svg
{
	namespace
	{

const struct { const wchar_t* name; Color4f color; } c_colorTable[] =
{
	L"black",	Color4f( 0.0f, 0.0f, 0.0f, 1.0f),
	L"red",		Color4f( 1.0f, 0.0f, 0.0f, 1.0f),
	L"green",	Color4f( 0.0f, 1.0f, 0.0f, 1.0f),
	L"blue",	Color4f( 0.0f, 0.0f, 1.0f, 1.0f),
	L"yellow",	Color4f( 1.0f, 1.0f, 0.0f, 1.0f),
	L"white",	Color4f( 1.0f, 1.0f, 1.0f, 1.0f),
	L"lime",	Color4f(0.25f, 0.5f, 1.0f, 1.0f)
};

bool parseColor(const std::wstring& color, Color4f& outColor)
{
	if (startsWith(color, L"#"))
	{
		int32_t red, green, blue;
		swscanf(color.c_str(), L"#%02x%02x%02x", &red, &green, &blue);
		outColor = Color4f(red / 255.0f, green / 255.0f, blue / 255.0f, 1.0f);
		return true;
	}
	else if (startsWith(color, L"rgb"))
	{
		int32_t red, green, blue;
		swscanf(color.c_str(), L"rgb(%d,%d,%d)", &red, &green, &blue);
		outColor = Color4f(red / 255.0f, green / 255.0f, blue / 255.0f, 1.0f);
		return true;
	}
	else if (toLower(color) == L"none")
		return false;

	for (int32_t i = 0; i < sizeof(c_colorTable) / sizeof(c_colorTable[0]); ++i)
	{
		if (toLower(color) == c_colorTable[i].name)
		{
			outColor = c_colorTable[i].color;
			return true;
		}
	}

	log::warning << L"Unknown color \"" << color << L"\"" << Endl;
	return false;
}

bool isWhiteSpace(wchar_t ch)
{
	return bool(ch == L' ' || ch == L'\t' || ch == L'\r' || ch == L'\n' || ch == L',');
}

bool isDigit(wchar_t ch)
{
	return bool(std::wstring(L"0123456789").find(ch) != std::wstring::npos);
}

bool isCommand(wchar_t ch)
{
	return bool(std::wstring(L"MLVHQTCSAZ").find(toupper(ch)) != std::wstring::npos);
}

void skipUntil(std::wstring::iterator& i, std::wstring::iterator end, bool (*isProc)(wchar_t))
{
	while (i != end && !isProc(*i))
		++i;
}

void skipUntilNot(std::wstring::iterator& i, std::wstring::iterator end, bool (*isProc)(wchar_t))
{
	while (i != end && isProc(*i))
		++i;
}

float parseDecimalNumber(std::wstring::iterator& i, std::wstring::iterator end)
{
	while (i != end && (isWhiteSpace(*i) || *i == '.' || *i == ','))
		++i;

	float number = 0.0f;
	try
	{
		size_t n = 0;
		number = std::stof(std::wstring(i, end), &n);
		i += n;
	}
	catch(const std::exception& e)
	{
		log::error << L"Exception while parsing number; \"" << std::wstring(i, end) << L"\"." << Endl;
		i = end;
	}

	return number;
}

bool parseUrl(const std::wstring& value, std::wstring& outURL)
{
	const size_t s = value.find(L"url(#");
	const size_t e = value.find(L")");
	if (s != value.npos && e != value.npos)
	{
		outURL = value.substr(s + 5, e - s - 5);
		return true;
	}
	else
		return false;
}

float parseNumber(const std::wstring& value, float defaultValue = 0.0f)
{
	try
	{
		return std::stof(value);
	}
	catch(const std::exception& e)
	{
		return defaultValue;
	}
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.svg.Parser", Parser, Object)

Parser::Parser()
{
	m_defaultStyle = new Style();
}

Ref< Shape > Parser::parse(xml::Document* doc)
{
	return traverse(doc->getDocumentElement());
}

Ref< Shape > Parser::parse(const traktor::Path& fileName)
{
	xml::Document xd;
	if (xd.loadFromFile(fileName))
		return parse(&xd);
	else
		return nullptr;
}

Ref< Shape > Parser::traverse(xml::Element* elm)
{
	Ref< Shape > shape;

	const std::wstring name = elm->getName();
	if (name == L"svg")
		shape = parseDocument(elm);
	else if(name == L"g")
		shape = parseGroup(elm);
	else if (name == L"circle")
		shape = parseCircle(elm);
	else if (name == L"ellipse")
		shape = parseEllipse(elm);
	else if (name == L"rect")
		shape = parseRect(elm);
	else if (name == L"polygon")
		shape = parsePolygon(elm);
	else if (name == L"polyline")
		shape = parsePolyLine(elm);
	else if (name == L"path")
		shape = parsePath(elm);
	else if (name == L"text")
		shape = parseText(elm);
	else if (name == L"image")
		shape = parseImage(elm);
	else if (name == L"defs")
		parseDefs(elm);
	else
		log::debug << L"Unknown SVG element \"" << name << L"\"" << Endl;

	if (shape)
	{
		for (const xml::Attribute* attr = elm->getFirstAttribute(); attr != nullptr; attr = attr->getNext())
			shape->setAttribute(attr->getName(), Any::fromString(attr->getValue()));

		if (!shape->getStyle())
			shape->setStyle(parseStyle(elm));
		if (!shape->getStyle())
			shape->setStyle(m_defaultStyle);

		shape->setTransform(parseTransform(elm, L"transform"));

		for (xml::Node* child = elm->getFirstChild(); child; child = child->getNextSibling())
		{
			if (!is_a< xml::Element >(child))
				continue;

			Ref< Shape > childShape = traverse(static_cast< xml::Element* >(child));
			if (childShape)
				shape->addChild(childShape);
		}
	}

	return shape;
}

Ref< Shape > Parser::parseDocument(xml::Element* elm)
{
	Ref< Document > doc = new Document();

	const float width = parseString< float >(elm->getAttribute(L"width", L"0")->getValue());
	const float height = parseString< float >(elm->getAttribute(L"height", L"0")->getValue());
	doc->setSize(Vector2(width, height));

	if (elm->hasAttribute(L"viewBox"))
	{
		std::wstring viewBox = elm->getAttribute(L"viewBox")->getValue();
		std::wstring::iterator i = viewBox.begin();

		const float left = parseDecimalNumber(i, viewBox.end());
		const float top = parseDecimalNumber(i, viewBox.end());
		const float width = parseDecimalNumber(i, viewBox.end());
		const float height = parseDecimalNumber(i, viewBox.end());

		doc->setViewBox(Aabb2(
			Vector2(left, top),
			Vector2(left + width, top + height)
		));
	}

	Ref< Style > defaultStyle = new Style();
	defaultStyle->setFillEnable(true);
	defaultStyle->setFill(Color4f(0.0f, 0.0f, 0.0f, 1.0f));
	doc->setStyle(defaultStyle);

	return doc;
}

Ref< Shape > Parser::parseGroup(xml::Element* elm)
{
	return new Shape();
}

Ref< Shape > Parser::parseCircle(xml::Element* elm)
{
	const float c_circleMagic = 0.5522847498f;

	const float cx = parseAttr(elm, L"cx");
	const float cy = parseAttr(elm, L"cy");
	const float r = parseAttr(elm, L"r");
	const float rk = r * c_circleMagic;

	Path path;

	path.moveTo(cx, cy + r);
	path.cubicTo(
		cx - rk, cy + r,
		cx - r, cy + rk,
		cx - r, cy
	);
	path.cubicTo(
		cx - r, cy - rk,
		cx - rk, cy - r,
		cx, cy - r
	);
	path.cubicTo(
		cx + rk, cy - r,
		cx + r, cy - rk,
		cx + r, cy
	);
	path.cubicTo(
		cx + r, cy + rk,
		cx + rk, cy + r,
		cx, cy + r
	);
	path.close();

	return new PathShape(path);
}

Ref< Shape > Parser::parseEllipse(xml::Element* elm)
{
	const float c_circleMagic = 0.5522847498f;

	const float cx = parseAttr(elm, L"cx");
	const float cy = parseAttr(elm, L"cy");
	const float rx = parseAttr(elm, L"rx");
	const float ry = parseAttr(elm, L"ry");
	const float rxk = rx * c_circleMagic;
	const float ryk = ry * c_circleMagic;

	Path path;

	path.moveTo(cx, cy + ry);
	path.cubicTo(
		cx - rxk, cy + ry,
		cx - rx, cy + ryk,
		cx - rx, cy
	);
	path.cubicTo(
		cx - rx, cy - ryk,
		cx - rxk, cy - ry,
		cx, cy - ry
	);
	path.cubicTo(
		cx + rxk, cy - ry,
		cx + rx, cy - ryk,
		cx + rx, cy
	);
	path.cubicTo(
		cx + rx, cy + ryk,
		cx + rxk, cy + ry,
		cx, cy + ry
	);
	path.close();

	return new PathShape(path);
}

Ref< Shape > Parser::parseRect(const xml::Element* elm)
{
	const float x = parseAttr(elm, L"x");
	const float y = parseAttr(elm, L"y");
	const float width = parseAttr(elm, L"width");
	const float height = parseAttr(elm, L"height");
	const float round = parseAttr(elm, L"ry");

	Path path;

	if (round <= 0.0f)
	{
		path.moveTo(x, y);
		path.lineTo(x + width, y);
		path.lineTo(x + width, y + height);
		path.lineTo(x, y + height);
		path.close ();
	}
	else
	{
		path.moveTo   (x + round, y);
		path.lineTo   (x + width - round, y);
		path.quadricTo(x + width, y, x + width, y + round);
		path.lineTo   (x + width, y + height - round);
		path.quadricTo(x + width, y + height, x + width - round, y + height);
		path.lineTo   (x + round, y + height);
		path.quadricTo(x, y + height, x, y + height - round);
		path.lineTo   (x, y + round);
		path.quadricTo(x, y, x + round, y);
		path.close    ();
	}

	return new PathShape(path);
}

Ref< Shape > Parser::parsePolygon(xml::Element* elm)
{
	Path path;
	bool first = true;

	std::wstring points = trim(elm->getAttribute(L"points")->getValue());
	std::wstring::iterator i = points.begin();
	while (i != points.end())
	{
		const float x = parseDecimalNumber(i, points.end());
		const float y = parseDecimalNumber(i, points.end());

		if (first)
			path.moveTo(x, y);
		else
			path.lineTo(x, y);

		first = false;
	}

	path.close();

	return new PathShape(path);
}

Ref< Shape > Parser::parsePolyLine(xml::Element* elm)
{
	if (!elm || !elm->hasAttribute(L"points"))
		return nullptr;

	Path path;
	bool first = true;

	std::wstring points = trim(elm->getAttribute(L"points")->getValue());
	std::wstring::iterator i = points.begin();
	while (i != points.end())
	{
		const float x = parseDecimalNumber(i, points.end());
		const float y = parseDecimalNumber(i, points.end());

		if (first)
			path.moveTo(x, y);
		else
			path.lineTo(x, y);

		first = false;
	}

	return new PathShape(path);
}

Ref< Shape > Parser::parsePath(xml::Element* elm)
{
	if (!elm || !elm->hasAttribute(L"d"))
		return nullptr;

	std::wstring def = trim(elm->getAttribute(L"d")->getValue());
	std::wstring::iterator i = def.begin();
	wchar_t cmdLead = 0;

	Path path;
	while (i != def.end())
	{
		skipUntilNot(i, def.end(), isWhiteSpace);
		if (i == def.end())
			break;

		wchar_t cmd = *i;
		if (!isCommand(cmd))
		{
			// No command; assume shorthand expressions.
			if (toupper(cmdLead) == L'M')
				cmd = isupper(cmdLead) ? L'L' : L'l';
			else if (toupper(cmdLead) == L'C')
				cmd = isupper(cmdLead) ? L'C' : L'c';
			else if (toupper(cmdLead) == L'L')
				cmd = isupper(cmdLead) ? L'L' : L'l';
			else if (toupper(cmdLead) == L'Q')
				cmd = isupper(cmdLead) ? L'Q' : L'q';
			else if (toupper(cmdLead) == L'A')
				cmd = isupper(cmdLead) ? L'A' : L'a';
			else if (toupper(cmdLead) == L'H')
				cmd = isupper(cmdLead) ? L'H' : L'h';
			else if (toupper(cmdLead) == L'V')
				cmd = isupper(cmdLead) ? L'V' : L'v';
			else
			{
				log::info << L"Invalid path, lead command \"" << cmdLead << L"\"." << Endl;
				return nullptr;
			}
		}
		else
		{
			cmdLead = cmd;
			++i;
		}

		const bool relative = islower(cmd);
		switch (toupper(cmd))
		{
		case L'M':	// Move to
			{
				const float x = parseDecimalNumber(i, def.end());
				const float y = parseDecimalNumber(i, def.end());
				path.moveTo(x, y, relative);
			}
			break;

		case L'L':	// Line to
			{
				const float x = parseDecimalNumber(i, def.end());
				const float y = parseDecimalNumber(i, def.end());
				path.lineTo(x, y, relative);
			}
			break;

		case L'V':	// Vertical line to
			{
				const float y = parseDecimalNumber(i, def.end());
				if (relative)
					path.lineTo(0, y, true);
				else
					path.lineTo(path.getCursor().x, y, false);
			}
			break;

		case L'H':	// Horizontal line to
			{
				const float x = parseDecimalNumber(i, def.end());
				if (relative)
					path.lineTo(x, 0, true);
				else
					path.lineTo(x, path.getCursor().y, false);
			}
			break;

		case L'Q':	// Quadric to
			{
				const float x1 = parseDecimalNumber(i, def.end());
				const float y1 = parseDecimalNumber(i, def.end());
				const float x2 = parseDecimalNumber(i, def.end());
				const float y2 = parseDecimalNumber(i, def.end());
				path.quadricTo(x1, y1, x2, y2, relative);
			}
			break;

		case L'T':	// Quadric to (shorthand/smooth)
			{
				const float x = parseDecimalNumber(i, def.end());
				const float y = parseDecimalNumber(i, def.end());
				path.quadricTo(x, y, relative);
				log::warning << L"Shorthand quadric not implemented properly!" << Endl;
			}
			break;

		case L'C':	// Cubic to
			{
				const float x1 = parseDecimalNumber(i, def.end());
				const float y1 = parseDecimalNumber(i, def.end());
				const float x2 = parseDecimalNumber(i, def.end());
				const float y2 = parseDecimalNumber(i, def.end());
				const float x = parseDecimalNumber(i, def.end());
				const float y = parseDecimalNumber(i, def.end());
				path.cubicTo(x1, y1, x2, y2, x, y, relative);
			}
			break;

		case L'S':	// Cubic to (shorthand/smooth)
			{
				const float x1 = parseDecimalNumber(i, def.end());
				const float y1 = parseDecimalNumber(i, def.end());
				const float x2 = parseDecimalNumber(i, def.end());
				const float y2 = parseDecimalNumber(i, def.end());
				path.cubicTo(x1, y1, x2, y2, relative);
			}
			break;

		case L'A':	// Elliptic arc
			{
				const float rx = parseDecimalNumber(i, def.end());
				const float ry = parseDecimalNumber(i, def.end());
				const float rotation = parseDecimalNumber(i, def.end());
				const float la = parseDecimalNumber(i, def.end());
				const float sf = parseDecimalNumber(i, def.end());
				const float x = parseDecimalNumber(i, def.end());
				const float y = parseDecimalNumber(i, def.end());
				path.lineTo(x, y, relative);
			}
			break;

		case L'Z':	// Close sub path
			path.close();
			break;

		default:
			log::error << L"Unknown path command character \"" << *i << L"\"" << Endl;
		}
	}

	return new PathShape(path);
}

Ref< Shape > Parser::parseText(xml::Element* elm)
{
	const float x = parseAttr(elm, L"x");
	const float y = parseAttr(elm, L"y");

	StringOutputStream ss;
	for (xml::Node* child = elm->getFirstChild(); child; child = child->getNextSibling())
	{
		if (auto ts = dynamic_type_cast< xml::Element* >(child))
		{
			if (ts->getName() == L"tspan")
				ss << ts->getValue();
		}
	}

	return new TextShape(Vector2(x, y), ss.str());
}

Ref< Shape > Parser::parseImage(xml::Element* elm)
{
	const float x = parseAttr(elm, L"x");
	const float y = parseAttr(elm, L"y");
	const float width = parseAttr(elm, L"width");
	const float height = parseAttr(elm, L"height");

	Ref< drawing::Image > image;

	// Embedded image.
	const xml::Attribute* href = elm->getAttribute(L"xlink:href");
	if (href)
	{
		const std::wstring& value = href->getValue();
		if (startsWith(value, L"data:image/png;base64,"))
		{
			AlignedVector< uint8_t > data = Base64().decode(value.substr(22));
			image = drawing::Image::load(data.c_ptr(), data.size(), L"png");
		}
	}

	if (!image)
		return nullptr;

	return new ImageShape(Vector2(x, y), Vector2(width, height), image);
}

void Parser::parseDefs(xml::Element* elm)
{
	for (xml::Node* child = elm->getFirstChild(); child; child = child->getNextSibling())
	{
		if (!is_a< xml::Element >(child))
			continue;

		const xml::Element* ch = static_cast< const xml::Element* >(child);
		if (!ch->hasAttribute(L"id"))
		{
			log::warning << L"Invalid definition, no \"id\" attribute" << Endl;
			continue;
		}

		const std::wstring name = ch->getName();
		const std::wstring id = ch->getAttribute(L"id")->getValue();

		if (name == L"rect")
		{
			Ref< Shape > rect = parseRect(ch);
			if (rect)
				m_shapeDefs[id] = rect;
		}
		else if (name == L"linearGradient" || name == L"radialGradient")
		{
			Ref< Gradient > gradient = parseGradientDef(elm, ch);
			if (gradient)
				m_gradients[id] = gradient;
		}
		else
			log::debug << L"Unknown definition element \"" << name << L"\"" << Endl;
	}
}

Ref< Gradient > Parser::parseGradientDef(const xml::Element* defs, const xml::Element* elm) const
{
	const std::wstring name = elm->getName();
	const std::wstring id = elm->getAttribute(L"id")->getValue();

	if (name != L"linearGradient" && name != L"radialGradient")
		return nullptr;

	Ref< Gradient > gradient = new Gradient(name == L"linearGradient" ? Gradient::GtLinear : Gradient::GtRadial);

	if (elm->hasAttribute(L"xlink:href"))
	{
		const std::wstring ref = elm->getAttribute(L"xlink:href")->getValue().substr(1);
		const xml::Element* xref = defs->getSingle(str(L"*[@id=%S]", ref.c_str()));
		if (xref != nullptr)
		{
			Ref< Gradient > refGradient = parseGradientDef(defs, xref);
			if (refGradient)
				gradient->setStops(refGradient->getStops());
		}
		else
			log::error << L"Unable to resolve reference \"" << ref << L"\"; no such element." << Endl;
	}

	RefArray< xml::Element > stops;
	elm->get(L"stop", stops);
	for (auto stop : stops)
	{
		if (!stop->hasAttribute(L"offset"))
			continue;

		float offset;
		std::wstringstream(stop->getAttribute(L"offset")->getValue()) >> offset;

		Color4f color(0.0f, 0.0f, 0.0f, 1.0f);
		if (stop->hasAttribute(L"stop-color"))
			parseColor(stop->getAttribute(L"stop-color")->getValue(), color);
		else if (stop->hasAttribute(L"style"))
		{
			std::vector< std::wstring > styles;
			Split< std::wstring >::any(stop->getAttribute(L"style")->getValue(), L";", styles);
			for (const auto& st : styles)
			{
				std::wstring::size_type i = st.find(L':');
				if (i == std::string::npos)
					continue;

				const std::wstring key = trim(st.substr(0, i));
				const std::wstring value = trim(st.substr(i + 1));

				if (key == L"stop-color")
				{
					Color4f stopColor;
					if (parseColor(value, stopColor))
					{
						stopColor.setAlpha(color.getAlpha());
						color = stopColor;
					}
				}
				else if (key == L"stop-opacity")
				{
					const float opacity = parseNumber(value);
					color.setAlpha(Scalar(opacity));
				}
			}
		}

		gradient->addStop(offset, color);
	}

	const float x1 = parseAttr(elm, L"x1");
	const float y1 = parseAttr(elm, L"y1");
	const float x2 = parseAttr(elm, L"x2");
	const float y2 = parseAttr(elm, L"y2");
	gradient->setBounds(Aabb2(Vector2(x1, y1), Vector2(x2, y2)));

	const Matrix33 gradientTransform = parseTransform(elm, L"gradientTransform");
	gradient->setTransform(gradientTransform);

	return gradient;
}

Ref< Style > Parser::parseStyle(xml::Element* elm)
{
	if (!elm)
		return nullptr;

	Ref< Style > style;
	Color4f color;

	if (elm->hasAttribute(L"fill"))
	{
		style = new Style();

		const std::wstring fillDesc = elm->getAttribute(L"fill")->getValue();
		if (parseColor(fillDesc, color))
		{
			style->setFillEnable(true);
			style->setFill(color);
		}
		else
			style->setFillEnable(false);
	}
	else if (elm->hasAttribute(L"style"))
	{
		style = new Style();

		std::vector< std::wstring > styles;
		Split< std::wstring >::any(elm->getAttribute(L"style")->getValue(), L";", styles);
		for (const auto& st : styles)
		{
			std::wstring::size_type i = st.find(L':');
			if (i == std::string::npos)
				continue;

			const std::wstring key = trim(st.substr(0, i));
			const std::wstring value = trim(st.substr(i + 1));

			if (key == L"display")
				;
			else if (key == L"opacity")
			{
				const float opacity = parseNumber(value);
				style->setOpacity(opacity);
			}
			else if (key == L"fill")
			{
				std::wstring id;
				if (parseUrl(value, id))
				{
					auto it = m_gradients.find(id);
					if (it != m_gradients.end())
					{
						style->setFillEnable(true);
						style->setFillGradient(it->second);
					}
					else
						log::error << L"Invalid style; no such gradient \"" << id << L"\"." << Endl;
				}
				else if (parseColor(value, color))
				{
					style->setFillEnable(true);

					const Color4f fillColor = style->getFill();
					color.setAlpha(fillColor.getAlpha());
					style->setFill(color);
				}
				else
					style->setFillEnable(false);
			}
			else if (key == L"fill-rule")
				;
			else if (key == L"fill-opacity")
			{
				const float fillOpacity = parseNumber(value);
				Color4f fillColor = style->getFill();
				fillColor.setAlpha(Scalar(fillOpacity));
				style->setFill(fillColor);
			}
			else if (key == L"shape-inside")
			{
				std::wstring id;
				if (parseUrl(value, id))
				{
					const auto it = m_shapeDefs.find(id);
					if (it != m_shapeDefs.end())
						style->setShapeInside(it->second);
					else
						log::warning << L"Unable to set \"shape-inside\"; no such def \"" << id << L"\"." << Endl;
				}
			}
			else if (key == L"stroke")
			{
				if (parseColor(value, color))
				{
					style->setStrokeEnable(true);

					const Color4f strokeColor = style->getStroke();
					color.setAlpha(strokeColor.getAlpha());
					style->setStroke(color);
				}
				else
					style->setStrokeEnable(false);
			}
			else if (key == L"stroke-width")
			{
				const float strokeWidth = parseNumber(value);
				style->setStrokeWidth(strokeWidth);
			}
			else if (key == L"stroke-dasharray")
				;
			else if (key == L"stroke-dashoffset")
				;
			else if (key == L"stroke-opacity")
			{
				const float strokeOpacity = parseNumber(value);
				Color4f strokeColor = style->getStroke();
				strokeColor.setAlpha(Scalar(strokeOpacity));
				style->setStroke(strokeColor);
			}
			else if (key == L"stroke-linecap")
				;
			else if (key == L"stroke-linejoin")
				;
			else if (key == L"stroke-miterlimit")
				;
			else if (key == L"font-family")
				style->setFontFamily(value);
			else if (key == L"font-size")
			{
				const float fontSize = parseNumber(value);
				style->setFontSize(fontSize);
			}
			else
				log::debug << L"Unknown CSS style \"" << key << L"\"" << Endl;
		}
	}

	return style;
}

Matrix33 Parser::parseTransform(const xml::Element* elm, const std::wstring& attrName) const
{
	if (!elm || !elm->hasAttribute(attrName))
		return Matrix33::identity();

	Matrix33 transform = Matrix33::identity();

	std::wstring transformDesc = elm->getAttribute(attrName)->getValue();
	std::wstring::iterator i = transformDesc.begin();

	while (i != transformDesc.end())
	{
		skipUntilNot(i, transformDesc.end(), isWhiteSpace);

		std::wstring::iterator j = i;

		while (i != transformDesc.end() && *i != L'(')
			++i;

		if (i == transformDesc.end())
			break;

		std::wstring fnc(j, i);

		j = ++i;

		while (i != transformDesc.end() && *i != L')')
			++i;

		if (i == transformDesc.end())
			break;

		std::wstring args(j, i);

		if (fnc == L"matrix")
		{
			StaticVector< float, 6 > argv;
			Split< std::wstring, float >::any(args, L",", argv, false, 6);

			if (argv.size() >= 6)
				transform = transform * Matrix33(
					argv[0], argv[1], 0.0f,
					argv[2], argv[3], 0.0f,
					argv[4], argv[5], 1.0f
				).transpose();
		}
		else if (fnc == L"translate")
		{
			StaticVector< float, 2 > argv;
			Split< std::wstring, float >::any(args, L",", argv, false, 2);

			if (argv.size() >= 2)
				transform = transform * translate(argv[0], argv[1]);
			else if (argv.size() >= 1)
				transform = transform * translate(argv[0], 0.0f);
		}
		else if (fnc == L"rotate")
		{
			StaticVector< float, 1 > argv;
			Split< std::wstring, float >::any(args, L",", argv, false, 1);

			if (argv.size() >= 1)
				transform = transform * rotate(deg2rad(argv[0]));
		}
		else if (fnc == L"scale")
		{
			StaticVector< float, 2 > argv;
			Split< std::wstring, float >::any(args, L",", argv, false, 2);

			if (argv.size() >= 2)
				transform = transform * scale(argv[0], argv[1]);
			else if (argv.size() >= 1)
				transform = transform * scale(argv[0], 0.0f);
		}
		else
			log::error << L"Unknown transform function \"" << fnc << L"\"" << Endl;

		++i;
	}

	return transform;
}

float Parser::parseAttr(const xml::Element* elm, const std::wstring& attrName, float defValue) const
{
	if (elm && elm->hasAttribute(attrName))
	{
		std::wstring attrValue = elm->getAttribute(attrName)->getValue();
		std::wstringstream ss(attrValue);
		ss >> defValue;
	}
	return defValue;
}

}
