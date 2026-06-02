#include "metadataeditor.h"
#include "audiofile.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLineEdit>
#include <QTextEdit>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QGroupBox>
#include <QFileDialog>
#include <QPixmap>
#include <memory>

MetadataEditor::MetadataEditor(QWidget *parent)
    : QWidget(parent),
      m_titleEdit(new QLineEdit()),
      m_artistEdit(new QLineEdit()),
      m_albumEdit(new QLineEdit()),
      m_albumArtistEdit(new QLineEdit()),
      m_genreEdit(new QLineEdit()),
      m_yearEdit(new QLineEdit()),
      m_trackEdit(new QLineEdit()),
      m_commentEdit(new QTextEdit()),
      m_filenameLabel(new QLabel(tr("No file selected"))),
      m_formatLabel(new QLabel()),
      m_durationLabel(new QLabel()),
      m_bitrateLabel(new QLabel()),
      m_sampleRateLabel(new QLabel()),
      m_coverArtLabel(new QLabel()),
      m_applyButton(new QPushButton(tr("Apply"))),
      m_revertButton(new QPushButton(tr("Revert"))),
      m_coverArtButton(new QPushButton(tr("Load Cover Art")))
{
    setupUI();
}

MetadataEditor::~MetadataEditor() = default;

void MetadataEditor::setupUI()
{
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(5, 5, 5, 5);

    // File info section
    auto fileInfoGroup = new QGroupBox(tr("File Information"));
    auto fileInfoLayout = new QGridLayout(fileInfoGroup);

    fileInfoLayout->addWidget(new QLabel(tr("Filename:")), 0, 0);
    fileInfoLayout->addWidget(m_filenameLabel, 0, 1);

    fileInfoLayout->addWidget(new QLabel(tr("Format:")), 1, 0);
    fileInfoLayout->addWidget(m_formatLabel, 1, 1);

    fileInfoLayout->addWidget(new QLabel(tr("Duration:")), 2, 0);
    fileInfoLayout->addWidget(m_durationLabel, 2, 1);

    fileInfoLayout->addWidget(new QLabel(tr("Bitrate:")), 3, 0);
    fileInfoLayout->addWidget(m_bitrateLabel, 3, 1);

    fileInfoLayout->addWidget(new QLabel(tr("Sample Rate:")), 4, 0);
    fileInfoLayout->addWidget(m_sampleRateLabel, 4, 1);

    mainLayout->addWidget(fileInfoGroup);

    // Metadata editor section
    auto metadataGroup = new QGroupBox(tr("Metadata"));
    auto metadataLayout = new QGridLayout(metadataGroup);

    // Setup metadata fields with labels
    metadataLayout->addWidget(new QLabel(tr("Title:")), 0, 0);
    metadataLayout->addWidget(m_titleEdit, 0, 1);

    metadataLayout->addWidget(new QLabel(tr("Artist:")), 1, 0);
    metadataLayout->addWidget(m_artistEdit, 1, 1);

    metadataLayout->addWidget(new QLabel(tr("Album:")), 2, 0);
    metadataLayout->addWidget(m_albumEdit, 2, 1);

    metadataLayout->addWidget(new QLabel(tr("Album Artist:")), 3, 0);
    metadataLayout->addWidget(m_albumArtistEdit, 3, 1);

    metadataLayout->addWidget(new QLabel(tr("Genre:")), 4, 0);
    metadataLayout->addWidget(m_genreEdit, 4, 1);

    metadataLayout->addWidget(new QLabel(tr("Year:")), 5, 0);
    metadataLayout->addWidget(m_yearEdit, 5, 1);

    metadataLayout->addWidget(new QLabel(tr("Track Number:")), 6, 0);
    metadataLayout->addWidget(m_trackEdit, 6, 1);

    metadataLayout->addWidget(new QLabel(tr("Comments:")), 7, 0, Qt::AlignTop);
    metadataLayout->addWidget(m_commentEdit, 7, 1);
    m_commentEdit->setMaximumHeight(60);

    mainLayout->addWidget(metadataGroup);

    // Cover art section
    auto coverArtGroup = new QGroupBox(tr("Cover Art"));
    auto coverArtLayout = new QVBoxLayout(coverArtGroup);

    m_coverArtLabel->setMinimumSize(120, 120);
    m_coverArtLabel->setMaximumSize(120, 120);
    m_coverArtLabel->setAlignment(Qt::AlignCenter);
    m_coverArtLabel->setStyleSheet("border: 1px solid #cccccc;");
    m_coverArtLabel->setPixmap(QPixmap(":/icons/music.svg").scaledToWidth(120, Qt::SmoothTransformation));

    coverArtLayout->addWidget(m_coverArtLabel, 0, Qt::AlignCenter);
    coverArtLayout->addWidget(m_coverArtButton);

    mainLayout->addWidget(coverArtGroup);

    // Buttons
    auto buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_revertButton);
    buttonLayout->addWidget(m_applyButton);

    mainLayout->addLayout(buttonLayout);

    // Connect signals
    connect(m_applyButton, &QPushButton::clicked, this, &MetadataEditor::onApplyClicked);
    connect(m_revertButton, &QPushButton::clicked, this, &MetadataEditor::onRevertClicked);
    connect(m_coverArtButton, &QPushButton::clicked, this, &MetadataEditor::onCoverArtClicked);

    // Disable by default
    clearDisplay();
}

