/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/

#include "labelpicker.h"

#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPainter>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QtMath>

#include "branding.h"
#include "iconmanager.h"
#include "prefscontext.h"
#include "prefsfile.h"
#include "prefsmanager.h"

namespace
{
	const QColor plum(126, 63, 95);
	const double mm = 1.0 / 25.4;

	LabelSpec sheet(const QString& name, const QString& kind, double w, double h, int cols, int rows,
					double top, double side, double gapAcross, double gapDown = 0.0, bool a4 = false)
	{
		LabelSpec s;
		s.name = name;
		s.kind = kind;
		if (a4)
		{
			s.pageWidth = 210 * mm;
			s.pageHeight = 297 * mm;
		}
		s.labelWidth = w;
		s.labelHeight = h;
		s.columns = cols;
		s.rows = rows;
		s.topMargin = top;
		s.sideMargin = side;
		s.gapAcross = gapAcross;
		s.gapDown = gapDown;
		return s;
	}

	QString inches(double v)
	{
		return QString::number(v, 'g', 4);
	}

	const char* customKey = "custom";
}

bool LabelSpec::fitsPage() const
{
	const double right = sideMargin + columns * labelWidth + (columns - 1) * gapAcross;
	const double bottom = topMargin + rows * labelHeight + (rows - 1) * gapDown;
	return columns > 0 && rows > 0 && labelWidth > 0 && labelHeight > 0
		&& right <= pageWidth + 0.01 && bottom <= pageHeight + 0.01;
}

QString LabelSpec::summary() const
{
	const bool a4 = pageWidth < 8.4;
	const QString size = a4
		? QObject::tr("%1 x %2 mm").arg(qRound(labelWidth / mm * 10) / 10.0).arg(qRound(labelHeight / mm * 10) / 10.0)
		: QObject::tr("%1 x %2 in").arg(inches(labelWidth), inches(labelHeight));
	return QObject::tr("%1 per sheet, %2 each, on %3 paper").arg(count()).arg(size, a4 ? QObject::tr("A4") : QObject::tr("letter"));
}

QList<LabelSpec> LabelPicker::presets()
{
	// Avery's published sheet geometry. The compatible numbers (8160, 5960 ...) share the same layout.
	return {
		sheet("Avery 5160 / 8160 / 5960", tr("Address labels"), 2.625, 1.0, 3, 10, 0.5, 0.1875, 0.125),
		sheet("Avery 5161 / 8161", tr("Address labels"), 4.0, 1.0, 2, 10, 0.5, 0.15625, 0.1875),
		sheet("Avery 5162 / 8162", tr("Address labels"), 4.0, 4.0 / 3, 2, 7, 5.0 / 6, 0.15625, 0.1875),
		sheet("Avery 5163 / 8163", tr("Shipping labels"), 4.0, 2.0, 2, 5, 0.5, 0.15625, 0.1875),
		sheet("Avery 5164 / 8164", tr("Shipping labels"), 4.0, 10.0 / 3, 2, 3, 0.5, 0.15625, 0.1875),
		sheet("Avery 5126 / 8126", tr("Half-sheet shipping labels"), 8.5, 5.5, 1, 2, 0.0, 0.0, 0.0),
		sheet("Avery 5167 / 8167", tr("Return address labels"), 1.75, 0.5, 4, 20, 0.5, 0.28125, 0.3125),
		sheet("Avery 5366 / 8366", tr("File folder labels"), 3.4375, 2.0 / 3, 2, 15, 0.5, 0.53125, 0.5625),
		sheet("Avery 5371 / 8371", tr("Business cards"), 3.5, 2.0, 2, 5, 0.5, 0.75, 0.0),
		sheet("Avery 5395 / 8395", tr("Name badges"), 3.375, 7.0 / 3, 2, 4, 0.59375, 0.6875, 0.375, 1.0 / 6),
		sheet("Avery L7160", tr("Address labels"), 63.5 * mm, 38.1 * mm, 3, 7, 15.15 * mm, 7.25 * mm, 2.54 * mm, 0.0, true),
		sheet("Avery L7163", tr("Address labels"), 99.1 * mm, 38.1 * mm, 2, 7, 15.15 * mm, 4.65 * mm, 2.5 * mm, 0.0, true),
		sheet("Avery L7173", tr("Shipping labels"), 99.1 * mm, 57.0 * mm, 2, 5, 6.0 * mm, 4.65 * mm, 2.5 * mm, 0.0, true),
		sheet("Avery L7651", tr("Mini labels"), 38.1 * mm, 21.2 * mm, 5, 13, 10.7 * mm, 4.75 * mm, 2.5 * mm, 0.0, true)
	};
}

