#include "mainwindow.h"
#include <QApplication>
#include <QMenuBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QGridLayout>
#include <QFileInfo>
#include <QFileSystemModel>
#include <QDir>
#include <QDirIterator>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QStyleFactory>
#include <QPalette>
#include <QFont>
#include <QAction>
#include <QScrollArea>
#include <QFrame>
#include <algorithm>

// TagLib includes
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/audioproperties.h>
#include <taglib/tstring.h>
#include <taglib/id3v2tag.h>
#include <taglib/mpegfile.h>
#include <taglib/flacfile.h>
#include <taglib/vorbisfile.h>
#include <taglib/mp4file.h>
#include <taglib/asffile.h>
#include <taglib/id3v2framefactory.h>

const QStringList MainWindow::GENRES = {
    "","Blues","Classic Rock","Country","Dance","Disco","Funk","Grunge",
    "Hip-Hop","Jazz","Metal","New Age","Oldies","Other","Pop","R&B","Rap",
    "Reggae","Rock","Techno","Industrial","Alternative","Ska","Death Metal",
    "Pranks","Soundtrack","Euro-Techno","Ambient","Trip-Hop","Vocal",
    "Jazz+Funk","Fusion","Trance","Classical","Instrumental","Acid","House",
    "Game","Sound Clip","Gospel","Noise","Alternative Rock","Bass","Soul",
    "Punk","Space","Meditative","Instrumental Pop","Instrumental Rock",
    "Ethnic","Gothic","Darkwave","Techno-Industrial","Electronic","Pop-Folk",
    "Eurodance","Dream","Southern Rock","Comedy","Cult","Gangsta","Top 40",
    "Christian Rap","Pop/Funk","Jungle","Native American","Cabaret","New Wave",
    "Psychedelic","Rave","Showtunes","Trailer","Lo-Fi","Tribal","Acid Punk",
    "Acid Jazz","Polka","Retro","Musical","Rock & Roll","Hard Rock","Folk",
    "Folk-Rock","National Folk","Swing","Fast Fusion","Bebob","Latin","Revival",
    "Celtic","Bluegrass","Avantgarde","Gothic Rock","Progressive Rock",
    "Psychedelic Rock","Symphonic Rock","Slow Rock","Big Band","Chorus",
    "Easy Listening","Acoustic","Humour","Speech","Chanson","Opera","Chamber Music",
    "Sonata","Symphony","Booty Bass","Primus","Porn Groove","Satire","Slow Jam",
    "Club","Tango","Samba","Folklore","Ballad","Power Ballad","Rhythmic Soul",
    "Freestyle","Duet","Punk Rock","Drum Solo","A Capella","Euro-House",
    "Dance Hall","Goa","Drum & Bass","Club-House","Hardcore","Terror",
    "Indie","Britpop","Negerpunk","Polsk Punk","Beat","Christian Gangsta",
    "Heavy Metal","Black Metal","Crossover","Contemporary C","Christian Rock",
    "Merengue","Salsa","Thrash Metal","Anime","JPop","Synthpop"
};

static QString tstr(const TagLib::String &s) {
    return QString::fromStdString(s.to8Bit(true));
}
static TagLib::String qstr(const QString &s) {
    return TagLib::String(s.toStdString(), TagLib::String::UTF8);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), m_updating(false)
{
    setWindowTitle("tagQ 0.1");
    setMinimumSize(1100, 700);
    resize(1280, 780);
    setAcceptDrops(true);

    setupUi();
    setupMenuBar();
    setupToolBar();
    setupStatusBar();
    connectSignals();
    updateStatusBar();
}

MainWindow::~MainWindow() {}

