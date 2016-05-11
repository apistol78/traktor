#include <iomanip>
#include "Amalgam/Editor/HostEnumerator.h"
#include "Amalgam/Editor/TargetConnection.h"
#include "Amalgam/Editor/Ui/ButtonCell.h"
#include "Amalgam/Editor/Ui/DropListCell.h"
#include "Amalgam/Editor/Ui/ProgressCell.h"
#include "Amalgam/Editor/Ui/TargetBrowseEvent.h"
#include "Amalgam/Editor/Ui/TargetBuildEvent.h"
#include "Amalgam/Editor/Ui/TargetCaptureEvent.h"
#include "Amalgam/Editor/Ui/TargetInstanceListItem.h"
#include "Amalgam/Editor/Ui/TargetMigrateEvent.h"
#include "Amalgam/Editor/Ui/TargetPlayEvent.h"
#include "Amalgam/Editor/Ui/TargetStopEvent.h"
#include "Amalgam/Editor/Deploy/Platform.h"
#include "Amalgam/Editor/Deploy/TargetConfiguration.h"
#include "Core/Misc/String.h"
#include "I18N/Text.h"
#include "Ui/Application.h"
#include "Ui/StyleBitmap.h"
#include "Ui/StyleSheet.h"
#include "Ui/Custom/Auto/AutoWidget.h"