QPixmap LabelPicker::sheetPreview(const LabelSpec& spec, int height)
{
	const qreal k = (height - 8) / spec.pageHeight;
	const QSizeF pageSize(spec.pageWidth * k, spec.pageHeight * k);
	QPixmap pm(qCeil(pageSize.width()) + 8, height);
	pm.fill(Qt::transparent);
	QPainter p(&pm);
	p.setRenderHint(QPainter::Antialiasing);
	const QRectF page(3, 3, pageSize.width(), pageSize.height());
	p.setPen(Qt::NoPen);
	p.setBrush(QColor(0, 0, 0, 40));
	p.drawRect(page.translated(2, 2));
	p.setBrush(Qt::white);
	p.setPen(QPen(QColor(120, 120, 126), 1));
	p.drawRect(page);
	p.setPen(QPen(plum, 1));
	p.setBrush(QColor(plum.red(), plum.green(), plum.blue(), 35));
	for (int r = 0; r < spec.rows; ++r)
		for (int c = 0; c < spec.columns; ++c)
		{
			const QRectF label(page.left() + (spec.sideMargin + c * (spec.labelWidth + spec.gapAcross)) * k,
							   page.top() + (spec.topMargin + r * (spec.labelHeight + spec.gapDown)) * k,
							   spec.labelWidth * k, spec.labelHeight * k);
			p.drawRoundedRect(label.adjusted(0.5, 0.5, -0.5, -0.5), 2, 2);
		}
	return pm;
}