void MainWindow::setupUi() {
    // Main splitter: folder tree | file list + tag editor
    m_mainSplitter = new QSplitter(Qt::Horizontal, this);
    setCentralWidget(m_mainSplitter);

    // Left: folder tree
    m_folderTree = new QTreeWidget;
    m_folderTree->setHeaderLabel("Folders");
    m_folderTree->setMinimumWidth(180);
    m_folderTree->setMaximumWidth(240);

    // Add drives/home
    auto addRoot = [&](const QString &path, const QString &label) {
        QTreeWidgetItem *item = new QTreeWidgetItem(m_folderTree);
        item->setText(0, label);
        item->setData(0, Qt::UserRole, path);
        // Add dummy child for expand
        new QTreeWidgetItem(item);
    };
    addRoot(QDir::homePath(), QDir::homePath());
    addRoot("/", "/");
    m_folderTree->expandItem(m_folderTree->topLevelItem(0));

    // Center: file table
    m_fileTable = new QTableWidget(0, COL_COUNT);
    m_fileTable->setHorizontalHeaderLabels({"File Name","Title","Artist","Album","Year","Track","Genre","Duration","Bitrate","Size","Path"});
    m_fileTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_fileTable->setAlternatingRowColors(true);
    m_fileTable->setSortingEnabled(true);
    m_fileTable->horizontalHeader()->setStretchLastSection(false);
    m_fileTable->horizontalHeader()->setSectionsMovable(true);
    m_fileTable->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
    m_fileTable->setColumnWidth(COL_FILENAME, 180);
    m_fileTable->setColumnWidth(COL_TITLE, 160);
    m_fileTable->setColumnWidth(COL_ARTIST, 140);
    m_fileTable->setColumnWidth(COL_ALBUM, 140);
    m_fileTable->setColumnWidth(COL_YEAR, 50);
    m_fileTable->setColumnWidth(COL_TRACK, 45);
    m_fileTable->setColumnWidth(COL_GENRE, 100);
    m_fileTable->setColumnWidth(COL_DURATION, 65);
    m_fileTable->setColumnWidth(COL_BITRATE, 65);
    m_fileTable->setColumnWidth(COL_SIZE, 70);
    m_fileTable->setColumnWidth(COL_PATH, 200);
    m_fileTable->verticalHeader()->setDefaultSectionSize(20);
    m_fileTable->verticalHeader()->setVisible(true);

    // Right: tag editor panel
    setupTagEditorPanel();

    // Right splitter: table | tag panel
    QSplitter *rightSplitter = new QSplitter(Qt::Horizontal);
    rightSplitter->addWidget(m_fileTable);
    rightSplitter->addWidget(m_tagPanel);
    rightSplitter->setStretchFactor(0, 3);
    rightSplitter->setStretchFactor(1, 1);
    rightSplitter->setSizes({800, 340});

    m_mainSplitter->addWidget(m_folderTree);
    m_mainSplitter->addWidget(rightSplitter);
    m_mainSplitter->setStretchFactor(0, 0);
    m_mainSplitter->setStretchFactor(1, 1);
    m_mainSplitter->setSizes({200, 1000});
}

