/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Math/Envelope.h"
#include "Core/Math/Float.h"
#include "Database/Instance.h"
#include "Drawing/Image.h"
#include "Drawing/Filters/GammaFilter.h"
#include "Render/Editor/Texture/ColorGradingAssetEditor.h"
#include "Render/Editor/Texture/ColorGradingTextureAsset.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/Container.h"
#include "Ui/Image.h"
#include "Ui/Slider.h"
#include "Ui/Static.h"
#include "Ui/TableLayout.h"
#include "Ui/Envelope/DefaultEnvelopeEvaluator.h"
#include "Ui/Envelope/EnvelopeContentChangeEvent.h"
#include "Ui/Envelope/EnvelopeControl.h"

namespace traktor::render
{
	namespace
	{

float calculateAverageIntensity(const drawing::Image* image)
{
	Scalar intensity = 0.0_simd;
	Color4f clr;

	for (int32_t y = 0; y < image->getHeight(); ++y)
	{
		for (int32_t x = 0; x < image->getWidth(); ++x)
		{
			image->getPixelUnsafe(x, y, clr);
			intensity += (clr.getRed() + clr.getAlpha() + clr.getBlue()) / 3.0_simd;
		}
	}

	return intensity / Scalar(image->getWidth() * image->getHeight());
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ColorGradingAssetEditor", ColorGradingAssetEditor, editor::IObjectEditor)

ColorGradingAssetEditor::ColorGradingAssetEditor(editor::IEditor* editor)
:	m_editor(editor)
{
}

bool ColorGradingAssetEditor::create(ui::Widget* parent, db::Instance* instance, ISerializable* object)
{
	m_instance = instance;
	m_asset = mandatory_non_null_type_cast< ColorGradingTextureAsset* >(object);

	m_referenceImage = drawing::Image::load(L"$(TRAKTOR_HOME)/resources/runtime/editor/Reference - CG.png");
	if (!m_referenceImage)
		return false;

	Ref< ui::Container > container = new ui::Container();
	container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"100%,100%,*", 0_ut, 0_ut));

	Ref< ui::Container > containerPreview = new ui::Container();
	containerPreview->create(container, ui::WsNone, new ui::TableLayout(L"100%,100%", L"100%", 0_ut, 0_ut));

	m_imageOriginal = new ui::Image();
	m_imageOriginal->create(containerPreview, new ui::Bitmap(m_referenceImage), ui::Image::WsScaleKeepAspect);

	m_imageResult = new ui::Image();
	m_imageResult->create(containerPreview, new ui::Bitmap(m_referenceImage), ui::Image::WsScaleKeepAspect);

	Ref< ui::Container > containerEnvelopes = new ui::Container();
	containerEnvelopes->create(container, ui::WsNone, new ui::TableLayout(L"100%,100%,100%", L"100%", 0_ut, 0_ut));

	m_envelopeRed = new ui::EnvelopeControl();
	m_envelopeRed->create(containerEnvelopes, new ui::DefaultEnvelopeEvaluator< HermiteEvaluator >());
	m_envelopeRed->addEventHandler< ui::EnvelopeContentChangeEvent >(this, &ColorGradingAssetEditor::eventEnvelopeChange);

	m_envelopeGreen = new ui::EnvelopeControl();
	m_envelopeGreen->create(containerEnvelopes, new ui::DefaultEnvelopeEvaluator< HermiteEvaluator >());
	m_envelopeGreen->addEventHandler< ui::EnvelopeContentChangeEvent >(this, &ColorGradingAssetEditor::eventEnvelopeChange);

	m_envelopeBlue = new ui::EnvelopeControl();
	m_envelopeBlue->create(containerEnvelopes, new ui::DefaultEnvelopeEvaluator< HermiteEvaluator >());
	m_envelopeBlue->addEventHandler< ui::EnvelopeContentChangeEvent >(this, &ColorGradingAssetEditor::eventEnvelopeChange);

	for (const auto& kv : m_asset->m_redCurve)
		m_envelopeRed->insertKey(new ui::EnvelopeKey(kv.first, kv.second, (kv.first <= 0.0f || kv.first >= 1.0f), false));
	for (const auto& kv : m_asset->m_greenCurve)
		m_envelopeGreen->insertKey(new ui::EnvelopeKey(kv.first, kv.second, (kv.first <= 0.0f || kv.first >= 1.0f), false));
	for (const auto& kv : m_asset->m_blueCurve)
		m_envelopeBlue->insertKey(new ui::EnvelopeKey(kv.first, kv.second, (kv.first <= 0.0f || kv.first >= 1.0f), false));

	Ref< ui::Container > containerSliders = new ui::Container();
	containerSliders->create(container, ui::WsNone, new ui::TableLayout(L"*,100%", L"*", 0_ut, 0_ut));

	Ref< ui::Static > staticBrightness = new ui::Static();
	staticBrightness->create(containerSliders, L"Brightness");

	m_sliderBrightness = new ui::Slider();
	m_sliderBrightness->create(containerSliders);
	m_sliderBrightness->setValue((int32_t)(m_asset->m_brightness * 100.0f));
	m_sliderBrightness->addEventHandler< ui::ContentChangeEvent >(this, &ColorGradingAssetEditor::eventSliderChange);

	Ref< ui::Static > staticContrast = new ui::Static();
	staticContrast->create(containerSliders, L"Contrast");

