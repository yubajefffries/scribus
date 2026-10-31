/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/
#ifndef STARTSCREEN_H
#define STARTSCREEN_H

#include <QDialog>
#include <QPixmap>
#include <QStringList>

#include "scribusapi.h"
#include "ui/labelpicker.h"

class QCheckBox;
class QListWidget;
class QListWidgetItem;

// PubWell start screen: Publisher-style "what would you like to make?" with big
// tiles for common jobs, opening a Publisher file and recent documents. The
// classic Scribus New Document dialog stays one click away ("More options").
class SCRIBUS_API StartScreen : public QDialog
{
	Q_OBJECT

public:
	enum Choice
	{
		None,
		BlankPage,
		Flyer,
		BusinessCard,
		LabelSheet,
		Newsletter,
		Postcard,
		OpenPublisher,
		OpenOther,
		OpenRecent,
		ConvertPublisherFolder,
		MoreOptions
	};

	StartScreen(QWidget* parent, const QStringList& recentDocs, bool startUp);

	Choice choice() const { return m_choice; }
	QString recentFile() const { return m_recentFile; }
	bool showAtStartup() const;
	//! The label sheet picked for LabelSheet, and the text for every label (may be empty)
	const LabelSpec& labelSpec() const { return m_labelSpec; }
	QString labelText() const { return m_labelText; }

	static QPixmap preview(Choice kind, int size);

private:
	Choice m_choice { None };
	QString m_recentFile;
	LabelSpec m_labelSpec;
	QString m_labelText;
	QCheckBox* m_showAtStartup { nullptr };
	QListWidget* m_recentList { nullptr };

	void choose(Choice c);
	void openRecent(QListWidgetItem* item);
};

#endif