LabelPicker::LabelPicker(QWidget* parent) : QDialog(parent),
	m_presets(presets())
{
	setObjectName("LabelPicker");
	setWindowTitle(tr("Labels"));
	setWindowIcon(IconManager::instance().loadIcon("app-icon"));
	setModal(true);

	auto* mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(20, 20, 20, 16);
	mainLayout->setSpacing(12);

	auto* title = new QLabel(tr("Which labels do you have?"), this);
	QFont titleFont(title->font());
	titleFont.setPointSizeF(titleFont.pointSizeF() * 1.5);
	titleFont.setBold(true);
	title->setFont(titleFont);
	mainLayout->addWidget(title);
	auto* hint = new QLabel(tr("Look for the product number on the label package, for example 5160."), this);
	mainLayout->addWidget(hint);

	auto* body = new QHBoxLayout;
	body->setSpacing(16);

	auto* left = new QVBoxLayout;
	m_search = new QLineEdit(this);
	m_search->setPlaceholderText(tr("Label number, or a word like \"shipping\""));
	m_search->setClearButtonEnabled(true);
	left->addWidget(m_search);
	m_list = new QListWidget(this);
	m_list->setMinimumWidth(300);
	m_list->setIconSize(QSize(34, 44));
	m_list->setSpacing(1);
	for (int i = 0; i < m_presets.count(); ++i)
	{
		const LabelSpec& s = m_presets.at(i);
		auto* item = new QListWidgetItem(QIcon(sheetPreview(s, 44)), s.name + "\n" + s.kind + ", " + tr("%n per sheet", nullptr, s.count()), m_list);
		item->setData(Qt::UserRole, i);
		item->setData(Qt::UserRole + 1, s.name + " " + s.kind);
	}
	auto* custom = new QListWidgetItem(tr("Custom size...") + "\n" + tr("Measure your own sheet"), m_list);
	custom->setData(Qt::UserRole, -1);
	custom->setData(Qt::UserRole + 1, QString(customKey) + " " + tr("custom"));
	left->addWidget(m_list, 1);
	body->addLayout(left, 1);

	auto* right = new QVBoxLayout;
	m_preview = new QLabel(this);
	m_preview->setAlignment(Qt::AlignCenter);
	m_preview->setMinimumSize(200, 252);
	right->addWidget(m_preview);
	m_details = new QLabel(this);
	m_details->setWordWrap(true);
	m_details->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
	m_details->setMinimumWidth(240);
	right->addWidget(m_details);

	m_customBox = new QGroupBox(tr("Your labels (inches)"), this);
	auto* form = new QFormLayout(m_customBox);
	auto makeLength = [this](double max) {
		auto* spin = new QDoubleSpinBox(this);
		spin->setDecimals(3);
		spin->setRange(0.0, max);
		spin->setSingleStep(0.0625);
		spin->setSuffix(" in");
		return spin;
	};
	auto makeCount = [this] {
		auto* spin = new QSpinBox(this);
		spin->setRange(1, 40);
		return spin;
	};
	m_width = makeLength(17);
	m_height = makeLength(17);
	m_columns = makeCount();
	m_rows = makeCount();
	m_top = makeLength(5);
	m_side = makeLength(5);
	m_gapAcross = makeLength(5);
	m_gapDown = makeLength(5);
	form->addRow(tr("Label width:"), m_width);
	form->addRow(tr("Label height:"), m_height);
	form->addRow(tr("Labels across:"), m_columns);
	form->addRow(tr("Labels down:"), m_rows);
	form->addRow(tr("Top edge to first label:"), m_top);
	form->addRow(tr("Left edge to first label:"), m_side);
	form->addRow(tr("Space between columns:"), m_gapAcross);
	form->addRow(tr("Space between rows:"), m_gapDown);
	right->addWidget(m_customBox);
	right->addStretch(1);
	body->addLayout(right);
	mainLayout->addLayout(body, 1);

	auto* textBox = new QGroupBox(tr("What goes on the labels?"), this);
	auto* textLayout = new QVBoxLayout(textBox);
	m_blank = new QRadioButton(tr("Leave them blank, I'll type each one"), textBox);
	m_same = new QRadioButton(tr("The same thing on every label, like a return address:"), textBox);
	m_text = new QPlainTextEdit(textBox);
	m_text->setPlaceholderText(tr("Your Name\n123 Main Street\nYour Town, ST 12345"));
	m_text->setTabChangesFocus(true);
	m_text->setFixedHeight(m_text->fontMetrics().lineSpacing() * 4 + 12);
	m_blank->setChecked(true);
	textLayout->addWidget(m_blank);
	textLayout->addWidget(m_same);
	textLayout->addWidget(m_text);
	mainLayout->addWidget(textBox);
	connect(m_text, &QPlainTextEdit::textChanged, this, [this] {
		if (!m_text->toPlainText().isEmpty())
			m_same->setChecked(true);
	});

	auto* buttons = new QDialogButtonBox(this);
	m_create = buttons->addButton(tr("Create Labels"), QDialogButtonBox::AcceptRole);
	m_create->setDefault(true);
	buttons->addButton(QDialogButtonBox::Cancel);
	connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
	mainLayout->addWidget(buttons);

	// Start from the sheet used last time
	PrefsContext* prefs = PrefsManager::instance().prefsFile->getContext("labelpicker");
	const QStringList c = prefs->get(customKey, "2.625,1,3,10,0.5,0.1875,0.125,0").split(',');
	if (c.count() == 8)
	{
		m_width->setValue(c[0].toDouble());
		m_height->setValue(c[1].toDouble());
		m_columns->setValue(c[2].toInt());
		m_rows->setValue(c[3].toInt());
		m_top->setValue(c[4].toDouble());
		m_side->setValue(c[5].toDouble());
		m_gapAcross->setValue(c[6].toDouble());
		m_gapDown->setValue(c[7].toDouble());
	}
	const QString last = prefs->get("last", m_presets.first().name);
	int row = 0;
	for (int i = 0; i < m_list->count(); ++i)
	{
		const int index = m_list->item(i)->data(Qt::UserRole).toInt();
		if ((index < 0 && last == customKey) || (index >= 0 && m_presets.at(index).name == last))
			row = i;
	}

	connect(m_search, &QLineEdit::textChanged, this, &LabelPicker::filter);
	connect(m_list, &QListWidget::currentRowChanged, this, &LabelPicker::selectionChanged);
	connect(m_list, &QListWidget::itemDoubleClicked, this, [this] { if (m_create->isEnabled()) accept(); });
	for (QDoubleSpinBox* spin : { m_width, m_height, m_top, m_side, m_gapAcross, m_gapDown })
		connect(spin, &QDoubleSpinBox::valueChanged, this, &LabelPicker::updatePreview);
	connect(m_columns, &QSpinBox::valueChanged, this, &LabelPicker::updatePreview);
	connect(m_rows, &QSpinBox::valueChanged, this, &LabelPicker::updatePreview);
	connect(this, &QDialog::accepted, this, [this] {
		PrefsContext* prefs = PrefsManager::instance().prefsFile->getContext("labelpicker");
		prefs->set("last", customSelected() ? QString(customKey) : spec().name);
		if (customSelected())
		{
			const LabelSpec s = customSpec();
			prefs->set(customKey, QStringList{ inches(s.labelWidth), inches(s.labelHeight), QString::number(s.columns), QString::number(s.rows),
				inches(s.topMargin), inches(s.sideMargin), inches(s.gapAcross), inches(s.gapDown) }.join(','));
		}
	});

	m_list->setCurrentRow(row);
	selectionChanged();
	m_search->setFocus();
}