void MetadataEditor::displayFile(const QString &filePath)
{
    m_currentFilePath = filePath;

    auto audioFile = std::make_shared<AudioFile>(filePath);
    loadFileInfo(audioFile);
}

void MetadataEditor::clearDisplay()
{
    m_titleEdit->clear();
    m_artistEdit->clear();
    m_albumEdit->clear();
    m_albumArtistEdit->clear();
    m_genreEdit->clear();
    m_yearEdit->clear();
    m_trackEdit->clear();
    m_commentEdit->clear();

    m_filenameLabel->setText(tr("No file selected"));
    m_formatLabel->clear();
    m_durationLabel->clear();
    m_bitrateLabel->clear();
    m_sampleRateLabel->clear();

    m_applyButton->setEnabled(false);
    m_revertButton->setEnabled(false);
    m_coverArtButton->setEnabled(false);

    m_currentFilePath.clear();
    m_originalTags.clear();
    m_currentTags.clear();
}

void MetadataEditor::loadFileInfo(const std::shared_ptr<AudioFile> &audioFile)
{
    m_filenameLabel->setText(audioFile->fileName());
    m_formatLabel->setText(audioFile->fileFormat());
    m_bitrateLabel->setText(QString("%1 kbps").arg(audioFile->bitrate()));
    m_sampleRateLabel->setText(QString("%1 Hz").arg(audioFile->sampleRate()));

    int seconds = audioFile->duration();
    int minutes = seconds / 60;
    int secs = seconds % 60;
    m_durationLabel->setText(QString("%1:%2").arg(minutes).arg(secs, 2, 10, QChar('0')));

    // Load metadata
    m_titleEdit->setText(audioFile->title());
    m_artistEdit->setText(audioFile->artist());
    m_albumEdit->setText(audioFile->album());
    m_albumArtistEdit->setText(audioFile->albumArtist());
    m_genreEdit->setText(audioFile->genre());
    m_yearEdit->setText(audioFile->year());
    m_trackEdit->setText(audioFile->trackNumber());
    m_commentEdit->setText(audioFile->comment());

    // Store original tags
    m_originalTags["TITLE"] = audioFile->title();
    m_originalTags["ARTIST"] = audioFile->artist();
    m_originalTags["ALBUM"] = audioFile->album();
    m_originalTags["ALBUMARTIST"] = audioFile->albumArtist();
    m_originalTags["GENRE"] = audioFile->genre();
    m_originalTags["DATE"] = audioFile->year();
    m_originalTags["TRACKNUMBER"] = audioFile->trackNumber();
    m_originalTags["COMMENT"] = audioFile->comment();

    m_applyButton->setEnabled(true);
    m_revertButton->setEnabled(true);
    m_coverArtButton->setEnabled(true);
}

void MetadataEditor::onApplyClicked()
{
    QMap<QString, QString> tags;
    tags["TITLE"] = m_titleEdit->text();
    tags["ARTIST"] = m_artistEdit->text();
    tags["ALBUM"] = m_albumEdit->text();
    tags["ALBUMARTIST"] = m_albumArtistEdit->text();
    tags["GENRE"] = m_genreEdit->text();
    tags["DATE"] = m_yearEdit->text();
    tags["TRACKNUMBER"] = m_trackEdit->text();
    tags["COMMENT"] = m_commentEdit->toPlainText();

    emit metadataChanged(m_currentFilePath, tags);

    m_originalTags = tags;
}

void MetadataEditor::onRevertClicked()
{
    m_titleEdit->setText(m_originalTags.value("TITLE"));
    m_artistEdit->setText(m_originalTags.value("ARTIST"));
    m_albumEdit->setText(m_originalTags.value("ALBUM"));
    m_albumArtistEdit->setText(m_originalTags.value("ALBUMARTIST"));
    m_genreEdit->setText(m_originalTags.value("GENRE"));
    m_yearEdit->setText(m_originalTags.value("DATE"));
    m_trackEdit->setText(m_originalTags.value("TRACKNUMBER"));
    m_commentEdit->setText(m_originalTags.value("COMMENT"));
}

void MetadataEditor::onCoverArtClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Select Cover Art"), "",
        tr("Image Files (*.png *.jpg *.jpeg *.bmp);;All Files (*)"));

    if (!fileName.isEmpty()) {
        QPixmap pixmap(fileName);
        if (!pixmap.isNull()) {
            m_coverArtPath = fileName;
            m_coverArtLabel->setPixmap(pixmap.scaledToWidth(120, Qt::SmoothTransformation));
        }
    }
}

void MetadataEditor::updatePreview()
{
    // Could update a preview of the file with current metadata
}
