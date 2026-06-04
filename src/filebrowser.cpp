#include "filebrowser.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolButton>
#include <QFileDialog>
#include <QDir>
#include <QMenu>
#include <QAction>
#include <QHeaderView>
#include <QFileInfo>

FileBrowser::FileBrowser(QWidget *parent)
    : QWidget(parent),
      m_treeWidget(new QTreeWidget(this)),
      m_fileModel(std::make_unique<QFileSystemModel>())
{
    setupUI();
}

FileBrowser::~FileBrowser() = default;

void FileBrowser::setupUI()
{
    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(2);

    // Toolbar row
    auto toolRow = new QHBoxLayout;
    auto btnBrowse = new QToolButton;
    btnBrowse->setText("…");
    btnBrowse->setToolTip("Browse for a folder");
    toolRow->addWidget(btnBrowse);
    toolRow->addStretch();
    layout->addLayout(toolRow);

    // Tree
    m_treeWidget->setHeaderLabel("Folders");
    m_treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    m_treeWidget->header()->setStretchLastSection(true);
    layout->addWidget(m_treeWidget);

    // Populate with home + filesystem roots
    auto addRoot = [&](const QString &path, const QString &label) {
        auto *item = new QTreeWidgetItem(m_treeWidget);
        item->setText(0, label);
        item->setData(0, Qt::UserRole, path);
        item->setIcon(0, QIcon::fromTheme("folder"));
        // Dummy child so the expand arrow appears
        new QTreeWidgetItem(item);
    };

    addRoot(QDir::homePath(), QDir::homePath());
    addRoot("/", "/  (root)");

    connect(btnBrowse, &QToolButton::clicked, this, &FileBrowser::browseFolders);

    connect(m_treeWidget, &QTreeWidget::itemExpanded, this, [this](QTreeWidgetItem *item) {
        const QString path = item->data(0, Qt::UserRole).toString();
        if (path.isEmpty()) return;

        item->takeChildren();
        QDir dir(path);
        const auto entries = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
        for (const QFileInfo &fi : entries) {
            auto *child = new QTreeWidgetItem(item);
            child->setText(0, fi.fileName());
            child->setData(0, Qt::UserRole, fi.filePath());
            child->setIcon(0, QIcon::fromTheme("folder"));
            // Peek whether the folder has subdirectories
            QDir sub(fi.filePath());
            if (!sub.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot).isEmpty())
                new QTreeWidgetItem(child); // dummy
        }
    });

    connect(m_treeWidget, &QTreeWidget::itemClicked,
            this, &FileBrowser::onFolderClicked);
    connect(m_treeWidget, &QTreeWidget::itemDoubleClicked,
            this, &FileBrowser::onFolderDoubleClicked);
    connect(m_treeWidget, &QTreeWidget::customContextMenuRequested,
            this, &FileBrowser::onContextMenuRequested);
}

void FileBrowser::browseFolders()
{
    const QString dir = QFileDialog::getExistingDirectory(
        this, tr("Select Folder"), QDir::homePath());
    if (dir.isEmpty()) return;

    // Add as a new top-level item and select it
    auto *item = new QTreeWidgetItem(m_treeWidget);
    item->setText(0, QFileInfo(dir).fileName().isEmpty() ? dir : QFileInfo(dir).fileName());
    item->setData(0, Qt::UserRole, dir);
    item->setIcon(0, QIcon::fromTheme("folder"));
    new QTreeWidgetItem(item); // dummy

    m_treeWidget->setCurrentItem(item);
    emit folderSelected(dir);
}

void FileBrowser::onFolderClicked(QTreeWidgetItem *item, int /*column*/)
{
    const QString path = item->data(0, Qt::UserRole).toString();
    if (!path.isEmpty())
        emit folderSelected(path);
}

void FileBrowser::onFolderDoubleClicked(QTreeWidgetItem *item, int /*column*/)
{
    const QString path = item->data(0, Qt::UserRole).toString();
    if (!path.isEmpty())
        emit folderSelected(path);
}

void FileBrowser::onContextMenuRequested(const QPoint &pos)
{
    QTreeWidgetItem *item = m_treeWidget->itemAt(pos);
    QMenu menu;

    auto *openAction = menu.addAction(QIcon::fromTheme("folder-open"), tr("Load Folder"));
    connect(openAction, &QAction::triggered, this, [this, item]() {
        if (item) {
            const QString path = item->data(0, Qt::UserRole).toString();
            if (!path.isEmpty()) emit folderSelected(path);
        }
    });

    auto *browseAction = menu.addAction(QIcon::fromTheme("folder"), tr("Browse…"));
    connect(browseAction, &QAction::triggered, this, &FileBrowser::browseFolders);

    menu.exec(m_treeWidget->viewport()->mapToGlobal(pos));
}

void FileBrowser::createContextMenu()
{
    // placeholder — menu is built inline in onContextMenuRequested
}
