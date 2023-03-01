/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Editor/IObjectEditor.h"

namespace traktor::db
{

class Instance;

}

namespace traktor::drawing
{

class Image;

}

namespace traktor::editor
{

class IEditor;

}

namespace traktor::ui
{

class ContentChangeEvent;
class EnvelopeContentChangeEvent;
class EnvelopeControl;
class Image;
class Slider;

}

namespace traktor::render
{

class ColorGradingTextureAsset;

class ColorGradingAssetEditor : public editor::IObjectEditor
{
	T_RTTI_CLASS;

public:
	explicit ColorGradingAssetEditor(editor::IEditor* editor);

	virtual bool create(ui::Widget* parent, db::Instance* instance, ISerializable* object) override final;

	virtual void destroy() override final;

	virtual void apply() override final;

	virtual bool handleCommand(const ui::Command& command) override final;

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId) override final;

	virtual ui::Size getPreferredSize() const override final;

private:
	editor::IEditor* m_editor;
	Ref< db::Instance > m_instance;
	Ref< ColorGradingTextureAsset > m_asset;
	Ref< drawing::Image > m_referenceImage;
	Ref< drawing::Image > m_resultImage;
	Ref< ui::Image > m_imageOriginal;
	Ref< ui::Image > m_imageResult;
	Ref< ui::EnvelopeControl > m_envelopeRed;
	Ref< ui::EnvelopeControl > m_envelopeGreen;
	Ref< ui::EnvelopeControl > m_envelopeBlue;
	Ref< ui::Slider > m_sliderBrightness;
	Ref< ui::Slider > m_sliderContrast;
	Ref< ui::Slider > m_sliderSaturation;

	void updatePreview();

	void eventEnvelopeChange(ui::EnvelopeContentChangeEvent* event);

	void eventSliderChange(ui::ContentChangeEvent* event);
};

}