void MainWindow::setupTagEditorPanel() {
    m_tagPanel = new QWidget;
    m_tagPanel->setMinimumWidth(300);

    m_tagTabs = new QTabWidget(m_tagPanel);

    // --- Tab 1: Tag Editor ---
    QWidget *tagTab = new QWidget;
    QVBoxLayout *tagLayout = new QVBoxLayout(tagTab);
    tagLayout->setSpacing(4);
    tagLayout->setContentsMargins(6,6,6,6);

    QGroupBox *grpTag = new QGroupBox("Tag");
    QFormLayout *form = new QFormLayout(grpTag);
    form->setLabelAlignment(Qt::AlignRight);
    form->setSpacing(4);

    m_edTitle = new QLineEdit; form->addRow("Title:", m_edTitle);
    m_edArtist = new QLineEdit; form->addRow("Artist:", m_edArtist);
    m_edAlbum = new QLineEdit; form->addRow("Album:", m_edAlbum);
    m_edAlbumArtist = new QLineEdit; form->addRow("Album Artist:", m_edAlbumArtist);
    m_edComposer = new QLineEdit; form->addRow("Composer:", m_edComposer);
    m_edYear = new QLineEdit; form->addRow("Year:", m_edYear);
    m_edTrack = new QLineEdit; form->addRow("Track:", m_edTrack);

    m_edGenre = new QComboBox;
    m_edGenre->setEditable(true);
    m_edGenre->addItems(GENRES);
    form->addRow("Genre:", m_edGenre);

    m_edComment = new QTextEdit;
    m_edComment->setMaximumHeight(60);
    form->addRow("Comment:", m_edComment);

    tagLayout->addWidget(grpTag);

    QHBoxLayout *btnRow = new QHBoxLayout;
    QPushButton *btnSave = new QPushButton("Save Tag");
    btnSave->setObjectName("btnSaveTag");
    QPushButton *btnClear = new QPushButton("Clear");
    btnClear->setObjectName("btnClearTag");
    btnRow->addWidget(btnSave);
    btnRow->addWidget(btnClear);
    tagLayout->addLayout(btnRow);
    tagLayout->addStretch();

    // --- Tab 2: File Info ---
    QWidget *infoTab = new QWidget;
    QVBoxLayout *infoLayout = new QVBoxLayout(infoTab);
    infoLayout->setSpacing(4);
    infoLayout->setContentsMargins(6,6,6,6);

    QGroupBox *grpInfo = new QGroupBox("File Information");
    QFormLayout *infoForm = new QFormLayout(grpInfo);
    infoForm->setLabelAlignment(Qt::AlignRight);
    infoForm->setSpacing(4);

    m_lblFilePath = new QLabel; m_lblFilePath->setWordWrap(true);
    m_lblFormat = new QLabel;
    m_lblBitrate = new QLabel;
    m_lblSampleRate = new QLabel;
    m_lblChannels = new QLabel;
    m_lblDuration = new QLabel;
    m_lblFileSize = new QLabel;

    infoForm->addRow("File:", m_lblFilePath);
    infoForm->addRow("Format:", m_lblFormat);
    infoForm->addRow("Bitrate:", m_lblBitrate);
    infoForm->addRow("Sample Rate:", m_lblSampleRate);
    infoForm->addRow("Channels:", m_lblChannels);
    infoForm->addRow("Duration:", m_lblDuration);
    infoForm->addRow("Size:", m_lblFileSize);

    infoLayout->addWidget(grpInfo);
    infoLayout->addStretch();

    // --- Tab 3: Rename / Pattern ---
    QWidget *renameTab = new QWidget;
    QVBoxLayout *renameLayout = new QVBoxLayout(renameTab);
    renameLayout->setSpacing(6);
    renameLayout->setContentsMargins(6,6,6,6);

    QGroupBox *grpRename = new QGroupBox("Rename Files");
    QVBoxLayout *rnLayout = new QVBoxLayout(grpRename);
    rnLayout->addWidget(new QLabel("Pattern (use %T title, %A artist, %L album,\n%Y year, %N track, %G genre):"));
    m_edRenamePattern = new QLineEdit("%N - %T");
    rnLayout->addWidget(m_edRenamePattern);
    QPushButton *btnRename = new QPushButton("Rename Selected Files");
    btnRename->setObjectName("btnRenameFiles");
    rnLayout->addWidget(btnRename);

    QGroupBox *grpFromTag = new QGroupBox("Tag → Filename");
    QVBoxLayout *ftLayout = new QVBoxLayout(grpFromTag);
    ftLayout->addWidget(new QLabel("Filename pattern from tag fields:"));
    m_edTagPattern = new QLineEdit("%A - %L - %N - %T");
    ftLayout->addWidget(m_edTagPattern);
    QPushButton *btnFromTag = new QPushButton("Apply Filename from Tag");
    btnFromTag->setObjectName("btnFilenameFromTag");
    ftLayout->addWidget(btnFromTag);

    renameLayout->addWidget(grpRename);
    renameLayout->addWidget(grpFromTag);
    renameLayout->addStretch();

    m_tagTabs->addTab(tagTab, "Tag Editor");
    m_tagTabs->addTab(infoTab, "File Info");
    m_tagTabs->addTab(renameTab, "Rename");

    QVBoxLayout *panelLayout = new QVBoxLayout(m_tagPanel);
    panelLayout->setContentsMargins(0,0,0,0);
    panelLayout->addWidget(m_tagTabs);

    // Connect buttons
    connect(btnSave, &QPushButton::clicked, this, &MainWindow::onSaveTags);
    connect(btnClear, &QPushButton::clicked, this, &MainWindow::onClearTags);
    connect(btnRename, &QPushButton::clicked, this, &MainWindow::onRenameFiles);
    connect(btnFromTag, &QPushButton::clicked, this, &MainWindow::onFilenameFromTag);
}

