/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/

#include "pubconvertdialog.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QDirIterator>
#include <QFileDialog>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QStandardPaths>
#include <QVBoxLayout>

#include "branding.h"
#include "iconmanager.h"
#include "prefscontext.h"
#include "prefsfile.h"
#include "prefsmanager.h"

PubConvertDialog::PubConvertDialog(QWidget* parent) : QDialog(parent)
{
	setObjectName("PubConvertDialog");
	setWindowTitle(tr("Convert Publisher Files"));
	setWindowIcon(IconManager::instance().loadIcon("app-icon"));
	setModal(true);
	setMinimumWidth(560);

	auto* mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(20, 20, 20, 16);
	mainLayout->setSpacing(12);

	auto* title = new QLabel(tr("Convert a folder of Publisher files"), this);
	QFont titleFont(title->font());
	titleFont.setPointSizeF(titleFont.pointSizeF() * 1.5);
	titleFont.setBold(true);
	title->setFont(titleFont);
	mainLayout->addWidget(title);
	auto* intro = new QLabel(tr("%1 opens each Publisher (.pub) file and saves a %1 copy you can edit. "
		"Your original files are not changed, and files that already have a copy are skipped.").arg(Branding::productName()), this);
	intro->setWordWrap(true);
	mainLayout->addWidget(intro);

	PrefsContext* prefs = PrefsManager::instance().prefsFile->getContext("pubconvert");
	QString startDir = prefs->get("source");
	if (startDir.isEmpty())
		startDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

	auto* sourceBox = new QGroupBox(tr("Folder with your Publisher files"), this);
	auto* sourceLayout = new QGridLayout(sourceBox);
	m_source = new QLineEdit(QDir::toNativeSeparators(startDir), sourceBox);
	auto* sourceBrowse = new QPushButton(tr("Browse..."), sourceBox);
	m_subfolders = new QCheckBox(tr("Include the folders inside it"), sourceBox);
	m_subfolders->setChecked(prefs->getBool("subfolders", true));
	m_found = new QLabel(sourceBox);
	sourceLayout->addWidget(m_source, 0, 0);
	sourceLayout->addWidget(sourceBrowse, 0, 1);
	sourceLayout->addWidget(m_subfolders, 1, 0, 1, 2);
	sourceLayout->addWidget(m_found, 2, 0, 1, 2);
	mainLayout->addWidget(sourceBox);

	auto* targetBox = new QGroupBox(tr("Where should the copies go?"), this);
	auto* targetLayout = new QGridLayout(targetBox);
	m_besideOriginals = new QRadioButton(tr("Next to each original file"), targetBox);
	m_inFolder = new QRadioButton(tr("All in this folder:"), targetBox);
	m_target = new QLineEdit(QDir::toNativeSeparators(prefs->get("target")), targetBox);
	m_targetBrowse = new QPushButton(tr("Browse..."), targetBox);
	(prefs->getBool("besideOriginals", true) ? m_besideOriginals : m_inFolder)->setChecked(true);
	targetLayout->addWidget(m_besideOriginals, 0, 0, 1, 3);
	targetLayout->addWidget(m_inFolder, 1, 0);
	targetLayout->addWidget(m_target, 1, 1);
	targetLayout->addWidget(m_targetBrowse, 1, 2);
	m_pdf = new QCheckBox(tr("Also save a PDF of each one (good for sharing and printing)"), targetBox);
	m_pdf->setChecked(prefs->getBool("pdf", false));
	targetLayout->addWidget(m_pdf, 2, 0, 1, 3);
	mainLayout->addWidget(targetBox);

	auto* buttons = new QDialogButtonBox(this);
	m_convert = buttons->addButton(tr("Convert"), QDialogButtonBox::AcceptRole);
	m_convert->setDefault(true);
	buttons->addButton(QDialogButtonBox::Cancel);
	connect(buttons, &QDialogButtonBox::accepted, this, [this] { remember(); accept(); });
	connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
	mainLayout->addWidget(buttons);

	connect(sourceBrowse, &QPushButton::clicked, this, &PubConvertDialog::browseSource);
	connect(m_targetBrowse, &QPushButton::clicked, this, &PubConvertDialog::browseTarget);
	connect(m_source, &QLineEdit::textChanged, this, &PubConvertDialog::rescan);
	connect(m_target, &QLineEdit::textChanged, this, &PubConvertDialog::rescan);
	connect(m_subfolders, &QCheckBox::toggled, this, &PubConvertDialog::rescan);
	connect(m_besideOriginals, &QRadioButton::toggled, this, &PubConvertDialog::rescan);
	rescan();
}

QString PubConvertDialog::sourceFolder() const
{
	return QDir::fromNativeSeparators(m_source->text().trimmed());
}

QString PubConvertDialog::targetFolder() const
{
	return m_besideOriginals->isChecked() ? QString() : QDir::fromNativeSeparators(m_target->text().trimmed());
}

bool PubConvertDialog::makePdf() const
{
	return m_pdf->isChecked();
}

void PubConvertDialog::browseSource()
{
	const QString dir = QFileDialog::getExistingDirectory(this, tr("Folder with your Publisher files"), sourceFolder());
	if (!dir.isEmpty())
		m_source->setText(QDir::toNativeSeparators(dir));
}

void PubConvertDialog::browseTarget()
{
	const QString start = m_target->text().isEmpty() ? sourceFolder() : QDir::fromNativeSeparators(m_target->text());
	const QString dir = QFileDialog::getExistingDirectory(this, tr("Where should the copies go?"), start);
	if (dir.isEmpty())
		return;
	m_target->setText(QDir::toNativeSeparators(dir));
	m_inFolder->setChecked(true);
}

void PubConvertDialog::rescan()
{
	m_files.clear();
	m_target->setEnabled(m_inFolder->isChecked());
	m_targetBrowse->setEnabled(m_inFolder->isChecked());
	const QString dir = sourceFolder();
	if (dir.isEmpty() || !QDir(dir).exists())
	{
		m_found->setText(tr("Choose the folder that holds your .pub files."));
		m_convert->setEnabled(false);
		return;
	}
	QDirIterator it(dir, QStringList{ "*.pub", "*.PUB" }, QDir::Files,
		m_subfolders->isChecked() ? QDirIterator::Subdirectories : QDirIterator::NoIteratorFlags);
	while (it.hasNext())
		m_files.append(it.next());
	m_files.removeDuplicates();
	m_files.sort(Qt::CaseInsensitive);
	if (m_files.isEmpty())
		m_found->setText(tr("No Publisher files in this folder."));
	else
		m_found->setText("<b>" + (m_files.count() == 1 ? tr("Found 1 Publisher file.") : tr("Found %1 Publisher files.").arg(m_files.count())) + "</b>");
	const bool targetOk = m_besideOriginals->isChecked() || !m_target->text().trimmed().isEmpty();
	m_convert->setEnabled(!m_files.isEmpty() && targetOk);
}

void PubConvertDialog::remember()
{
	PrefsContext* prefs = PrefsManager::instance().prefsFile->getContext("pubconvert");
	prefs->set("source", sourceFolder());
	prefs->set("subfolders", m_subfolders->isChecked());
	prefs->set("besideOriginals", m_besideOriginals->isChecked());
	prefs->set("target", QDir::fromNativeSeparators(m_target->text().trimmed()));
	prefs->set("pdf", m_pdf->isChecked());
}
