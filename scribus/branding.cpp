/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/
/***************************************************************************
	PubWell product branding (DarkHorse fork of Scribus)
***************************************************************************/

#include "branding.h"

#include <QCoreApplication>
#include <QList>
#include <QPointer>
#include <QRegularExpression>
#include <QTranslator>

namespace
{
	// "Scribus" as a word, unless it names Scribus itself: a version or file format
	// ("Scribus 1.2.x Document"), the project's site, manual or team, a domain or
	// file name ("Scribus.net"), or a real Scribus install ("version of Scribus",
	// "Old Scribus Settings").
	const QRegularExpression& scribusWord()
	{
		static const QRegularExpression re(QStringLiteral(
			"(?<!version of )(?<!Old )\\bScribus\\b"
			"(?!\\s+\\d)"
			"(?!\\s+(?i:wiki|homepage|online|manual|website|web site|team|version|frontend|forums?|bug))"
			"(?!\\.\\w)"));
		return re;
	}

	class RebrandTranslator : public QTranslator
	{
	public:
		using QTranslator::QTranslator;

		QList<QPointer<const QTranslator>> wrapped;

		bool isEmpty() const override { return false; }

		QString translate(const char* context, const char* sourceText, const char* disambiguation, int n) const override
		{
			if (sourceText == nullptr)
				return QString();
			// Ask the wrapped translators directly (going back through
			// QCoreApplication::translate would re-enter its lock).
			for (const auto& translator : wrapped)
			{
				if (translator.isNull())
					continue;
				QString text = translator->translate(context, sourceText, disambiguation, n);
				if (!text.isNull())
					return Branding::rebrand(text);
			}
			// Untranslated: answer only when there is something to rebrand, so Qt
			// otherwise falls back to the source text as usual.
			QString source = QString::fromUtf8(sourceText);
			return source.contains(QLatin1String("Scribus")) ? Branding::rebrand(source) : QString();
		}
	};
}

QString Branding::productName()
{
	return QStringLiteral("PubWell");
}

QString Branding::rebrand(const QString& text)
{
	if (!text.contains(QLatin1String("Scribus")))
		return text;
	QString result(text);
	result.replace(scribusWord(), productName());
	return result;
}

void Branding::installRebrandTranslator(QCoreApplication* app, const QList<const QTranslator*>& wrapped)
{
	static RebrandTranslator* translator = nullptr;
	if (translator == nullptr)
		translator = new RebrandTranslator(app);
	translator->wrapped.clear();
	for (const QTranslator* t : wrapped)
		translator->wrapped.append(t);
	// Translators are searched newest first, so reinstall to stay in front.
	QCoreApplication::removeTranslator(translator);
	QCoreApplication::installTranslator(translator);
}