void MainWindow::setupMenuBar() {
    // File menu
    QMenu *fileMenu = menuBar()->addMenu("&File");
    QAction *aAddFiles = fileMenu->addAction(QIcon::fromTheme("document-open"), "&Add Files...");
    aAddFiles->setShortcut(QKeySequence("Ctrl+O"));
    connect(aAddFiles, &QAction::triggered, this, &MainWindow::onAddFiles);

    QAction *aAddFolder = fileMenu->addAction(QIcon::fromTheme("folder-open"), "Add &Folder...");
    aAddFolder->setShortcut(QKeySequence("Ctrl+Shift+O"));
    connect(aAddFolder, &QAction::triggered, this, &MainWindow::onAddFolder);

    fileMenu->addSeparator();
    QAction *aRemove = fileMenu->addAction("&Remove Selected");
    aRemove->setShortcut(QKeySequence::Delete);
    connect(aRemove, &QAction::triggered, this, &MainWindow::onRemoveSelected);

    QAction *aClear = fileMenu->addAction("Clear &List");
    connect(aClear, &QAction::triggered, this, &MainWindow::onClearList);

    fileMenu->addSeparator();
    QAction *aExit = fileMenu->addAction("E&xit");
    aExit->setShortcut(QKeySequence::Quit);
    connect(aExit, &QAction::triggered, qApp, &QApplication::quit);

    // Edit menu
    QMenu *editMenu = menuBar()->addMenu("&Edit");
    QAction *aSelectAll = editMenu->addAction("Select &All");
    aSelectAll->setShortcut(QKeySequence::SelectAll);
    connect(aSelectAll, &QAction::triggered, this, &MainWindow::onSelectAll);

    QAction *aInvert = editMenu->addAction("&Invert Selection");
    connect(aInvert, &QAction::triggered, this, &MainWindow::onInvertSelection);

    editMenu->addSeparator();
    QAction *aAutoTag = editMenu->addAction("Auto-&Tag from Web");
    connect(aAutoTag, &QAction::triggered, this, &MainWindow::onAutoTag);

    QAction *aTagFromFile = editMenu->addAction("Tag from &Filename");
    connect(aTagFromFile, &QAction::triggered, this, &MainWindow::onTagFromFilename);

    // Tags menu
    QMenu *tagsMenu = menuBar()->addMenu("&Tags");
    QAction *aSaveAll = tagsMenu->addAction(QIcon::fromTheme("document-save"), "&Save All Changes");
    aSaveAll->setShortcut(QKeySequence("Ctrl+S"));
    connect(aSaveAll, &QAction::triggered, this, &MainWindow::onSaveAll);

    QAction *aUndo = tagsMenu->addAction("&Undo Changes");
    connect(aUndo, &QAction::triggered, this, &MainWindow::onUndoChanges);

    tagsMenu->addSeparator();
    QAction *aClearTags = tagsMenu->addAction("&Clear Tags");
    connect(aClearTags, &QAction::triggered, this, &MainWindow::onClearTags);

    // Help
    QMenu *helpMenu = menuBar()->addMenu("&Help");
    QAction *aAbout = helpMenu->addAction("&About tagQ");
    connect(aAbout, &QAction::triggered, this, &MainWindow::onAbout);
}

void MainWindow::setupToolBar() {
    m_mainToolBar = addToolBar("Main");
    m_mainToolBar->setMovable(false);
    m_mainToolBar->setIconSize(QSize(22, 22));

    auto addBtn = [&](const QString &icon, const QString &tip, auto slot) {
        QAction *a = m_mainToolBar->addAction(QIcon::fromTheme(icon), tip);
        connect(a, &QAction::triggered, this, slot);
        return a;
    };

    addBtn("document-open", "Add Files (Ctrl+O)", &MainWindow::onAddFiles);
    addBtn("folder-open", "Add Folder", &MainWindow::onAddFolder);
    m_mainToolBar->addSeparator();
    addBtn("document-save", "Save All Changes (Ctrl+S)", &MainWindow::onSaveAll);
    addBtn("edit-undo", "Undo Changes", &MainWindow::onUndoChanges);
    m_mainToolBar->addSeparator();
    addBtn("list-remove", "Remove Selected", &MainWindow::onRemoveSelected);
    addBtn("edit-clear", "Clear List", &MainWindow::onClearList);
    m_mainToolBar->addSeparator();
    addBtn("edit-select-all", "Select All", &MainWindow::onSelectAll);

    // Search filter
    m_mainToolBar->addSeparator();
    m_mainToolBar->addWidget(new QLabel(" Filter: "));
    QLineEdit *filterEdit = new QLineEdit;
    filterEdit->setPlaceholderText("Filter files...");
    filterEdit->setMaximumWidth(180);
    m_mainToolBar->addWidget(filterEdit);
    connect(filterEdit, &QLineEdit::textChanged, this, &MainWindow::onFilterChanged);
}

void MainWindow::setupStatusBar() {
    m_statusFiles = new QLabel("0 files");
    m_statusSelected = new QLabel("0 selected");
    m_progressBar = new QProgressBar;
    m_progressBar->setVisible(false);
    m_progressBar->setMaximumWidth(200);
    m_progressBar->setMaximumHeight(16);

    statusBar()->addWidget(m_statusFiles);
    statusBar()->addWidget(new QLabel("|"));
    statusBar()->addWidget(m_statusSelected);
    statusBar()->addPermanentWidget(m_progressBar);
}

