/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/

#include "fsw_appearance.h"
#include "prefsstructs.h"
#include <QEvent>
#include <QRadioButton>

FSW_Appearance::FSW_Appearance(QWidget* parent)
	: QWizardPage(parent)
{
	setupUi(this);
	themeAutoRadio->setChecked(true);
	workspaceMinimalRadio->setChecked(true);
	wireRadios();
}

void FSW_Appearance::wireRadios()
{
	connect(themeLightRadio, &QRadioButton::toggled, this, [this](bool on){ if (on) emit themeModeChanged(0); });
	connect(themeDarkRadio,  &QRadioButton::toggled, this, [this](bool on){ if (on) emit themeModeChanged(1); });
	connect(themeAutoRadio,  &QRadioButton::toggled, this, [this](bool on){ if (on) emit themeModeChanged(2); });
}

void FSW_Appearance::restoreDefaults(const ApplicationPrefs* prefsData)
{
	if (!prefsData)
		return;

	const QString& palette = prefsData->uiPrefs.stylePalette;
	if (palette == QLatin1String("light"))
		themeLightRadio->setChecked(true);
	else if (palette == QLatin1String("dark"))
		themeDarkRadio->setChecked(true);
	else
		themeAutoRadio->setChecked(true);

	if (prefsData->uiPrefs.simpleWorkspace)
		workspaceMinimalRadio->setChecked(true);
	else
		workspaceFullRadio->setChecked(true);
}

int FSW_Appearance::themeMode() const
{
	if (themeLightRadio->isChecked())
		return 0;
	if (themeDarkRadio->isChecked())
		return 1;
	return 2;
}

bool FSW_Appearance::minimalWorkspace() const
{
	// Stored as uiPrefs.simpleWorkspace (see ScribusMainWindow::setSimpleWorkspace)
	return workspaceMinimalRadio->isChecked();
}

void FSW_Appearance::changeEvent(QEvent* e)
{
	if (e->type() == QEvent::LanguageChange)
		retranslateUi(this);
	QWizardPage::changeEvent(e);
}
