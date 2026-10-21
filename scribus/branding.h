/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/
/***************************************************************************
	PubWell product branding (DarkHorse fork of Scribus)
***************************************************************************/

#ifndef BRANDING_H
#define BRANDING_H

#include <QList>
#include <QString>

#include "scribusapi.h"

class QCoreApplication;
class QTranslator;

namespace Branding
{
	//! Name shown to users, e.g. in window titles.
	SCRIBUS_API QString productName();

	/*!
	 * Replace "Scribus" with the product name in user-visible text, except where it
	 * really means Scribus: file format and version names ("Scribus 1.6.0"), the
	 * Scribus website, manual and team, and settings of a real Scribus install.
	 */
	SCRIBUS_API QString rebrand(const QString& text);

	/*!
	 * Install (or move to the front) a translator that looks strings up in the
	 * wrapped translators (in order) and rebrands the result, so the Scribus sources
	 * and translations stay untouched. Call it after installing the regular translators.
	 */
	void installRebrandTranslator(QCoreApplication* app, const QList<const QTranslator*>& wrapped);
}

#endif
