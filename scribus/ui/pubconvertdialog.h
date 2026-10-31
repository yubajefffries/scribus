/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/
#ifndef PUBCONVERTDIALOG_H
#define PUBCONVERTDIALOG_H

#include <QDialog>
#include <QStringList>

#include "scribusapi.h"

class QCheckBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;

// PubWell "Convert a Folder of Publisher Files": pick a folder, where the copies
// go, and whether to make PDFs too. The conversion itself runs in the main window.
class SCRIBUS_API PubConvertDialog : public QDialog
{
	Q_OBJECT

public:
	explicit PubConvertDialog(QWidget* parent);

	//! The .pub files found, full paths
	QStringList files() const { return m_files; }
	QString sourceFolder() const;
	//! Empty: next to each original
	QString targetFolder() const;
	bool makePdf() const;

private:
	QStringList m_files;
	QLineEdit* m_source { nullptr };
	QCheckBox* m_subfolders { nullptr };
	QRadioButton* m_besideOriginals { nullptr };
	QRadioButton* m_inFolder { nullptr };
	QLineEdit* m_target { nullptr };
	QPushButton* m_targetBrowse { nullptr };
	QCheckBox* m_pdf { nullptr };
	QLabel* m_found { nullptr };
	QPushButton* m_convert { nullptr };

	void browseSource();
	void browseTarget();
	void rescan();
	void remember();
};

#endif
