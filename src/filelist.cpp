#include "filelist.h"
#include "audiofile.h"

#include <QVBoxLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QDir>
#include <QFileInfo>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QInputDialog>
#include <QRegularExpression>
#include <QProgressDialog>
#include <QApplication>

FileList::FileList(QWidget *parent)
    : QWidget(parent),
      m_tableWidget(new QTableWidget(this)),
      m_skipCellChangedSignal(false)
{
    setupUI();
}

FileList::~FileList() = default;

void FileList::setupUI()
{
    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableWidget->setSelectionMode(QAbstractItemView::MultiSelection);
    m_tableWidget->setAlternatingRowColors(true);
    m_tableWidget->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::AnyKeyPressed);
    m_tableWidget->horizontalHeader()->setStretchLastSection(true);
    m_tableWidget->horizontalHeader()->setSectionsMovable(true);
    m_tableWidget->verticalHeader()->setDefaultSectionSize(20);

    setupColumns();

    layout->addWidget(m_tableWidget);

    // Connect signals
    connect(m_tableWidget, &QTableWidget::cellClicked,
            this, &FileList::onCellClicked);
    connect(m_tableWidget, &QTableWidget::cellChanged,
            this, &FileList::onCellChanged);
    connect(m_tableWidget, &QTableWidget::customContextMenuRequested,
            this, &FileList::onContextMenuRequested);
}

void FileList::setupColumns()
{
    m_tableWidget->setColumnCount(10);
    m_tableWidget->setHorizontalHeaderLabels({
        tr("Title"),
        tr("Artist"),
        tr("Album"),
        tr("Genre"),
        tr("Year"),
        tr("Track"),
        tr("Duration"),
        tr("Bitrate"),
        tr("Filename"),
        tr("Path")
    });

    // Set column widths
    int widths[] = {150, 120, 150, 80, 50, 40, 60, 60, 150, 200};
    for (int i = 0; i < 10; ++i) {
        m_tableWidget->setColumnWidth(i, widths[i]);
    }
}

void FileList::loadFiles(const QString &folderPath)
{
    m_currentFolder = folderPath;
    m_tableWidget->setRowCount(0);
    m_rowToFilePath.clear();
    m_audioFiles.clear();

    loadAudioFiles(folderPath);
}

void FileList::loadAudioFiles(const QString &folderPath, bool recursive)
{
    QDir dir(folderPath);
    dir.setFilter(QDir::Files | QDir::NoSymLinks);

    // Audio file extensions
    QStringList filters;
    filters << "*.mp3" << "*.flac" << "*.ogg" << "*.m4a" << "*.wav"
            << "*.aac" << "*.opus" << "*.wma";
    dir.setNameFilters(filters);

    const auto files = dir.entryList();
    
    for (const auto &file : files) {
        QString filePath = dir.absoluteFilePath(file);
        auto audioFile = std::make_shared<AudioFile>(filePath);

        int row = m_tableWidget->rowCount();
        m_tableWidget->insertRow(row);

        m_rowToFilePath[row] = filePath;
        m_audioFiles[filePath] = audioFile;

        m_skipCellChangedSignal = true;

        auto titleItem = new QTableWidgetItem(audioFile->title());
        titleItem->setData(Qt::UserRole, filePath);
        m_tableWidget->setItem(row, 0, titleItem);

        m_tableWidget->setItem(row, 1, new QTableWidgetItem(audioFile->artist()));
        m_tableWidget->setItem(row, 2, new QTableWidgetItem(audioFile->album()));
        m_tableWidget->setItem(row, 3, new QTableWidgetItem(audioFile->genre()));
        m_tableWidget->setItem(row, 4, new QTableWidgetItem(audioFile->year()));
        m_tableWidget->setItem(row, 5, new QTableWidgetItem(audioFile->trackNumber()));
        m_tableWidget->setItem(row, 6, new QTableWidgetItem(formatDuration(audioFile->duration())));
        
        QString bitrate = QString::number(audioFile->bitrate()) + " kbps";
        m_tableWidget->setItem(row, 7, new QTableWidgetItem(bitrate));
        
        m_tableWidget->setItem(row, 8, new QTableWidgetItem(audioFile->fileName()));
        m_tableWidget->setItem(row, 9, new QTableWidgetItem(audioFile->filePath()));

        m_skipCellChangedSignal = false;
    }

    // Recursively load subdirectories if enabled
    if (recursive) {
        dir.setFilter(QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot);
        const auto subdirs = dir.entryList();
        
        for (const auto &subdir : subdirs) {
            loadAudioFiles(dir.absoluteFilePath(subdir), true);
        }
    }
}

void FileList::onCellClicked(int row, int column)
{
    Q_UNUSED(column);
    
    if (m_rowToFilePath.contains(row)) {
        emit fileSelected(m_rowToFilePath[row]);
    }
}

void FileList::onCellChanged(int row, int column)
{
    if (m_skipCellChangedSignal) {
        return;
    }

    if (!m_rowToFilePath.contains(row)) {
        return;
    }

    QString filePath = m_rowToFilePath[row];
    if (!m_audioFiles.contains(filePath)) {
        return;
    }

    auto audioFile = m_audioFiles[filePath];
    QTableWidgetItem *item = m_tableWidget->item(row, column);
    if (!item) return;

    QString value = item->text();

    // Map column to tag
    switch (column) {
        case 0: audioFile->setTitle(value); break;
        case 1: audioFile->setArtist(value); break;
        case 2: audioFile->setAlbum(value); break;
        case 3: audioFile->setGenre(value); break;
        case 4: audioFile->setYear(value); break;
        case 5: audioFile->setTrackNumber(value); break;
        default: break;
    }
}

void FileList::selectAll()
{
    m_tableWidget->selectAll();
}