bool LabelPicker::customSelected() const
{
	const QListWidgetItem* item = m_list->currentItem();
	return item && item->data(Qt::UserRole).toInt() < 0;
}

LabelSpec LabelPicker::customSpec() const
{
	LabelSpec s;
	s.name = tr("Custom labels");
	s.kind = tr("Labels");
	s.labelWidth = m_width->value();
	s.labelHeight = m_height->value();
	s.columns = m_columns->value();
	s.rows = m_rows->value();
	s.topMargin = m_top->value();
	s.sideMargin = m_side->value();
	s.gapAcross = m_gapAcross->value();
	s.gapDown = m_gapDown->value();
	return s;
}

LabelSpec LabelPicker::spec() const
{
	if (customSelected())
		return customSpec();
	const QListWidgetItem* item = m_list->currentItem();
	return m_presets.value(item ? item->data(Qt::UserRole).toInt() : 0);
}

QString LabelPicker::repeatedText() const
{
	return m_same->isChecked() ? m_text->toPlainText().trimmed() : QString();
}

void LabelPicker::filter(const QString& text)
{
	const QString needle = text.trimmed();
	int firstVisible = -1;
	for (int i = 0; i < m_list->count(); ++i)
	{
		QListWidgetItem* item = m_list->item(i);
		const bool isCustom = item->data(Qt::UserRole).toInt() < 0;
		// Custom stays available: if your number isn't listed, measure it
		const bool show = needle.isEmpty() || isCustom || item->data(Qt::UserRole + 1).toString().contains(needle, Qt::CaseInsensitive);
		item->setHidden(!show);
		if (show && !isCustom && firstVisible < 0)
			firstVisible = i;
	}
	if (firstVisible >= 0 && (!m_list->currentItem() || m_list->currentItem()->isHidden() || customSelected()))
		m_list->setCurrentRow(firstVisible);
	else if (firstVisible < 0)
		m_list->setCurrentRow(m_list->count() - 1);
}

void LabelPicker::selectionChanged()
{
	m_customBox->setVisible(customSelected());
	updatePreview();
}

void LabelPicker::updatePreview()
{
	const LabelSpec s = spec();
	m_preview->setPixmap(sheetPreview(s, 252));
	const bool fits = s.fitsPage();
	if (fits)
		m_details->setText("<b>" + s.name.toHtmlEscaped() + "</b><br>" + s.kind.toHtmlEscaped() + "<br>" + s.summary().toHtmlEscaped());
	else
		m_details->setText("<b>" + s.name.toHtmlEscaped() + "</b><br><span style='color:#b00020'>"
			+ tr("These labels don't fit on the page. Check the sizes and the number across and down.").toHtmlEscaped() + "</span>");
	m_create->setEnabled(fits);
}