void MainWindow::connectSignals() {
    connect(m_fileTable, &QTableWidget::itemSelectionChanged, this, &MainWindow::onFileSelectionChanged);
    connect(m_fileTable, &QTableWidget::cellChanged, this, &MainWindow::onCellChanged);
    connect(m_fileTable->horizontalHeader(), &QHeaderView::sectionClicked, this, &MainWindow::onSortBy);

    // Folder tree expand/click
    connect(m_folderTree, &QTreeWidget::itemExpanded, this, [this](QTreeWidgetItem *item) {
        QString path = item->data(0, Qt::UserRole).toString();
        if (path.isEmpty()) return;
        item->takeChildren();
        QDir dir(path);
        for (const auto &entry : dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot)) {
            QTreeWidgetItem *child = new QTreeWidgetItem(item);
            child->setText(0, entry.fileName());
            child->setData(0, Qt::UserRole, entry.filePath());
            new QTreeWidgetItem(child); // dummy
        }
    });
    connect(m_folderTree, &QTreeWidget::itemDoubleClicked, this, [this](QTreeWidgetItem *item) {
        QString path = item->data(0, Qt::UserRole).toString();
        if (!path.isEmpty()) loadFolder(path, false);
    });
}

void MainWindow::onAddFiles() {
    QStringList files = QFileDialog::getOpenFileNames(
        this, "Add Audio Files", QDir::homePath(),
        "Audio Files (*.mp3 *.flac *.ogg *.m4a *.mp4 *.wma *.asf *.aiff *.wav *.ape *.mpc *.tta *.wv);;All Files (*)"
    );
    m_progressBar->setVisible(true);
    m_progressBar->setMaximum(files.size());
    int i = 0;
    for (const QString &f : files) {
        loadFile(f);
        m_progressBar->setValue(++i);
        QApplication::processEvents();
    }
    m_progressBar->setVisible(false);
    updateStatusBar();
}

void MainWindow::onAddFolder() {
    QString dir = QFileDialog::getExistingDirectory(this, "Select Folder", QDir::homePath());
    if (!dir.isEmpty()) {
        QMessageBox::StandardButton r = QMessageBox::question(
            this, "Add Folder", "Include subfolders?",
            QMessageBox::Yes | QMessageBox::No
        );
        loadFolder(dir, r == QMessageBox::Yes);
    }
}

void MainWindow::loadFolder(const QString &path, bool recursive) {
    QStringList filters = {"*.mp3","*.flac","*.ogg","*.m4a","*.mp4","*.wma","*.asf","*.aiff","*.wav","*.ape","*.mpc","*.tta","*.wv"};
    QDirIterator::IteratorFlags flags = recursive
        ? QDirIterator::Subdirectories : QDirIterator::NoIteratorFlags;
    QDirIterator it(path, filters, QDir::Files, flags);
    QStringList files;
    while (it.hasNext()) files << it.next();
    m_progressBar->setVisible(true);
    m_progressBar->setMaximum(files.size());
    int i = 0;
    for (const QString &f : files) {
        loadFile(f);
        m_progressBar->setValue(++i);
        if (i % 10 == 0) QApplication::processEvents();
    }
    m_progressBar->setVisible(false);
    updateStatusBar();
}

void MainWindow::loadFile(const QString &path) {
    // Check not already loaded
    for (const auto &f : m_files)
        if (f.filePath == path) return;

    AudioFileInfo info;
    info.filePath = path;
    info.fileName = QFileInfo(path).fileName();
    info.modified = false;

    TagLib::FileRef ref(path.toLocal8Bit().constData());
    if (!ref.isNull()) {
        info.hasTag = true;
        if (ref.tag()) {
            auto *tag = ref.tag();
            info.title = tstr(tag->title());
            info.artist = tstr(tag->artist());
            info.album = tstr(tag->album());
            info.year = tag->year() > 0 ? QString::number(tag->year()) : "";
            info.track = tag->track() > 0 ? QString::number(tag->track()) : "";
            info.genre = tstr(tag->genre());
            info.comment = tstr(tag->comment());
        }
        if (ref.audioProperties()) {
            auto *ap = ref.audioProperties();
            info.bitrate = ap->bitrate();
            info.sampleRate = ap->sampleRate();
            info.channels = ap->channels();
            info.duration = ap->lengthInSeconds();
        }
    } else {
        info.hasTag = false;
    }
    info.fileSize = QFileInfo(path).size();

    int row = m_fileTable->rowCount();
    m_fileTable->setRowCount(row + 1);
    m_files.append(info);
    populateRow(row, info);
}

