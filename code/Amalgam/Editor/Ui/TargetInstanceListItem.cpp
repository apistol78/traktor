#include <iomanip>
#include "Amalgam/Editor/HostEnumerator.h"
#include "Amalgam/Editor/Platform.h"
#include "Amalgam/Editor/TargetConfiguration.h"
#include "Amalgam/Editor/TargetConnection.h"
#include "Amalgam/Editor/TargetInstance.h"
#include "Amalgam/Editor/Ui/ButtonCell.h"
#include "Amalgam/Editor/Ui/DropListCell.h"
#include "Amalgam/Editor/Ui/ProgressCell.h"
#include "Amalgam/Editor/Ui/TargetInstanceListItem.h"
#include "Core/Misc/String.h"
#include "Ui/Application.h"
#include "Ui/Custom/Auto/AutoWidget.h"

// Resources
#include "Resources/Platforms.h"
#include "Resources/TargetControl.h"

namespace traktor
{
	namespace amalgam
	{
		namespace
		{

Ref< ui::Bitmap > s_bitmapPlatforms;
Ref< ui::Bitmap > s_bitmapTargetControl;

const int32_t c_performanceHeight = 48;

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
{
	if (!s_bitmapPlatforms)
		s_bitmapPlatforms = ui::Bitmap::load(c_ResourcePlatforms, sizeof(c_ResourcePlatforms), L"png");
	if (!s_bitmapTargetControl)
		s_bitmapTargetControl = ui::Bitmap::load(c_ResourceTargetControl, sizeof(c_ResourceTargetControl), L"png");

	m_progressCell = new ProgressCell();
	m_hostsCell = new DropListCell(hostEnumerator, instance);
	m_playCell = new ButtonCell(s_bitmapTargetControl, 0, true, ui::EiUser + 1, instance, ui::Command(L"Amalgam.Play"));
}

ui::Size TargetInstanceListItem::getSize() const
{
	RefArray< TargetConnection > connections = m_instance->getConnections();
	return ui::Size(128, 28 + connections.size() * c_performanceHeight);
}

void TargetInstanceListItem::placeCells(ui::custom::AutoWidget* widget, const ui::Rect& rect)
{
	RefArray< TargetConnection > connections = m_instance->getConnections();

	ui::Rect controlRect = rect;
	controlRect.bottom = rect.top + 28;

	if (m_instance->getState() == TsIdle)
	{
		widget->placeCell(
			m_hostsCell,
			ui::Rect(
				controlRect.getCenter().x,
				controlRect.getCenter().y - 10,
				controlRect.right - 24 * 1 - 12,
				controlRect.getCenter().y + 10
			)
		);
	}

	if (m_instance->getState() == TsProgress)
	{
		widget->placeCell(
			m_progressCell,
			ui::Rect(
				controlRect.left + 30,
				controlRect.getCenter().y - 8,
				controlRect.right - 24 * 1 - 8,
				controlRect.getCenter().y + 8
			)
		);
	}

	widget->placeCell(
		m_playCell,
		ui::Rect(
			controlRect.right - 24 * 1 - 4,
			controlRect.top,
			controlRect.right - 24 * 0 - 4,
			controlRect.bottom
		)
	);

	controlRect.top = controlRect.bottom;
	controlRect.bottom = controlRect.top + c_performanceHeight;

	m_stopCells.resize(connections.size());
	for (uint32_t i = 0; i < connections.size(); ++i)
	{
		if (!m_stopCells[i])
			m_stopCells[i] = new ButtonCell(s_bitmapTargetControl, 2, true, ui::EiUser + 2, m_instance, ui::Command(i, L"Amalgam.Stop"));

		widget->placeCell(
			m_stopCells[i],
			ui::Rect(
				controlRect.right - 24 * 1 - 4,
				controlRect.top,
				controlRect.right - 24 * 0 - 4,
				controlRect.bottom
			)
		);

		controlRect = controlRect.offset(0, controlRect.getHeight());
	}
}

void TargetInstanceListItem::paint(ui::custom::AutoWidget* widget, ui::Canvas& canvas, const ui::Rect& rect)
{
	const Platform* platform = m_instance->getPlatform();
	const TargetConfiguration* targetConfiguration = m_instance->getTargetConfiguration();
	RefArray< TargetConnection > connections = m_instance->getConnections();

	ui::Rect controlRect = rect; controlRect.bottom = rect.top + 28;

	canvas.setForeground(Color4ub(255, 255, 255));
	canvas.setBackground(ui::getSystemColor(ui::ScButtonFace));
	canvas.fillGradientRect(controlRect);

	ui::Rect performanceRect = rect;
	performanceRect.top = rect.top + 28;
	performanceRect.bottom = performanceRect.top + c_performanceHeight;
	for (uint32_t i = 0; i < connections.size(); ++i)
	{
		canvas.setForeground(Color4ub(180, 180, 180));
		canvas.setBackground(Color4ub(200, 200, 200));
		canvas.fillGradientRect(performanceRect);
		performanceRect = performanceRect.offset(0, performanceRect.getHeight());
	}

	canvas.setForeground(ui::getSystemColor(ui::ScButtonShadow));
	canvas.drawLine(rect.left, rect.bottom - 1, rect.right, rect.bottom - 1);

	if (m_instance->getState() == TsProgress)
	{
		int32_t progress = m_instance->getBuildProgress();
		m_progressCell->setProgress(progress);
	}
	else
		m_progressCell->setProgress(-1);

	canvas.drawBitmap(
		ui::Point(controlRect.left + 2, controlRect.top + 2),
		ui::Point(platform->getIconIndex() * 24, 0),
		ui::Size(24, 24),
		s_bitmapPlatforms,
		ui::BmAlpha
	);

	ui::Rect textRect = controlRect;
	textRect.left += 34;
	textRect.right -= 24 * 3 - 8;

	canvas.setForeground(ui::getSystemColor(ui::ScWindowText));
	canvas.drawText(textRect, targetConfiguration->getName(), ui::AnLeft, ui::AnCenter);

	ui::Font widgetFont = widget->getFont();
	ui::Font performanceFont = widgetFont; performanceFont.setSize(8);
	canvas.setFont(performanceFont);

	performanceRect = rect;
	performanceRect.top = rect.top + 28;
	performanceRect.bottom = performanceRect.top + c_performanceHeight;
	for (uint32_t i = 0; i < connections.size(); ++i)
	{
		const TargetPerformance& performance = connections[i]->getPerformance();

		ui::Rect topRect = performanceRect;
		topRect.bottom = topRect.top + 12;

		topRect.left += 6;
		canvas.drawText(topRect, toString(int32_t(performance.fps)), ui::AnLeft, ui::AnCenter);

		topRect.left += 20;
		canvas.drawText(topRect, L"U: " + formatPerformanceTime(performance.update), ui::AnLeft, ui::AnCenter);

		topRect.left += 80;
		canvas.drawText(topRect, L"B: " + formatPerformanceTime(performance.build), ui::AnLeft, ui::AnCenter);

		topRect.left += 80;
		canvas.drawText(topRect, L"R: " + formatPerformanceTime(performance.render), ui::AnLeft, ui::AnCenter);

		topRect.left += 80;
		canvas.drawText(topRect, L"GC: " + formatPerformanceTime(performance.garbageCollect), ui::AnLeft, ui::AnCenter);

		ui::Rect middleRect = performanceRect;
		middleRect.top = performanceRect.top + 12;
		middleRect.bottom = performanceRect.top + 24;

		middleRect.left += 26;
		canvas.drawText(middleRect, L"P: " + formatPerformanceTime(performance.physics), ui::AnLeft, ui::AnCenter);

		middleRect.left += 80;
		canvas.drawText(middleRect, L"I: " + formatPerformanceTime(performance.input), ui::AnLeft, ui::AnCenter);

		middleRect.left += 80;
		canvas.drawText(middleRect, L"S: " + toString(int32_t(performance.steps)) + L", " + formatPerformanceTime(performance.interval) + L", " + toString(performance.collisions), ui::AnLeft, ui::AnCenter);

		ui::Rect middleRect2 = performanceRect;
		middleRect2.top = performanceRect.top + 24;
		middleRect2.bottom = performanceRect.top + 36;

		middleRect2.left += 26;
		canvas.drawText(middleRect2, L"Draw: " + toString(performance.drawCalls), ui::AnLeft, ui::AnCenter);

		middleRect2.left += 100;
		canvas.drawText(middleRect2, L"Prim: " + toString(performance.primitiveCount), ui::AnLeft, ui::AnCenter);

		middleRect2.left += 100;
		canvas.drawText(middleRect2, L"Phys: " + toString(performance.activeBodyCount) + L"/" + toString(performance.bodyCount) + L", " + toString(performance.manifoldCount), ui::AnLeft, ui::AnCenter);

		middleRect2.left += 100;
		canvas.drawText(middleRect2, L"Snd: " + toString(performance.activeSoundChannels), ui::AnLeft, ui::AnCenter);

		ui::Rect bottomRect = performanceRect;
		bottomRect.top = performanceRect.top + 36;

		bottomRect.left += 26;
		canvas.drawText(bottomRect, L"Mem: " + toString(performance.memInUse / 1024) + L" KiB", ui::AnLeft, ui::AnCenter);

		bottomRect.left += 100;
		canvas.drawText(bottomRect, L"Obj: " + toString(performance.heapObjects), ui::AnLeft, ui::AnCenter);

		bottomRect.left += 100;
		canvas.drawText(bottomRect, L"Smem: " + toString(performance.memInUseScript / 1024) + L" KiB", ui::AnLeft, ui::AnCenter);

		bottomRect.left += 100;
		canvas.drawText(bottomRect, L"Res: " + toString(performance.residentResourcesCount) + L", " + toString(performance.exclusiveResourcesCount), ui::AnLeft, ui::AnCenter);

		canvas.drawBitmap(
			ui::Point(performanceRect.right - 24 * 1 - 4, performanceRect.getCenter().y - 10),
			ui::Point(2 * 19, 0),
			ui::Size(19, 19),
			s_bitmapTargetControl,
			ui::BmAlpha
		);

		performanceRect = performanceRect.offset(0, performanceRect.getHeight());
	}

	canvas.setFont(widgetFont);

	m_playCell->setEnable(m_instance->getState() == TsIdle);
}

	}
}