	m_sliderContrast = new ui::Slider();
	m_sliderContrast->create(containerSliders);
	m_sliderContrast->setRange(0, 400);
	m_sliderContrast->setValue((int32_t)(m_asset->m_contrast * 100.0f));
	m_sliderContrast->addEventHandler< ui::ContentChangeEvent >(this, &ColorGradingAssetEditor::eventSliderChange);

	Ref< ui::Static > staticSaturation = new ui::Static();
	staticSaturation->create(containerSliders, L"Saturation");

	m_sliderSaturation = new ui::Slider();
	m_sliderSaturation->create(containerSliders);
	m_sliderSaturation->setRange(0, 400);
	m_sliderSaturation->setValue((int32_t)(m_asset->m_saturation * 100.0f));
	m_sliderSaturation->addEventHandler< ui::ContentChangeEvent >(this, &ColorGradingAssetEditor::eventSliderChange);

	// Convert reference image into linear color space.
	const drawing::GammaFilter gammaFilter(2.2f, 1.0f);
	m_referenceImage->apply(&gammaFilter);

	// Calculate reference intensity.
	m_referenceIntensity = calculateAverageIntensity(m_referenceImage);

	m_resultImage = m_referenceImage->clone();
	T_FATAL_ASSERT(m_resultImage);

	updatePreview();
	return true;
}

void ColorGradingAssetEditor::destroy()
{
	m_instance = nullptr;
	m_asset = nullptr;
}

void ColorGradingAssetEditor::apply()
{
	m_instance->setObject(m_asset);
}

bool ColorGradingAssetEditor::handleCommand(const ui::Command& command)
{
	return false;
}

void ColorGradingAssetEditor::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
}

ui::Size ColorGradingAssetEditor::getPreferredSize() const
{
	return ui::Size(
		700,
		700
	);
}

void ColorGradingAssetEditor::updatePreview()
{
	Color4f clr;

	const Envelope< float > redEnvelope(m_asset->m_redCurve);
	const Envelope< float > greenEnvelope(m_asset->m_greenCurve);
	const Envelope< float > blueEnvelope(m_asset->m_blueCurve);

	const float brightness = m_asset->m_brightness;
	const float contrast = m_asset->m_contrast;
	const float saturation = m_asset->m_saturation;

	for (int32_t y = 0; y < m_referenceImage->getHeight(); ++y)
	{
		for (int32_t x = 0; x < m_referenceImage->getWidth(); ++x)
		{
			m_referenceImage->getPixelUnsafe(x, y, clr);

			// Transform color through envelopes.
			float r = redEnvelope(clr.getRed());
			float g = greenEnvelope(clr.getGreen());
			float b = blueEnvelope(clr.getBlue());

			// Apply brightness and contrast.
			r = clamp(contrast * (r - 0.5f) + 0.5f + brightness, 0.0f, 1.0f);
			g = clamp(contrast * (g - 0.5f) + 0.5f + brightness, 0.0f, 1.0f);
			b = clamp(contrast * (b - 0.5f) + 0.5f + brightness, 0.0f, 1.0f);

			// Apply saturation.
			float intensity = (r + g + b) / 3.0f;
			r = clamp(lerp(intensity, r, saturation), 0.0f, 1.0f);
			g = clamp(lerp(intensity, g, saturation), 0.0f, 1.0f);
			b = clamp(lerp(intensity, b, saturation), 0.0f, 1.0f);

			m_resultImage->setPixelUnsafe(x, y, Color4f(r, g, b, 1.0f));
		}
	}

	// Calculate difference in intensity.
	const float resultIntensity = calculateAverageIntensity(m_resultImage);

	// Convert result image into sRGB so we can preview it properly.
	const drawing::GammaFilter gammaFilter(1.0f, 2.2f);
	m_resultImage->apply(&gammaFilter);

	m_imageResult->setImage(new ui::Bitmap(m_resultImage));

	// Log difference of intensity.
	const float diff = (resultIntensity * 100.0f) / m_referenceIntensity;
	log::info << diff << L"% intensity of reference." << Endl;
}

void ColorGradingAssetEditor::eventEnvelopeChange(ui::EnvelopeContentChangeEvent* event)
{
	auto control = mandatory_non_null_type_cast< ui::EnvelopeControl* >(event->getSender());

	AlignedVector< std::pair< float, float > > kv;
	for (auto key : control->getKeys())
		kv.push_back({
			key->getT(),
			key->getValue()
		});

	if (control == m_envelopeRed)
		m_asset->m_redCurve = kv;
	else if (control == m_envelopeGreen)
		m_asset->m_greenCurve = kv;
	else if (control == m_envelopeBlue)
		m_asset->m_blueCurve = kv;

	updatePreview();
}

void ColorGradingAssetEditor::eventSliderChange(ui::ContentChangeEvent* event)
{
	auto control = mandatory_non_null_type_cast< ui::Slider* >(event->getSender());

	if (control == m_sliderBrightness)
		m_asset->m_brightness = control->getValue() / 100.0f;
	else if (control == m_sliderContrast)
		m_asset->m_contrast = control->getValue() / 100.0f;
	else if (control == m_sliderSaturation)
		m_asset->m_saturation = control->getValue() / 100.0f;

	updatePreview();
}

}