void MainWindow::populateRow(int row, const AudioFileInfo &info) {
    m_updating = true;
    auto setItem = [&](int col, const QString &text, bool editable = true) {
        QTableWidgetItem *item = new QTableWidgetItem(text);
        if (!editable) item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        m_fileTable->setItem(row, col, item);
    };
    setItem(COL_FILENAME, info.fileName, false);
    setItem(COL_TITLE, info.title);
    setItem(COL_ARTIST, info.artist);
    setItem(COL_ALBUM, info.album);
    setItem(COL_YEAR, info.year);
    setItem(COL_TRACK, info.track);
    setItem(COL_GENRE, info.genre);
    setItem(COL_DURATION, formatDuration(info.duration), false);
    setItem(COL_BITRATE, info.bitrate > 0 ? QString("%1 kbps").arg(info.bitrate) : "", false);
    setItem(COL_SIZE, formatSize(info.fileSize), false);
    setItem(COL_PATH, info.filePath, false);

    if (info.modified) markRowModified(row, true);
    m_updating = false;
}

void MainWindow::markRowModified(int row, bool modified) {
    for (int col = 0; col < COL_COUNT; ++col) {
        QTableWidgetItem *item = m_fileTable->item(row, col);
        if (item) {
            item->setBackground(modified ? QColor(255, 255, 200) : QApplication::palette().base());
        }
    }
}

void MainWindow::onCellChanged(int row, int col) {
    if (m_updating) return;
    if (row < 0 || row >= m_files.size()) return;
    QTableWidgetItem *item = m_fileTable->item(row, col);
    if (!item) return;
    QString val = item->text();
    auto &info = m_files[row];
    switch(col) {
        case COL_TITLE: info.title = val; break;
        case COL_ARTIST: info.artist = val; break;
        case COL_ALBUM: info.album = val; break;
        case COL_YEAR: info.year = val; break;
        case COL_TRACK: info.track = val; break;
        case COL_GENRE: info.genre = val; break;
        default: return;
    }
    info.modified = true;
    markRowModified(row, true);
}

void MainWindow::onFileSelectionChanged() {
    int count = m_fileTable->selectedItems().isEmpty() ? 0 :
        m_fileTable->selectionModel()->selectedRows().size();
    m_statusSelected->setText(QString("%1 selected").arg(count));

    auto rows = m_fileTable->selectionModel()->selectedRows();
    if (rows.isEmpty()) return;
    int row = rows.first().row();
    updateEditorFromRow(row);
}

void MainWindow::updateEditorFromRow(int row) {
    if (row < 0 || row >= m_files.size()) return;
    const auto &info = m_files[row];
    m_updating = true;
    m_edTitle->setText(info.title);
    m_edArtist->setText(info.artist);
    m_edAlbum->setText(info.album);
    m_edYear->setText(info.year);
    m_edTrack->setText(info.track);
    m_edAlbumArtist->setText(info.albumArtist);
    m_edComposer->setText(info.composer);
    int gIdx = m_edGenre->findText(info.genre);
    if (gIdx >= 0) m_edGenre->setCurrentIndex(gIdx);
    else { m_edGenre->setCurrentIndex(0); m_edGenre->setCurrentText(info.genre); }
    m_edComment->setPlainText(info.comment);

    // File info tab
    m_lblFilePath->setText(info.filePath);
    m_lblFormat->setText(QFileInfo(info.filePath).suffix().toUpper());
    m_lblBitrate->setText(info.bitrate > 0 ? QString("%1 kbps").arg(info.bitrate) : "N/A");
    m_lblSampleRate->setText(info.sampleRate > 0 ? QString("%1 Hz").arg(info.sampleRate) : "N/A");
    m_lblChannels->setText(info.channels > 0 ? (info.channels == 1 ? "Mono" : "Stereo") : "N/A");
    m_lblDuration->setText(formatDuration(info.duration));
    m_lblFileSize->setText(formatSize(info.fileSize));
    m_updating = false;
}

void MainWindow::applyTagFromEditor(int row) {
    if (row < 0 || row >= m_files.size()) return;
    auto &info = m_files[row];
    info.title = m_edTitle->text();
    info.artist = m_edArtist->text();
    info.album = m_edAlbum->text();
    info.year = m_edYear->text();
    info.track = m_edTrack->text();
    info.genre = m_edGenre->currentText();
    info.comment = m_edComment->toPlainText();
    info.albumArtist = m_edAlbumArtist->text();
    info.composer = m_edComposer->text();
    info.modified = true;
    populateRow(row, info);
    markRowModified(row, true);
}

void MainWindow::onSaveTags() {
    auto rows = m_fileTable->selectionModel()->selectedRows();
    for (const auto &idx : rows)
        applyTagFromEditor(idx.row());
    // Then save to file
    for (const auto &idx : rows)
        saveFileTag(idx.row());
    updateStatusBar();
}