void FileList::deselectAll()
{
    m_tableWidget->clearSelection();
}

void FileList::saveTags()
{
    QProgressDialog progress(tr("Saving tags..."), tr("Cancel"), 0,
                            m_audioFiles.size(), this);
    progress.setWindowModality(Qt::WindowModal);

    int saved = 0;
    int errors = 0;

    for (auto &audioFile : m_audioFiles) {
        if (!audioFile->saveMetadata()) {
            errors++;
        } else {
            saved++;
        }
        progress.setValue(saved + errors);
        QApplication::processEvents();

        if (progress.wasCanceled()) {
            break;
        }
    }

    QString message = tr("Saved %1 files").arg(saved);
    if (errors > 0) {
        message += tr(" (%1 errors)").arg(errors);
    }

    QMessageBox::information(this, tr("Save Complete"), message);
}

void FileList::refresh()
{
    loadFiles(m_currentFolder);
}

void FileList::updateFileMetadata(const QString &filePath, const QMap<QString, QString> &tags)
{
    if (!m_audioFiles.contains(filePath)) {
        return;
    }

    auto audioFile = m_audioFiles[filePath];

    m_skipCellChangedSignal = true;

    for (auto it = tags.begin(); it != tags.end(); ++it) {
        audioFile->setTag(it.key(), it.value());
    }

    // Update table rows
    for (auto it = m_rowToFilePath.begin(); it != m_rowToFilePath.end(); ++it) {
        if (it.value() == filePath) {
            int row = it.key();
            m_tableWidget->item(row, 0)->setText(audioFile->title());
            m_tableWidget->item(row, 1)->setText(audioFile->artist());
            m_tableWidget->item(row, 2)->setText(audioFile->album());
            m_tableWidget->item(row, 3)->setText(audioFile->genre());
            m_tableWidget->item(row, 4)->setText(audioFile->year());
            m_tableWidget->item(row, 5)->setText(audioFile->trackNumber());
            break;
        }
    }

    m_skipCellChangedSignal = false;
}

void FileList::batchRename()
{
    auto rows = m_tableWidget->selectionModel()->selectedRows();
    if (rows.isEmpty()) {
        QMessageBox::warning(this, tr("No Selection"), tr("Please select files to rename."));
        return;
    }

    bool ok;
    QString pattern = QInputDialog::getText(this, tr("Batch Rename"),
                                           tr("Enter pattern (use {title}, {artist}, {album}, {track}):\n"
                                              "Example: {artist} - {title}"),
                                           QLineEdit::Normal,
                                           "{artist} - {title}",
                                           &ok);
    if (!ok || pattern.isEmpty()) {
        return;
    }

    // Implementation would rename files based on pattern
    QMessageBox::information(this, tr("Batch Rename"),
                            tr("Batch rename would be applied to %1 files.").arg(rows.size()));
}

void FileList::autoTagFromFilename()
{
    auto rows = m_tableWidget->selectionModel()->selectedRows();
    if (rows.isEmpty()) {
        QMessageBox::warning(this, tr("No Selection"), tr("Please select files."));
        return;
    }

    // Parse filename pattern: "Artist - Title" or "Artist - Album - Title"
    // This is a simple implementation
    int count = 0;
    for (auto index : rows) {
        int row = index.row();
        if (m_rowToFilePath.contains(row)) {
            QString filePath = m_rowToFilePath[row];
            QFileInfo info(filePath);
            QString filename = info.baseName();

            QStringList parts = filename.split(" - ");
            if (parts.size() >= 2) {
                if (!m_audioFiles.contains(filePath)) continue;
                
                auto audioFile = m_audioFiles[filePath];
                audioFile->setArtist(parts[0].trimmed());
                
                if (parts.size() >= 3) {
                    audioFile->setAlbum(parts[1].trimmed());
                    audioFile->setTitle(parts[2].trimmed());
                } else {
                    audioFile->setTitle(parts[1].trimmed());
                }

                count++;
            }
        }
    }

    QMessageBox::information(this, tr("Auto Tag"),
                            tr("Auto-tagged %1 files from filename.").arg(count));
    
    // Refresh display
    for (auto index : rows) {
        m_tableWidget->viewport()->update(m_tableWidget->visualRect(index));
    }
}

void FileList::onContextMenuRequested(const QPoint &pos)
{
    QMenu menu;

    auto selectAllAction = menu.addAction(tr("Select All"));
    connect(selectAllAction, &QAction::triggered, this, &FileList::selectAll);

    auto deselectAction = menu.addAction(tr("Deselect All"));
    connect(deselectAction, &QAction::triggered, this, &FileList::deselectAll);

    menu.addSeparator();

    auto autoTagAction = menu.addAction(tr("Auto Tag from Filename"));
    connect(autoTagAction, &QAction::triggered, this, &FileList::autoTagFromFilename);

    auto batchRenameAction = menu.addAction(tr("Batch Rename"));
    connect(batchRenameAction, &QAction::triggered, this, &FileList::batchRename);

    menu.addSeparator();

    auto saveAction = menu.addAction(tr("Save Tags"));
    connect(saveAction, &QAction::triggered, this, &FileList::saveTags);

    menu.exec(m_tableWidget->mapToGlobal(pos));
}

QString FileList::formatDuration(int seconds) const
{
    if (seconds <= 0) return "0:00";
    
    int hours = seconds / 3600;
    int minutes = (seconds % 3600) / 60;
    int secs = seconds % 60;

    if (hours > 0) {
        return QString("%1:%2:%3").arg(hours)
                                 .arg(minutes, 2, 10, QChar('0'))
                                 .arg(secs, 2, 10, QChar('0'));
    } else {
        return QString("%1:%2").arg(minutes)
                              .arg(secs, 2, 10, QChar('0'));
    }
}
