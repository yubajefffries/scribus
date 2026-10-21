/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/

#include "startscreen.h"

#include <QCheckBox>
#include <QDir>
#include <QFileInfo>
#include <QFont>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPainter>
#include <QPushButton>
#include <QToolButton>
#include <QVBoxLayout>

#include "branding.h"
#include "iconmanager.h"
#include "prefsmanager.h"

namespace
{
	const QColor plum(126, 63, 95);
	const QColor sketch(150, 150, 156);
	const QColor paperEdge(120, 120, 126);
}

StartScreen::StartScreen(QWidget* parent, const QStringList& recentDocs, bool startUp) : QDialog(parent)
{
	setObjectName("StartScreen");
	setWindowTitle(tr("Welcome to %1").arg(Branding::productName()));
	setWindowIcon(IconManager::instance().loadIcon("app-icon"));
	setModal(true);

	auto* mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(20, 20, 20, 16);
	mainLayout->setSpacing(14);

	auto* header = new QHBoxLayout;
	auto* logo = new QLabel(this);
	logo->setPixmap(IconManager::instance().loadPixmap("app-icon", 48));
	header->addWidget(logo);
	auto* title = new QLabel(tr("What would you like to make?"), this);
	QFont titleFont(title->font());
	titleFont.setPointSizeF(titleFont.pointSizeF() * 1.7);
	titleFont.setBold(true);
	title->setFont(titleFont);
	header->addWidget(title, 1);
	mainLayout->addLayout(header);

	auto* body = new QHBoxLayout;
	body->setSpacing(20);

	struct Tile { Choice kind; QString name; QString size; };
	const QList<Tile> tiles {
		{ Flyer, tr("Flyer"), tr("8.5 x 11 in") },
		{ BusinessCard, tr("Business Card"), tr("3.5 x 2 in") },
		{ LabelSheet, tr("Address Labels"), tr("30 per sheet (Avery 5160)") },
		{ Newsletter, tr("Newsletter"), tr("8.5 x 11 in, 2 columns") },
		{ Postcard, tr("Postcard"), tr("6 x 4 in") },
		{ BlankPage, tr("Blank Page"), tr("8.5 x 11 in") }
	};
	auto* grid = new QGridLayout;
	grid->setSpacing(10);
	for (int i = 0; i < tiles.count(); ++i)
	{
		const Tile& t = tiles.at(i);
		auto* button = new QToolButton(this);
		button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		button->setIcon(QIcon(preview(t.kind, 112)));
		button->setIconSize(QSize(112, 112));
		button->setText(t.name + "\n" + t.size);
		button->setFixedSize(176, 170);
		button->setAutoRaise(false);
		button->setCursor(Qt::PointingHandCursor);
		button->setAccessibleName(t.name);
		connect(button, &QToolButton::clicked, this, [this, t] { choose(t.kind); });
		grid->addWidget(button, i / 3, i % 3);
	}
	body->addLayout(grid);

	auto* side = new QVBoxLayout;
	side->setSpacing(8);
	auto* openPub = new QPushButton(IconManager::instance().loadIcon("document-open"), tr("Open a Publisher File..."), this);
	openPub->setToolTip(tr("Open a Microsoft Publisher (.pub) file and keep working on it"));
	openPub->setMinimumHeight(36);
	connect(openPub, &QPushButton::clicked, this, [this] { choose(OpenPublisher); });
	side->addWidget(openPub);
	auto* openOther = new QPushButton(tr("Open Another File..."), this);
	openOther->setMinimumHeight(30);
	connect(openOther, &QPushButton::clicked, this, [this] { choose(OpenOther); });
	side->addWidget(openOther);

	auto* recentLabel = new QLabel(tr("Recent"), this);
	QFont recentFont(recentLabel->font());
	recentFont.setBold(true);
	recentLabel->setFont(recentFont);
	side->addSpacing(8);
	side->addWidget(recentLabel);
	m_recentList = new QListWidget(this);
	m_recentList->setMinimumWidth(240);
	for (const QString& path : recentDocs)
	{
		QFileInfo fi(path);
		if (!fi.exists())
			continue;
		auto* item = new QListWidgetItem(fi.fileName(), m_recentList);
		item->setToolTip(QDir::toNativeSeparators(path));
		item->setData(Qt::UserRole, path);
	}
	if (m_recentList->count() == 0)
	{
		auto* item = new QListWidgetItem(tr("Nothing yet"), m_recentList);
		item->setFlags(Qt::NoItemFlags);
	}
	connect(m_recentList, &QListWidget::itemClicked, this, &StartScreen::openRecent);
	connect(m_recentList, &QListWidget::itemActivated, this, &StartScreen::openRecent);
	side->addWidget(m_recentList, 1);
	body->addLayout(side, 1);
	mainLayout->addLayout(body, 1);

	auto* footer = new QHBoxLayout;
	if (startUp)
	{
		m_showAtStartup = new QCheckBox(tr("Show this screen when %1 starts").arg(Branding::productName()), this);
		m_showAtStartup->setChecked(PrefsManager::instance().appPrefs.uiPrefs.showStartupDialog);
		footer->addWidget(m_showAtStartup);
	}
	footer->addStretch(1);
	auto* more = new QPushButton(tr("More Options..."), this);
	more->setToolTip(tr("Any page size, margins, columns, bleed and templates"));
	connect(more, &QPushButton::clicked, this, [this] { choose(MoreOptions); });
	footer->addWidget(more);
	auto* close = new QPushButton(startUp ? tr("Start Without a Document") : tr("Cancel"), this);
	connect(close, &QPushButton::clicked, this, &QDialog::reject);
	footer->addWidget(close);
	mainLayout->addLayout(footer);
}