namespace traktor
{
	namespace amalgam
	{
		namespace
		{

Ref< ui::IBitmap > s_bitmapLogos;

const int32_t c_performanceLineHeight = 14;
const int32_t c_performanceHeight = 7 * c_performanceLineHeight;

const Color4ub c_markerColors[] =
{
	Color4ub(255, 255, 120),
	Color4ub(200, 200, 80),
	Color4ub(255, 120, 255),
	Color4ub(200, 80, 200),
	Color4ub(120, 255, 255),
	Color4ub(80, 200, 200),
	Color4ub(255, 120, 120),
	Color4ub(200, 80, 80),
	Color4ub(120, 255, 120),
	Color4ub(80, 200, 80),
	Color4ub(120, 120, 255),
	Color4ub(80, 80, 200)
};

const wchar_t* c_markerNames[] =
{
	L"End",
	L"Render update",
	L"Session",
	L"Script GC",
	L"Audio",
	L"Rumble",
	L"Input",
	L"State",
	L"Physics",
	L"Build",
	L"Audio Layer",
	L"Flash Layer",
	L"Video Layer",
	L"World Layer",
	L"Entity Events",
	L"Script"
};

std::wstring formatPerformanceTime(float time)
{
	std::wstringstream ss;
	ss << std::setprecision(1) << std::fixed << (time * 1000.0f);
	return ss.str();
}

std::wstring formatPerformanceValue(float value)
{
	std::wstringstream ss;
	ss << std::setprecision(1) << std::fixed << value;
	return ss.str();
}

		}

TargetInstanceListItem::TargetInstanceListItem(HostEnumerator* hostEnumerator, TargetInstance* instance)
:	m_instance(instance)
,	m_lastInstanceState((TargetState)-1)
{
	if (!s_bitmapLogos)
		s_bitmapLogos = new ui::StyleBitmap(L"Amalgam.Logos");

	m_progressCell = new ProgressCell();
	m_hostsCell = new DropListCell(hostEnumerator, instance);

	m_playCell = new ButtonCell(new ui::StyleBitmap(L"Amalgam.TargetPlay"), ui::Command());
	m_playCell->addEventHandler< ui::ButtonClickEvent >(this, &TargetInstanceListItem::eventPlayButtonClick);

	m_buildCell = new ButtonCell(new ui::StyleBitmap(L"Amalgam.TargetBuild"), ui::Command());
	m_buildCell->addEventHandler< ui::ButtonClickEvent >(this, &TargetInstanceListItem::eventBuildButtonClick);

	m_migrateCell = new ButtonCell(new ui::StyleBitmap(L"Amalgam.TargetMigrate"), ui::Command());
	m_migrateCell->addEventHandler< ui::ButtonClickEvent >(this, &TargetInstanceListItem::eventMigrateButtonClick);

	m_browseCell = new ButtonCell(new ui::StyleBitmap(L"Amalgam.TargetBrowse"), ui::Command());
	m_browseCell->addEventHandler< ui::ButtonClickEvent >(this, &TargetInstanceListItem::eventBrowseButtonClick);
}

ui::Size TargetInstanceListItem::getSize() const
{
	RefArray< TargetConnection > connections = m_instance->getConnections();
	return ui::Size(128, ui::scaleBySystemDPI(28 + connections.size() * c_performanceHeight));
}

void TargetInstanceListItem::placeCells(ui::custom::AutoWidget* widget, const ui::Rect& rect)
{
	RefArray< TargetConnection > connections = m_instance->getConnections();

	ui::Rect controlRect = rect;
	controlRect.bottom = rect.top + ui::scaleBySystemDPI(28);

	if (m_instance->getState() == TsIdle)
	{
		widget->placeCell(
			m_hostsCell,
			ui::Rect(
				controlRect.left + ui::scaleBySystemDPI(130),
				controlRect.getCenter().y - ui::scaleBySystemDPI(10),
				controlRect.right - ui::scaleBySystemDPI(24) * 4 - 12,
				controlRect.getCenter().y + ui::scaleBySystemDPI(10)
			)
		);
	}
	else
	{
		widget->placeCell(
			m_progressCell,
			ui::Rect(
				controlRect.left + ui::scaleBySystemDPI(30),
				controlRect.getCenter().y - ui::scaleBySystemDPI(8),
				controlRect.right - ui::scaleBySystemDPI(24) * 4 - 8,
				controlRect.getCenter().y + ui::scaleBySystemDPI(8)
			)
		);
	}

	widget->placeCell(
		m_playCell,
		ui::Rect(
			controlRect.right - ui::scaleBySystemDPI(24) * 4 - 4,
			controlRect.top,
			controlRect.right - ui::scaleBySystemDPI(24) * 3 - 4,
			controlRect.bottom
		)
	);
	widget->placeCell(
		m_buildCell,
		ui::Rect(
			controlRect.right - ui::scaleBySystemDPI(24) * 3 - 4,
			controlRect.top,
			controlRect.right - ui::scaleBySystemDPI(24) * 2 - 4,
			controlRect.bottom
		)
	);
	widget->placeCell(
		m_migrateCell,
		ui::Rect(
			controlRect.right - ui::scaleBySystemDPI(24) * 2 - 4,
			controlRect.top,
			controlRect.right - ui::scaleBySystemDPI(24) * 1 - 4,
			controlRect.bottom
		)
	);
	widget->placeCell(
		m_browseCell,
		ui::Rect(
			controlRect.right - ui::scaleBySystemDPI(24) * 1 - 4,
			controlRect.top,
			controlRect.right - ui::scaleBySystemDPI(24) * 0 - 4,
			controlRect.bottom
		)
	);

	controlRect.top = controlRect.bottom;
	controlRect.bottom = controlRect.top + ui::scaleBySystemDPI(c_performanceHeight);

	m_stopCells.resize(connections.size());
	m_captureCells.resize(connections.size());

	for (uint32_t i = 0; i < connections.size(); ++i)
	{
		if (!m_stopCells[i])
		{
			m_stopCells[i] = new ButtonCell(new ui::StyleBitmap(L"Amalgam.TargetStop"), ui::Command(i));
			m_stopCells[i]->addEventHandler< ui::ButtonClickEvent >(this, &TargetInstanceListItem::eventStopButtonClick);
		}

		if (!m_captureCells[i])
		{
			m_captureCells[i] = new ButtonCell(new ui::StyleBitmap(L"Amalgam.TargetProfile"), ui::Command(i));
			m_captureCells[i]->addEventHandler< ui::ButtonClickEvent >(this, &TargetInstanceListItem::eventCaptureButtonClick);
		}

		widget->placeCell(
			m_stopCells[i],
			ui::Rect(
				controlRect.right - ui::scaleBySystemDPI(24) * 1 - 4,
				controlRect.top,
				controlRect.right - ui::scaleBySystemDPI(24) * 0 - 4,
				(controlRect.top + controlRect.bottom) / 2
			)
		);

		widget->placeCell(
			m_captureCells[i],
			ui::Rect(
				controlRect.right - ui::scaleBySystemDPI(24) * 1 - 4,
				(controlRect.top + controlRect.bottom) / 2,
				controlRect.right - ui::scaleBySystemDPI(24) * 0 - 4,
				controlRect.bottom
			)
		);

		controlRect = controlRect.offset(0, controlRect.getHeight());
	}

	AutoWidgetCell::placeCells(widget, rect);
}

void TargetInstanceListItem::paint(ui::Canvas& canvas, const ui::Rect& rect)
{
	const ui::StyleSheet* ss = ui::Application::getInstance()->getStyleSheet();
	const Platform* platform = m_instance->getPlatform();
	const TargetConfiguration* targetConfiguration = m_instance->getTargetConfiguration();
	RefArray< TargetConnection > connections = m_instance->getConnections();

	ui::Rect controlRect = rect; controlRect.bottom = rect.top + ui::scaleBySystemDPI(28);

	canvas.setBackground(ss->getColor(getWidget(), L"item-background-color"));
	canvas.fillRect(controlRect);

	ui::Rect performanceRect = rect;
	performanceRect.top = rect.top + ui::scaleBySystemDPI(28);
	performanceRect.bottom = performanceRect.top + ui::scaleBySystemDPI(c_performanceHeight);
	for (uint32_t i = 0; i < connections.size(); ++i)
	{
		canvas.setBackground(ss->getColor(getWidget(), L"item-connection-background-color"));
		canvas.fillRect(performanceRect);
		performanceRect = performanceRect.offset(0, performanceRect.getHeight());
	}

	canvas.setForeground(ss->getColor(getWidget(), L"item-seperator-color"));
	canvas.drawLine(rect.left, rect.bottom - 1, rect.right, rect.bottom - 1);

	if (m_instance->getState() != m_lastInstanceState)
	{
		const wchar_t* c_textIds[] =
		{
			L"AMALGAM_STATE_IDLE",
			L"AMALGAM_STATE_BUILDING",
			L"AMALGAM_STATE_DEPLOYING",
			L"AMALGAM_STATE_LAUNCHING",
			L"AMALGAM_STATE_MIGRATING",
			L"AMALGAM_STATE_PENDING"
		};
		T_FATAL_ASSERT(m_instance->getState() < sizeof_array(c_textIds));
		m_progressCell->setText(i18n::Text(c_textIds[m_instance->getState()]));
		m_lastInstanceState = m_instance->getState();
	}

	if (m_instance->getState() != TsIdle)
	{
		int32_t progress = m_instance->getBuildProgress();
		m_progressCell->setProgress(progress);
	}
	else
		m_progressCell->setProgress(-1);

	int32_t logoSize = s_bitmapLogos->getSize().cy;
	canvas.drawBitmap(
		ui::Point(controlRect.left + 2, controlRect.getCenter().y - logoSize / 2),
		ui::Point(platform->getIconIndex() * logoSize, 0),
		ui::Size(logoSize, logoSize),
		s_bitmapLogos,
		ui::BmAlpha
	);

	ui::Rect textRect = controlRect;
	textRect.left += logoSize + 10;
	textRect.right -= ui::scaleBySystemDPI(24) * 3 - 8;

	canvas.setForeground(ss->getColor(getWidget(), L"color"));
	canvas.drawText(textRect, targetConfiguration->getName(), ui::AnLeft, ui::AnCenter);

	ui::Font widgetFont = getWidget()->getFont();
	ui::Font performanceFont = widgetFont; performanceFont.setSize(10);
	ui::Font performanceBoldFont = performanceFont; performanceBoldFont.setBold(true);
	ui::Font markerFont = widgetFont; markerFont.setSize(7);

	performanceRect = rect;
	performanceRect.right -= ui::scaleBySystemDPI(34);
	performanceRect.top = rect.top + ui::scaleBySystemDPI(28);
	performanceRect.bottom = performanceRect.top + ui::scaleBySystemDPI(c_performanceHeight);

	for (uint32_t i = 0; i < connections.size(); ++i)
	{
		const TargetPerformance& performance = connections[i]->getPerformance();

		canvas.setClipRect(performanceRect);

		ui::Rect nameRect = performanceRect;
		nameRect.bottom = nameRect.top + ui::scaleBySystemDPI(c_performanceLineHeight);

		nameRect.left += 6;
		canvas.setFont(performanceBoldFont);
		canvas.drawText(nameRect, connections[i]->getName(), ui::AnLeft, ui::AnCenter);
		canvas.setFont(performanceFont);

		ui::Rect topRect = performanceRect;
		topRect.top = performanceRect.top + ui::scaleBySystemDPI(c_performanceLineHeight);
		topRect.bottom = topRect.top + ui::scaleBySystemDPI(c_performanceLineHeight);

		topRect.left += ui::scaleBySystemDPI(6);
		canvas.drawText(topRect, toString(int32_t(performance.fps)), ui::AnLeft, ui::AnCenter);

		topRect.left += ui::scaleBySystemDPI(20);
		canvas.drawText(topRect, L"U: " + formatPerformanceTime(performance.update), ui::AnLeft, ui::AnCenter);

		topRect.left += ui::scaleBySystemDPI(80);
		canvas.drawText(topRect, L"B: " + formatPerformanceTime(performance.build), ui::AnLeft, ui::AnCenter);

		topRect.left += ui::scaleBySystemDPI(80);
		canvas.drawText(topRect, L"R: " + formatPerformanceTime(performance.render), ui::AnLeft, ui::AnCenter);

		topRect.left += ui::scaleBySystemDPI(80);
		canvas.drawText(topRect, L"GC: " + formatPerformanceTime(performance.garbageCollect), ui::AnLeft, ui::AnCenter);

		ui::Rect middleRect = performanceRect;
		middleRect.top = performanceRect.top + ui::scaleBySystemDPI(c_performanceLineHeight) * 2;
		middleRect.bottom = middleRect.top + ui::scaleBySystemDPI(c_performanceLineHeight);

		middleRect.left += ui::scaleBySystemDPI(26);
		canvas.drawText(middleRect, L"P: " + formatPerformanceTime(performance.physics), ui::AnLeft, ui::AnCenter);

		middleRect.left += ui::scaleBySystemDPI(80);
		canvas.drawText(middleRect, L"I: " + formatPerformanceTime(performance.input), ui::AnLeft, ui::AnCenter);

		middleRect.left += ui::scaleBySystemDPI(80);
		canvas.drawText(middleRect, L"Sim: " + toString(int32_t(performance.steps)) + L", " + formatPerformanceTime(performance.interval) + L", " + toString(performance.collisions), ui::AnLeft, ui::AnCenter);

		middleRect.left += ui::scaleBySystemDPI(150);
		canvas.drawText(middleRect, L"F: " + toString(performance.flashCharacterCount) + L", " + toString(performance.flashGCCandidates), ui::AnLeft, ui::AnCenter);

		ui::Rect middleRect2 = performanceRect;
		middleRect2.top = performanceRect.top + ui::scaleBySystemDPI(c_performanceLineHeight) * 3;
		middleRect2.bottom = middleRect2.top + ui::scaleBySystemDPI(c_performanceLineHeight);

		middleRect2.left += ui::scaleBySystemDPI(26);
		canvas.drawText(middleRect2, L"Draw: " + toString(performance.drawCalls), ui::AnLeft, ui::AnCenter);

		middleRect2.left += ui::scaleBySystemDPI(100);
		canvas.drawText(middleRect2, L"Prim: " + toString(performance.primitiveCount), ui::AnLeft, ui::AnCenter);

		middleRect2.left += ui::scaleBySystemDPI(100);
		canvas.drawText(middleRect2, L"Phys: " + toString(performance.activeBodyCount) + L"/" + toString(performance.bodyCount) + L", M: " + toString(performance.manifoldCount) + L", Q: " + toString(performance.queryCount), ui::AnLeft, ui::AnCenter);

		middleRect2.left += ui::scaleBySystemDPI(150);
		canvas.drawText(middleRect2, L"Snd: " + toString(performance.activeSoundChannels), ui::AnLeft, ui::AnCenter);

		ui::Rect bottomRect = performanceRect;
		bottomRect.top = performanceRect.top + ui::scaleBySystemDPI(c_performanceLineHeight) * 4;
		bottomRect.bottom = bottomRect.top + ui::scaleBySystemDPI(c_performanceLineHeight);

		bottomRect.left += ui::scaleBySystemDPI(26);
		canvas.drawText(bottomRect, L"Mem: " + toString(performance.memInUse / 1024) + L" KiB", ui::AnLeft, ui::AnCenter);

		bottomRect.left += ui::scaleBySystemDPI(100);
		canvas.drawText(bottomRect, L"Obj: " + toString(performance.heapObjects), ui::AnLeft, ui::AnCenter);

		bottomRect.left += ui::scaleBySystemDPI(100);
		canvas.drawText(bottomRect, L"Smem: " + toString(performance.memInUseScript / 1024) + L" KiB", ui::AnLeft, ui::AnCenter);

		bottomRect.left += ui::scaleBySystemDPI(100);
		canvas.drawText(bottomRect, L"Res: " + toString(performance.residentResourcesCount) + L", " + toString(performance.exclusiveResourcesCount), ui::AnLeft, ui::AnCenter);

		if (performance.frameMarkers.size() >= 1)
		{
			ui::Rect graphRect = performanceRect;
			graphRect.left += ui::scaleBySystemDPI(26);
			graphRect.top = performanceRect.top + ui::scaleBySystemDPI(c_performanceLineHeight) * 5;
			graphRect.bottom = graphRect.top + ui::scaleBySystemDPI(c_performanceLineHeight) + ui::scaleBySystemDPI(c_performanceLineHeight) / 2;

			int32_t w = graphRect.getWidth();

			float endTime = performance.frameMarkers.back().end;

			canvas.setBackground(Color4ub(255, 255, 255));
			canvas.fillRect(graphRect);

			canvas.setFont(markerFont);

			for (uint32_t j = 0; j < performance.frameMarkers.size() - 1; ++j)
			{
				const TargetPerformance::FrameMarker& fm = performance.frameMarkers[j];

				int32_t xb = graphRect.left + int32_t(fm.begin * w / endTime);
				int32_t xe = graphRect.left + int32_t(fm.end * w / endTime);

				if (xe <= xb)
					continue;

				int32_t o = fm.level * 2;

				ui::Rect markerRect(
					xb, graphRect.top + o,
					xe, graphRect.bottom - o
				);

				canvas.setBackground(c_markerColors[j % sizeof_array(c_markerColors)]);
				canvas.fillRect(markerRect);

				canvas.setClipRect(markerRect);
				canvas.drawText(markerRect, c_markerNames[fm.id], ui::AnLeft, ui::AnCenter);

				canvas.setClipRect(performanceRect);
			}
		}

		performanceRect = performanceRect.offset(0, ui::scaleBySystemDPI(c_performanceHeight));
	}

	canvas.resetClipRect();
	canvas.setFont(widgetFont);

	m_playCell->setEnable(m_instance->getState() == TsIdle);
	m_buildCell->setEnable(m_instance->getState() == TsIdle);
	m_migrateCell->setEnable(m_instance->getState() == TsIdle);
}

void TargetInstanceListItem::eventPlayButtonClick(ui::ButtonClickEvent* event)
{
	TargetPlayEvent playEvent(this, m_instance);
	getWidget()->raiseEvent(&playEvent);
}

void TargetInstanceListItem::eventBuildButtonClick(ui::ButtonClickEvent* event)
{
	TargetBuildEvent buildEvent(this, m_instance);
	getWidget()->raiseEvent(&buildEvent);
}

void TargetInstanceListItem::eventMigrateButtonClick(ui::ButtonClickEvent* event)
{
	TargetMigrateEvent migrateEvent(this, m_instance);
	getWidget()->raiseEvent(&migrateEvent);
}

void TargetInstanceListItem::eventBrowseButtonClick(ui::ButtonClickEvent* event)
{
	TargetBrowseEvent browseEvent(this, m_instance);
	getWidget()->raiseEvent(&browseEvent);
}

void TargetInstanceListItem::eventStopButtonClick(ui::ButtonClickEvent* event)
{
	TargetStopEvent stopEvent(this, m_instance, event->getCommand().getId());
	getWidget()->raiseEvent(&stopEvent);
}

void TargetInstanceListItem::eventCaptureButtonClick(ui::ButtonClickEvent* event)
{
	TargetCaptureEvent captureEvent(this, m_instance, event->getCommand().getId());
	getWidget()->raiseEvent(&captureEvent);
}

	}
}
