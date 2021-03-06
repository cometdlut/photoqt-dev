/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "quicksettings.h"

QuickSettings::QuickSettings(QMap<QString, QVariant> set, bool v, QWidget *parent) : QWidget(parent) {

	verbose = v;

	// The global settings
	globSet = set;

	// style widget
	this->setStyleSheet("QWidget { border-radius: 8px; border-bottom-right-radius: 0px; border-top-right-radius: 0px; background-color: rgba(0, 0, 0, 200); }");
	this->setContentsMargins(10,10,10,10);

	// Initiate and set layout
	central = new QVBoxLayout;
	this->setLayout(central);

	// Setup animation
	ani = new QPropertyAnimation(this,"geometry");

	// A boolean storing if widget is shown or hidden
	isShown = false;

	// block widget in certain circumstances
	blockAll = false;

	// Store if widget shall be triggered my mouse movement
	mouseTrickerEnable = true;

	// The dimensions of the widget (shown and hidden)
	rectShown = QRect(0,50,325,500);
	rectHidden = QRect(-10,-10,10,10);

	// Initially the widget it hidden
	this->setGeometry(rectHidden);


	// Title of widget
	CustomLabel *title = new CustomLabel(tr("Quick Settings"));
	title->setAlignment(Qt::AlignCenter);
	title->setFontSize(13);
	title->setBold(true);
	// Description what this widget is for
	CustomLabel *desc = new CustomLabel(tr("Change settings with one click. They are saved and applied immediately. If you're unsure what a setting does, check the full settings for descriptions."));

	// Sort files by
	CustomLabel *sortbyLabel = new CustomLabel(tr("Sort by"));
	sortby = new CustomComboBox;
	sortby->setBackgroundColor("rgba(0,0,0,0)");
	sort_asc = new CustomRadioButton;
	sort_asc->setIcon(QIcon(":/img/sortascending.png"));
	sort_des = new CustomRadioButton;
	sort_des->setIcon(QIcon(":/img/sortdescending.png"));
	QButtonGroup *grp = new QButtonGroup;
	grp->addButton(sort_asc);
	grp->addButton(sort_des);
	sortby->setPadding(4);
	sortby->addItem(tr("Name"),"name");
	sortby->addItem(tr("Natural Name"),"naturalname");
	sortby->addItem(tr("Date"),"date");
	sortby->addItem(tr("File Size"),"size");
	QHBoxLayout *sortbyLay = new QHBoxLayout;
	sortbyLay->addWidget(sortbyLabel);
	sortbyLay->addWidget(sortby);
	sortbyLay->addWidget(sort_asc);
	sortbyLay->addWidget(sort_des);
	sortbyLay->addStretch();

	// Background style
	background = new CustomComboBox;
	background->setBackgroundColor("rgba(0,0,0,0)");
	background->setPadding(4);
	background->addItem(tr("Real transparency"),"real");
	background->addItem(tr("Faked transparency"),"faked");
	background->addItem(tr("Background image"),"image");
	background->addItem(tr("Coloured background"),"color");

	// Different settings that can be adjusted
	trayicon = new CustomCheckBox(tr("Hide to system tray"));
	loopthroughfolder = new CustomCheckBox(tr("Loop through folder"));
	windowmode = new CustomCheckBox(tr("Window mode"));
	windowdeco = new CustomCheckBox(tr("Show window decoration"));
	clickonempty = new CustomCheckBox(tr("Close on click on background"));
	thumbnailskeepvisible = new CustomCheckBox(tr("Keep thumbnails visible"));
	quickSettings = new CustomCheckBox(tr("Enable 'Quick Settings'"));

	// Thumbnail type
	thumbnailsdynamic = new CustomComboBox;
	thumbnailsdynamic->setBackgroundColor("rgba(0,0,0,0)");
	thumbnailsdynamic->setPadding(4);
	thumbnailsdynamic->addItem(tr("Normal thumbnails"));
	thumbnailsdynamic->addItem(tr("Dynamic thumbnails"));
	thumbnailsdynamic->addItem(tr("Smart thumbnails"));

	// Set default values
	trayicon->setChecked(globSet.value("TrayIcon").toBool());
	loopthroughfolder->setChecked(globSet.value("LoopThroughFolder").toBool());
	windowmode->setChecked(globSet.value("WindowMode").toBool());
	windowdeco->setChecked(globSet.value("WindowDecoration").toBool());
	clickonempty->setChecked(globSet.value("CloseOnGrey").toBool());
	thumbnailskeepvisible->setChecked(globSet.value("ThumbnailKeepVisible").toBool());
	thumbnailsdynamic->setCurrentIndex(globSet.value("ThumbnailDynamic").toInt());
	quickSettings->setChecked(globSet.value("QuickSettings").toBool());

	// Save & Apply settings instantly
	connect(sortby, SIGNAL(currentTextChanged(QString)), this, SLOT(settingChanged()));
	connect(sort_asc, SIGNAL(clicked()), this, SLOT(settingChanged()));
	connect(sort_des, SIGNAL(clicked()), this, SLOT(settingChanged()));
	connect(background, SIGNAL(currentIndexChanged(int)), this, SLOT(settingChanged()));
	connect(trayicon, SIGNAL(toggled(bool)), this, SLOT(settingChanged()));
	connect(loopthroughfolder, SIGNAL(toggled(bool)), this, SLOT(settingChanged()));
	connect(windowmode, SIGNAL(toggled(bool)), this, SLOT(settingChanged()));
	connect(windowdeco, SIGNAL(toggled(bool)), this, SLOT(settingChanged()));
	connect(clickonempty, SIGNAL(toggled(bool)), this, SLOT(settingChanged()));
	connect(thumbnailskeepvisible, SIGNAL(toggled(bool)), this, SLOT(settingChanged()));
	connect(thumbnailsdynamic, SIGNAL(currentIndexChanged(int)), this, SLOT(settingChanged()));
	connect(quickSettings, SIGNAL(toggled(bool)), this, SLOT(settingChanged()));

	// Window deco depends on window mode checked
	windowdeco->setEnabled(false);
	connect(windowmode, SIGNAL(toggled(bool)), windowdeco, SLOT(setEnabled(bool)));

	// Button to show full settings
	CustomPushButton *showsettings = new CustomPushButton(tr("Show full settings"));
	QHBoxLayout *showsettingsLay = new QHBoxLayout;
	showsettingsLay->addStretch();
	showsettingsLay->addWidget(showsettings);
	showsettingsLay->addStretch();
	// Emulate shortcut for full settings
	QSignalMapper *mapper = new QSignalMapper;
	mapper->setMapping(showsettings, "__settings");
	connect(showsettings, SIGNAL(clicked()), mapper, SLOT(map()));
	connect(showsettings, SIGNAL(clicked()), this, SLOT(animate()));
	connect(mapper, SIGNAL(mapped(QString)), this, SIGNAL(emulateShortcut(QString)));


	// Seperators
	CustomLine *sortbyLine = new CustomLine;
	CustomLine *compositeLine = new CustomLine;
	CustomLine *trayiconLine = new CustomLine;
	CustomLine *loopthroughfolderLine = new CustomLine;
	CustomLine *windowmodeLine = new CustomLine;
	CustomLine *clickonemptyLine = new CustomLine;
	CustomLine *thumbnailskeepvisibleLine = new CustomLine;
	CustomLine *thumbnailsdynamicLine = new CustomLine;


	// And the layout

	central->addStretch();

	central->addWidget(title);
	central->addSpacing(10);
	central->addWidget(desc);

	central->addSpacing(15);
	central->addLayout(sortbyLay);
	central->addSpacing(5);
	central->addWidget(sortbyLine);
	central->addSpacing(5);
	central->addWidget(background);
	central->addSpacing(5);
	central->addWidget(compositeLine);
	central->addSpacing(5);
	central->addWidget(trayicon);
	central->addSpacing(5);
	central->addWidget(trayiconLine);
	central->addSpacing(5);
	central->addWidget(loopthroughfolder);
	central->addSpacing(5);
	central->addWidget(loopthroughfolderLine);
	central->addSpacing(5);
	central->addWidget(windowmode);
	central->addSpacing(10);
	central->addWidget(windowdeco);
	central->addSpacing(5);
	central->addWidget(windowmodeLine);
	central->addSpacing(5);
	central->addWidget(clickonempty);
	central->addSpacing(5);
	central->addWidget(clickonemptyLine);
	central->addSpacing(5);
	central->addWidget(thumbnailskeepvisible);
	central->addSpacing(5);
	central->addWidget(thumbnailskeepvisibleLine);
	central->addSpacing(5);
	central->addWidget(thumbnailsdynamic);
	central->addSpacing(5);
	central->addWidget(thumbnailsdynamicLine);
	central->addSpacing(5);
	central->addWidget(quickSettings);
	central->addSpacing(20);
	central->addLayout(showsettingsLay);

	central->addStretch();

	loadSettings();

}