bool MainWindow::saveFileTag(int row) {
    if (row < 0 || row >= m_files.size()) return false;
    auto &info = m_files[row];
    TagLib::FileRef ref(info.filePath.toLocal8Bit().constData());
    if (ref.isNull() || !ref.tag()) return false;
    auto *tag = ref.tag();
    tag->setTitle(qstr(info.title));
    tag->setArtist(qstr(info.artist));
    tag->setAlbum(qstr(info.album));
    tag->setYear(info.year.toUInt());
    tag->setTrack(info.track.toUInt());
    tag->setGenre(qstr(info.genre));
    tag->setComment(qstr(info.comment));
    if (ref.save()) {
        info.modified = false;
        markRowModified(row, false);
        return true;
    }
    return false;
}

void MainWindow::onSaveAll() {
    int saved = 0, failed = 0;
    m_progressBar->setVisible(true);
    m_progressBar->setMaximum(m_files.size());
    for (int i = 0; i < m_files.size(); ++i) {
        if (m_files[i].modified) {
            if (saveFileTag(i)) saved++; else failed++;
        }
        m_progressBar->setValue(i + 1);
    }
    m_progressBar->setVisible(false);
    statusBar()->showMessage(QString("Saved %1 files. %2 failed.").arg(saved).arg(failed), 3000);
    updateStatusBar();
}

void MainWindow::onUndoChanges() {
    auto rows = m_fileTable->selectionModel()->selectedRows();
    for (const auto &idx : rows) {
        int row = idx.row();
        if (row >= m_files.size()) continue;
        auto &info = m_files[row];
        // Reload from disk
        TagLib::FileRef ref(info.filePath.toLocal8Bit().constData());
        if (!ref.isNull() && ref.tag()) {
            auto *tag = ref.tag();
            info.title = tstr(tag->title());
            info.artist = tstr(tag->artist());
            info.album = tstr(tag->album());
            info.year = tag->year() > 0 ? QString::number(tag->year()) : "";
            info.track = tag->track() > 0 ? QString::number(tag->track()) : "";
            info.genre = tstr(tag->genre());
            info.comment = tstr(tag->comment());
        }
        info.modified = false;
        populateRow(row, info);
    }
    if (!rows.isEmpty()) updateEditorFromRow(rows.first().row());
}

void MainWindow::onRemoveSelected() {
    auto rows = m_fileTable->selectionModel()->selectedRows();
    // Sort descending
    QList<int> rowNums;
    for (const auto &idx : rows) rowNums << idx.row();
    std::sort(rowNums.rbegin(), rowNums.rend());
    for (int r : rowNums) {
        m_fileTable->removeRow(r);
        m_files.removeAt(r);
    }
    updateStatusBar();
}

void MainWindow::onClearList() {
    m_fileTable->setRowCount(0);
    m_files.clear();
    updateStatusBar();
}

void MainWindow::onSelectAll() {
    m_fileTable->selectAll();
}

void MainWindow::onInvertSelection() {
    for (int i = 0; i < m_fileTable->rowCount(); ++i) {
        bool selected = m_fileTable->selectionModel()->isRowSelected(i, {});
        if (selected) m_fileTable->selectionModel()->select(
            m_fileTable->model()->index(i, 0),
            QItemSelectionModel::Rows | QItemSelectionModel::Deselect);
        else m_fileTable->selectionModel()->select(
            m_fileTable->model()->index(i, 0),
            QItemSelectionModel::Rows | QItemSelectionModel::Select);
    }
}

void MainWindow::onClearTags() {
    auto rows = m_fileTable->selectionModel()->selectedRows();
    for (const auto &idx : rows) {
        int row = idx.row();
        if (row >= m_files.size()) continue;
        auto &info = m_files[row];
        info.title = info.artist = info.album = info.year = info.track = info.genre = info.comment = "";
        info.modified = true;
        populateRow(row, info);
    }
    auto selRows = m_fileTable->selectionModel()->selectedRows();
    if (!selRows.isEmpty()) updateEditorFromRow(selRows.first().row());
}

