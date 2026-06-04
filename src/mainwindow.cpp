#include "mainwindow.h"
#include "filebrowser.h"
#include "filelist.h"
#include "metadataeditor.h"

#include <QApplication>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QSplitter>
#include <QFileDialog>
#include <QMessageBox>
#include <QCloseEvent>
#include <QSettings>
#include <QLabel>
#include <QLineEdit>
#include <QDir>
#include <QDockWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_fileBrowser(std::make_unique<FileBrowser>()),
      m_fileList(std::make_unique<FileList>()),
      m_metadataEditor(std::make_unique<MetadataEditor>())
{
    setWindowTitle("tagQ 0.1");
    setMinimumSize(1100, 680);
    resize(1280, 780);
    setAcceptDrops(true);

    setupUI();
    createMenuBar();
    createToolBar();
    createConnections();
    loadSettings();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUI()
{
    // Three-pane layout: [FileBrowser | FileList | MetadataEditor]
    auto *splitter = new QSplitter(Qt::Horizontal, this);

    m_fileBrowser->setMinimumWidth(170);
    m_fileBrowser->setMaximumWidth(240);
    splitter->addWidget(m_fileBrowser.get());

    splitter->addWidget(m_fileList.get());

    m_metadataEditor->setMinimumWidth(290);
    m_metadataEditor->setMaximumWidth(380);
    splitter->addWidget(m_metadataEditor.get());

    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 3);
    splitter->setStretchFactor(2, 1);
    splitter->setSizes({200, 860, 320});

    setCentralWidget(splitter);

    // Status bar
    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::createMenuBar()
{
    // ── File ──────────────────────────────────────
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));

    QAction *aAddFiles = fileMenu->addAction(
        QIcon::fromTheme("document-open"), tr("&Add Files…"));
    aAddFiles->setShortcut(QKeySequence("Ctrl+O"));
    connect(aAddFiles, &QAction::triggered, this, [this] {
        const QStringList files = QFileDialog::getOpenFileNames(
            this, tr("Add Audio Files"), QDir::homePath(),
            tr("Audio Files (*.mp3 *.flac *.ogg *.m4a *.mp4 *.wma *.asf "
               "*.aiff *.wav *.ape *.mpc *.tta *.wv *.opus);;All Files (*)"));
        for (const QString &f : files)
            m_fileList->loadFiles(QFileInfo(f).absolutePath());
    });

    QAction *aAddFolder = fileMenu->addAction(
        QIcon::fromTheme("folder-open"), tr("Add &Folder…"));
    aAddFolder->setShortcut(QKeySequence("Ctrl+Shift+O"));
    connect(aAddFolder, &QAction::triggered, this, [this] {
        const QString dir = QFileDialog::getExistingDirectory(
            this, tr("Select Folder"), QDir::homePath());
        if (!dir.isEmpty())
            m_fileList->loadFiles(dir);
    });

    fileMenu->addSeparator();

    QAction *aExit = fileMenu->addAction(tr("E&xit"));
    aExit->setShortcut(QKeySequence::Quit);
    connect(aExit, &QAction::triggered, this, &MainWindow::close);

    // ── Edit ──────────────────────────────────────
    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));

    QAction *aSelAll = editMenu->addAction(tr("Select &All"));
    aSelAll->setShortcut(QKeySequence::SelectAll);
    connect(aSelAll, &QAction::triggered, m_fileList.get(), &FileList::selectAll);

    QAction *aDesel = editMenu->addAction(tr("&Deselect All"));
    connect(aDesel, &QAction::triggered, m_fileList.get(), &FileList::deselectAll);

    editMenu->addSeparator();

    QAction *aAutoTag = editMenu->addAction(tr("Auto-Tag from &Filename"));
    connect(aAutoTag, &QAction::triggered, m_fileList.get(), &FileList::autoTagFromFilename);

    QAction *aBatchRename = editMenu->addAction(tr("&Batch Rename…"));
    connect(aBatchRename, &QAction::triggered, m_fileList.get(), &FileList::batchRename);

    // ── Tags ──────────────────────────────────────
    QMenu *tagsMenu = menuBar()->addMenu(tr("&Tags"));

    QAction *aSaveAll = tagsMenu->addAction(
        QIcon::fromTheme("document-save"), tr("&Save All Tags"));
    aSaveAll->setShortcut(QKeySequence("Ctrl+S"));
    connect(aSaveAll, &QAction::triggered, m_fileList.get(), &FileList::saveTags);

    QAction *aRefresh = tagsMenu->addAction(
        QIcon::fromTheme("view-refresh"), tr("&Refresh"));
    aRefresh->setShortcut(QKeySequence::Refresh);
    connect(aRefresh, &QAction::triggered, m_fileList.get(), &FileList::refresh);

    // ── Help ──────────────────────────────────────
    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));

    QAction *aAbout = helpMenu->addAction(tr("&About tagQ"));
    connect(aAbout, &QAction::triggered, this, [this] {
        QMessageBox::about(this, tr("About tagQ"),
            "<b>tagQ 0.1</b><br>"
            "Qt6-based audio tag editor<br><br>"
            "Supported formats: MP3, FLAC, OGG, M4A, WMA, AIFF, WAV, APE, Opus<br>"
            "Powered by TagLib 1.13 and Qt 6<br><br>"
            "Copyright 2026 asmerovski");
    });
}