void QuickSettings::loadSettings() {

	QString sort = globSet.value("SortImagesBy").toString();
	for(int i = 0; i < sortby->count(); ++i) {
		if(sortby->itemData(i).toString() == sort) {
			sortby->setCurrentIndex(i);
			break;
		}
	}
	sort_asc->setChecked(globSet.value("SortImagesAscending").toBool());
	sort_des->setChecked(!globSet.value("SortImagesAscending").toBool());

	if(globSet.value("Composite").toBool()) background->setCurrentIndex(0);
	else if(globSet.value("BackgroundImageScreenshot").toBool()) background->setCurrentIndex(1);
	else if(globSet.value("BackgroundImageUse").toBool()) background->setCurrentIndex(2);
	else background->setCurrentIndex(3);

	trayicon->setChecked(globSet.value("TrayIcon").toBool());
	loopthroughfolder->setChecked(globSet.value("LoopThroughFolder").toBool());
	windowmode->setChecked(globSet.value("WindowMode").toBool());
	windowdeco->setChecked(globSet.value("WindowDecoration").toBool());
	clickonempty->setChecked(globSet.value("CloseOnGrey").toBool());
	thumbnailskeepvisible->setChecked(globSet.value("ThumbnailKeepVisible").toBool());
	thumbnailsdynamic->setCurrentIndex(globSet.value("ThumbnailDynamic").toInt());
	quickSettings->setChecked(globSet.value("QuickSettings").toBool());

	windowdeco->setEnabled(windowmode->isChecked());

	defaults.clear();
	defaults.insert("SortImagesAscending",sort_asc->isChecked());
	defaults.insert("SortImagesBy",sort);
	defaults.insert("Background",background->itemData(background->currentIndex()));
	defaults.insert("TrayIcon",trayicon->isChecked());
	defaults.insert("LoopThroughFolder",loopthroughfolder->isChecked());
	defaults.insert("WindowMode",windowmode->isChecked());
	defaults.insert("WindowDecoration",windowdeco->isChecked());
	defaults.insert("CloseOnGrey",clickonempty->isChecked());
	defaults.insert("ThumbnailKeepVisible",thumbnailskeepvisible->isChecked());
	defaults.insert("ThumbnailDynamic",thumbnailsdynamic->currentIndex());
	defaults.insert("QuickSettings",quickSettings->isChecked());

}

