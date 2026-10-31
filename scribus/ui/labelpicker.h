/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/
#ifndef LABELPICKER_H
#define LABELPICKER_H

#include <QDialog>
#include <QList>
#include <QPixmap>
#include <QString>

#include "scribusapi.h"

class QDoubleSpinBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QPlainTextEdit;
class QPushButton;
class QRadioButton;
class QSpinBox;

// One sheet of labels. All measurements in inches.
struct SCRIBUS_API LabelSpec
{
	QString name;
	QString kind;
	double pageWidth { 8.5 };
	double pageHeight { 11.0 };
	double labelWidth { 2.625 };
	double labelHeight { 1.0 };
	int columns { 3 };
	int rows { 10 };
	double topMargin { 0.5 };
	double sideMargin { 0.1875 };
	double gapAcross { 0.125 };
	double gapDown { 0.0 };

	int count() const { return columns * rows; }
	bool fitsPage() const;
	QString summary() const;
};

// PubWell label chooser: pick the sheet by the number on the package (Avery and
// compatible), or type in a custom size, and optionally the text that goes on
// every label.
class SCRIBUS_API LabelPicker : public QDialog
{
	Q_OBJECT

public:
	explicit LabelPicker(QWidget* parent);

	LabelSpec spec() const;
	//! Text for every label, empty for blank labels
	QString repeatedText() const;

	static QList<LabelSpec> presets();
	static QPixmap sheetPreview(const LabelSpec& spec, int height);

private:
	QList<LabelSpec> m_presets;
	QLineEdit* m_search { nullptr };
	QListWidget* m_list { nullptr };
	QLabel* m_preview { nullptr };
	QLabel* m_details { nullptr };
	QGroupBox* m_customBox { nullptr };
	QDoubleSpinBox* m_width { nullptr };
	QDoubleSpinBox* m_height { nullptr };
	QSpinBox* m_columns { nullptr };
	QSpinBox* m_rows { nullptr };
	QDoubleSpinBox* m_top { nullptr };
	QDoubleSpinBox* m_side { nullptr };
	QDoubleSpinBox* m_gapAcross { nullptr };
	QDoubleSpinBox* m_gapDown { nullptr };
	QRadioButton* m_blank { nullptr };
	QRadioButton* m_same { nullptr };
	QPlainTextEdit* m_text { nullptr };
	QPushButton* m_create { nullptr };

	bool customSelected() const;
	LabelSpec customSpec() const;
	void filter(const QString& text);
	void selectionChanged();
	void updatePreview();
};

#endif