void MainWindow::createToolBar()
{
    QToolBar *tb = addToolBar(tr("Main"));
    tb->setMovable(false);
    tb->setIconSize(QSize(22, 22));

    tb->addAction(QIcon::fromTheme("document-open"),   tr("Add Files"),
                  this, [this] {
        const QStringList files = QFileDialog::getOpenFileNames(
            this, tr("Add Audio Files"), QDir::homePath(),
            tr("Audio Files (*.mp3 *.flac *.ogg *.m4a *.mp4 *.wma *.asf "
               "*.aiff *.wav *.ape *.mpc *.tta *.wv *.opus);;All Files (*)"));
        for (const QString &f : files)
            m_fileList->loadFiles(QFileInfo(f).absolutePath());
    });

    tb->addAction(QIcon::fromTheme("folder-open"), tr("Add Folder"),
                  this, [this] {
        const QString dir = QFileDialog::getExistingDirectory(
            this, tr("Select Folder"), QDir::homePath());
        if (!dir.isEmpty())
            m_fileList->loadFiles(dir);
    });

    tb->addSeparator();

    tb->addAction(QIcon::fromTheme("document-save"), tr("Save All (Ctrl+S)"),
                  m_fileList.get(), &FileList::saveTags);

    tb->addAction(QIcon::fromTheme("view-refresh"), tr("Refresh"),
                  m_fileList.get(), &FileList::refresh);

    tb->addSeparator();

    tb->addAction(QIcon::fromTheme("edit-select-all"), tr("Select All"),
                  m_fileList.get(), &FileList::selectAll);

    // Live filter field
    tb->addSeparator();
    tb->addWidget(new QLabel(tr(" Filter: ")));
    auto *filterEdit = new QLineEdit;
    filterEdit->setPlaceholderText(tr("Filter files…"));
    filterEdit->setMaximumWidth(180);
    filterEdit->setClearButtonEnabled(true);
    tb->addWidget(filterEdit);
    // FileList doesn't expose a filter slot yet — wire it up when needed
    Q_UNUSED(filterEdit);
}

void MainWindow::createConnections()
{
    // Folder browser → load files
    connect(m_fileBrowser.get(), &FileBrowser::folderSelected,
            m_fileList.get(),    &FileList::loadFiles);

    // File list selection → metadata editor
    connect(m_fileList.get(),      &FileList::fileSelected,
            m_metadataEditor.get(), &MetadataEditor::displayFile);

    // Metadata editor apply → update file list cell values
    connect(m_metadataEditor.get(), &MetadataEditor::metadataChanged,
            m_fileList.get(),       &FileList::updateFileMetadata);
}

void MainWindow::loadSettings()
{
    QSettings s("tagQ", "tagQ");
    restoreGeometry(s.value("geometry").toByteArray());
    restoreState(s.value("windowState").toByteArray());
}

void MainWindow::saveSettings()
{
    QSettings s("tagQ", "tagQ");
    s.setValue("geometry",    saveGeometry());
    s.setValue("windowState", saveState());
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    saveSettings();
    event->accept();
}