// Save and Notify parent of new settings
void QuickSettings::settingChanged() {

	QMap<QString,QVariant> changedSet;

	if(defaults.value("SortImagesBy").toString() != sortby->itemData(sortby->currentIndex()).toString())
		changedSet.insert("SortImagesBy",sortby->itemData(sortby->currentIndex()));
	if(defaults.value("SortImagesAscending").toBool() != sort_asc->isChecked())
		changedSet.insert("SortImagesAscending",sort_asc->isChecked());

	if(defaults.value("Background").toString() != background->itemData(background->currentIndex()).toString()) {
		changedSet.insert("Composite",background->itemData(background->currentIndex()).toString()=="real");
		changedSet.insert("BackgroundImageScreenshot",background->itemData(background->currentIndex()).toString()=="faked");
		changedSet.insert("BackgroundImageUse",background->itemData(background->currentIndex()).toString()=="image");
	}


	if(defaults.value("TrayIcon").toBool() != trayicon->isChecked())
		changedSet.insert("TrayIcon",trayicon->isChecked());

	if(defaults.value("LoopThroughFolder").toBool() != loopthroughfolder->isChecked())
		changedSet.insert("LoopThroughFolder",loopthroughfolder->isChecked());

	if(defaults.value("WindowMode").toBool() != windowmode->isChecked())
		changedSet.insert("WindowMode",windowmode->isChecked());

	if(defaults.value("WindowDecoration").toBool() != windowdeco->isChecked())
		changedSet.insert("WindowDecoration",windowdeco->isChecked());

	if(defaults.value("CloseOnGrey").toBool() != clickonempty->isChecked())
		changedSet.insert("CloseOnGrey",clickonempty->isChecked());

	if(defaults.value("ThumbnailKeepVisible").toBool() != thumbnailskeepvisible->isChecked())
		changedSet.insert("ThumbnailKeepVisible",thumbnailskeepvisible->isChecked());

	if(defaults.value("ThumbnailDynamic").toInt() != thumbnailsdynamic->currentIndex())
		changedSet.insert("ThumbnailDynamic",thumbnailsdynamic->currentIndex());

	if(defaults.value("QuickSettings").toBool() != quickSettings->isChecked())
		changedSet.insert("QuickSettings",quickSettings->isChecked());

	// Update new defaults
	defaults.clear();
	defaults.insert("SortImagesAscending",sort_asc->isChecked());
	defaults.insert("SortImagesBy",sortby->itemData(sortby->currentIndex()));
	defaults.insert("Background",background->itemData(background->currentIndex()));
	defaults.insert("TrayIcon",trayicon->isChecked());
	defaults.insert("LoopThroughFolder",loopthroughfolder->isChecked());
	defaults.insert("WindowMode",windowmode->isChecked());
	defaults.insert("WindowDecoration",windowdeco->isChecked());
	defaults.insert("CloseOnGrey",clickonempty->isChecked());
	defaults.insert("ThumbnailKeepVisible",thumbnailskeepvisible->isChecked());
	defaults.insert("ThumbnailDynamic",thumbnailsdynamic->currentIndex());
	defaults.insert("QuickSettings",quickSettings->isChecked());

	emit updateSettings(changedSet);

}