void MainWindow::onRenameFiles() {
    auto rows = m_fileTable->selectionModel()->selectedRows();
    if (rows.isEmpty()) { statusBar()->showMessage("No files selected.", 2000); return; }
    QString pattern = m_edRenamePattern->text();
    int renamed = 0;
    for (const auto &idx : rows) {
        int row = idx.row();
        if (row >= m_files.size()) continue;
        auto &info = m_files[row];
        QString newName = pattern;
        newName.replace("%T", info.title.isEmpty() ? "Unknown" : info.title);
        newName.replace("%A", info.artist.isEmpty() ? "Unknown" : info.artist);
        newName.replace("%L", info.album.isEmpty() ? "Unknown" : info.album);
        newName.replace("%Y", info.year.isEmpty() ? "0000" : info.year);
        newName.replace("%N", info.track.isEmpty() ? "00" : info.track.rightJustified(2,'0'));
        newName.replace("%G", info.genre.isEmpty() ? "Unknown" : info.genre);
        QString ext = QFileInfo(info.filePath).suffix();
        QFileInfo fi(info.filePath);
        QString newPath = fi.dir().filePath(newName + "." + ext);
        if (QFile::rename(info.filePath, newPath)) {
            info.filePath = newPath;
            info.fileName = QFileInfo(newPath).fileName();
            m_updating = true;
            m_fileTable->item(row, COL_FILENAME)->setText(info.fileName);
            m_fileTable->item(row, COL_PATH)->setText(info.filePath);
            m_updating = false;
            renamed++;
        }
    }
    statusBar()->showMessage(QString("Renamed %1 files.").arg(renamed), 3000);
}

void MainWindow::onTagFromFilename() {
    auto rows = m_fileTable->selectionModel()->selectedRows();
    for (const auto &idx : rows) {
        int row = idx.row();
        if (row >= m_files.size()) continue;
        auto &info = m_files[row];
        QString base = QFileInfo(info.filePath).completeBaseName();
        // Simple: "02 - Title" or "Artist - Album - 02 - Title"
        QStringList parts = base.split(" - ");
        if (parts.size() >= 2) {
            bool ok;
            parts[0].trimmed().toInt(&ok);
            if (ok && parts.size() >= 2) {
                info.track = parts[0].trimmed();
                info.title = parts[1].trimmed();
            } else if (parts.size() >= 4) {
                info.artist = parts[0].trimmed();
                info.album = parts[1].trimmed();
                info.track = parts[2].trimmed();
                info.title = parts[3].trimmed();
            } else {
                info.artist = parts[0].trimmed();
                info.title = parts[1].trimmed();
            }
            info.modified = true;
            populateRow(row, info);
        }
    }
}

void MainWindow::onFilenameFromTag() {
    onRenameFiles();
}

void MainWindow::onAutoTag() {
    QMessageBox::information(this, "Auto-Tag",
        "Auto-tagging from online databases (MusicBrainz/Discogs)\nwould require network access. This feature stub shows\nwhere integration would occur.");
}

void MainWindow::onFilterChanged(const QString &text) {
    for (int i = 0; i < m_fileTable->rowCount(); ++i) {
        bool visible = text.isEmpty();
        if (!visible) {
            for (int col = 0; col < COL_COUNT - 1; ++col) {
                QTableWidgetItem *item = m_fileTable->item(i, col);
                if (item && item->text().contains(text, Qt::CaseInsensitive)) {
                    visible = true;
                    break;
                }
            }
        }
        m_fileTable->setRowHidden(i, !visible);
    }
}

void MainWindow::onSortBy(int col) {
    m_fileTable->sortByColumn(col, Qt::AscendingOrder);
}

void MainWindow::onColumnToggled(QAction *) {}

void MainWindow::onAbout() {
    QMessageBox::about(this, "About tagQ",
        "<b>tagQ 0.1</b><br>"
        "Qt6-based audio tag editor<br><br>"
        "Supported formats: MP3, FLAC, OGG, M4A, WMA, AIFF, WAV, APE<br>"
        "Powered by TagLib 1.13 and Qt 6.4<br><br>"
        "Open-source implementation inspired by tagQ.");
}

void MainWindow::updateStatusBar() {
    m_statusFiles->setText(QString("%1 files").arg(m_files.size()));
    int sel = m_fileTable->selectionModel()->selectedRows().size();
    m_statusSelected->setText(QString("%1 selected").arg(sel));
}

QString MainWindow::formatDuration(int secs) {
    if (secs <= 0) return "";
    return QString("%1:%2").arg(secs/60).arg(secs%60, 2, 10, QChar('0'));
}

QString MainWindow::formatSize(qint64 bytes) {
    if (bytes <= 0) return "";
    if (bytes < 1024*1024) return QString("%1 KB").arg(bytes/1024);
    return QString("%1 MB").arg(bytes/(1024*1024));
}

// Drag & drop
void MainWindow::dragEnterEvent(QDragEnterEvent *e) {
    if (e->mimeData()->hasUrls()) e->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *e) {
    for (const QUrl &url : e->mimeData()->urls()) {
        QString path = url.toLocalFile();
        QFileInfo fi(path);
        if (fi.isDir()) loadFolder(path, false);
        else loadFile(path);
    }
    updateStatusBar();
}