bool StartScreen::showAtStartup() const
{
	return m_showAtStartup ? m_showAtStartup->isChecked() : PrefsManager::instance().appPrefs.uiPrefs.showStartupDialog;
}

void StartScreen::choose(Choice c)
{
	m_choice = c;
	accept();
}

void StartScreen::openRecent(QListWidgetItem* item)
{
	if (!item || !(item->flags() & Qt::ItemIsEnabled))
		return;
	m_recentFile = item->data(Qt::UserRole).toString();
	if (!m_recentFile.isEmpty())
		choose(OpenRecent);
}

// Small drawing of each kind of job, on a sheet with the job's proportions.
QPixmap StartScreen::preview(Choice kind, int size)
{
	const qreal dpr = 2.0;
	QPixmap pm(QSize(size, size) * dpr);
	pm.setDevicePixelRatio(dpr);
	pm.fill(Qt::transparent);

	QSizeF paper(8.5, 11);
	if (kind == BusinessCard)
		paper = QSizeF(3.5, 2);
	else if (kind == Postcard)
		paper = QSizeF(6, 4);
	const qreal scale = (size - 8) / qMax(paper.width(), paper.height());
	const QSizeF ps = paper * scale;
	const QRectF page((size - ps.width()) / 2, (size - ps.height()) / 2, ps.width(), ps.height());

	QPainter p(&pm);
	p.setRenderHint(QPainter::Antialiasing);
	p.setPen(Qt::NoPen);
	p.setBrush(QColor(0, 0, 0, 40));
	p.drawRect(page.translated(2, 2));
	p.setBrush(Qt::white);
	p.setPen(QPen(paperEdge, 1));
	p.drawRect(page);

	const qreal u = page.width() / 20.0; // one "unit" across the sheet
	const QRectF in = page.adjusted(2 * u, 2 * u, -2 * u, -2 * u);
	auto lines = [&p](const QRectF& r, int count, qreal gap, qreal width)
	{
		p.setPen(QPen(sketch, width, Qt::SolidLine, Qt::RoundCap));
		for (int i = 0; i < count; ++i)
		{
			const qreal y = r.top() + i * gap;
			if (y > r.bottom())
				break;
			const qreal right = (i == count - 1) ? r.left() + r.width() * 0.6 : r.right();
			p.drawLine(QPointF(r.left(), y), QPointF(right, y));
		}
	};
	auto picture = [&p](const QRectF& r)
	{
		p.setPen(Qt::NoPen);
		p.setBrush(QColor(plum.red(), plum.green(), plum.blue(), 45));
		p.drawRect(r);
		QPolygonF hills;
		hills << QPointF(r.left(), r.bottom()) << QPointF(r.left() + r.width() * 0.35, r.top() + r.height() * 0.45)
			<< QPointF(r.left() + r.width() * 0.55, r.top() + r.height() * 0.7) << QPointF(r.left() + r.width() * 0.75, r.top() + r.height() * 0.35)
			<< QPointF(r.right(), r.bottom());
		p.setBrush(QColor(plum.red(), plum.green(), plum.blue(), 120));
		p.drawPolygon(hills);
		const qreal d = r.height() * 0.22;
		p.drawEllipse(QRectF(r.left() + r.width() * 0.15, r.top() + r.height() * 0.15, d, d));
	};

	switch (kind)
	{
		case Flyer:
			p.setPen(Qt::NoPen);
			p.setBrush(plum);
			p.drawRect(QRectF(in.left(), in.top(), in.width(), 2.4 * u));
			picture(QRectF(in.left(), in.top() + 4 * u, in.width(), 9 * u));
			lines(QRectF(in.left(), in.top() + 15.5 * u, in.width(), in.height() - 15.5 * u), 4, 1.6 * u, 1.2);
			break;
		case BusinessCard:
		{
			p.setPen(Qt::NoPen);
			p.setBrush(plum);
			const qreal d = in.height() * 0.55;
			p.drawEllipse(QRectF(in.left(), in.top() + (in.height() - d) / 2, d, d));
			const QRectF text(in.left() + d + 2 * u, in.top() + in.height() * 0.25, in.width() - d - 2 * u, in.height() * 0.6);
			p.setPen(QPen(QColor(70, 70, 76), 2.2, Qt::SolidLine, Qt::RoundCap));
			p.drawLine(QPointF(text.left(), text.top()), QPointF(text.right() - 2 * u, text.top()));
			lines(text.adjusted(0, 2.4 * u, 0, 0), 2, 1.8 * u, 1.2);
			break;
		}
		case LabelSheet:
		{
			// Avery 5160 geometry, inches: 3 x 10 labels of 2.625 x 1, 0.5 top margin, 0.1875 side margin
			const qreal k = page.width() / 8.5;
			p.setPen(QPen(sketch, 1));
			p.setBrush(QColor(plum.red(), plum.green(), plum.blue(), 30));
			for (int r = 0; r < 10; ++r)
				for (int c = 0; c < 3; ++c)
					p.drawRoundedRect(QRectF(page.left() + (0.1875 + c * 2.75) * k, page.top() + (0.5 + r) * k, 2.625 * k, 1.0 * k).adjusted(0.6, 0.6, -0.6, -0.6), 1.5, 1.5);
			break;
		}
		case Newsletter:
		{
			p.setPen(Qt::NoPen);
			p.setBrush(plum);
			p.drawRect(QRectF(in.left(), in.top(), in.width(), 3 * u));
			const qreal top = in.top() + 4.5 * u;
			const qreal colW = (in.width() - u) / 2;
			picture(QRectF(in.left(), top, colW, 5 * u));
			lines(QRectF(in.left(), top + 6.5 * u, colW, in.bottom() - top - 6.5 * u), 9, 1.4 * u, 1.0);
			lines(QRectF(in.left() + colW + u, top, colW, in.bottom() - top), 13, 1.4 * u, 1.0);
			break;
		}
		case Postcard:
		{
			const QRectF half(in.left(), in.top(), in.width() * 0.55, in.height());
			picture(half);
			p.setPen(QPen(sketch, 1));
			const qreal x = in.left() + in.width() * 0.62;
			p.drawRect(QRectF(in.right() - 2.2 * u, in.top(), 2.2 * u, 2.6 * u));
			lines(QRectF(x, in.top() + in.height() * 0.45, in.right() - x, in.height() * 0.5), 3, 2 * u, 1.2);
			break;
		}
		default:
			break;
	}
	return pm;
}