void QuickSettings::setRect(QRect fullscreen) {

	rectShown = QRect(fullscreen.width()-375, (fullscreen.height()-650)/3, 375, 650);
	rectHidden = QRect(fullscreen.width()+rectShown.width(),rectShown.y(),rectShown.width(),rectShown.height());

	this->setGeometry(this->isVisible() ? rectShown : rectHidden);

}

void QuickSettings::makeShow() {
	if(!isShown) animate();
}

void QuickSettings::makeHide() {
	if(isShown) animate();
}

// Animate open/close the widget
void QuickSettings::animate() {

	// Open widget
	if(!isShown) {

		if(ani->state() != QPropertyAnimation::Stopped)
			ani->targetObject()->setProperty(ani->propertyName(),ani->endValue());

		ani->setDuration(600);
		isShown = true;

		ani->setStartValue(rectHidden);
		ani->setEndValue(rectShown);
		ani->setEasingCurve(QEasingCurve::InBack);
		ani->start();

		this->raise();

	// Close widget
	} else if(isShown) {

		if(ani->state() != QPropertyAnimation::Stopped)
			ani->targetObject()->setProperty(ani->propertyName(),ani->endValue());

		ani->setDuration(500);
		isShown = false;

		ani->setStartValue(rectShown);
		ani->setEndValue(rectHidden);
		ani->setEasingCurve(QEasingCurve::OutBack);
		ani->start();

	}

}

// Make the widget styleable
void QuickSettings::paintEvent(QPaintEvent *) {
	QStyleOption o;
	o.initFrom(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &o, &p, this);
}
