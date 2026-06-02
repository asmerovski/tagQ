#ifndef FILEBROWSER_H
#define FILEBROWSER_H

#include <QWidget>
#include <QTreeWidget>
#include <QFileSystemModel>
#include <memory>

class FileBrowser : public QWidget
{
    Q_OBJECT

public:
    explicit FileBrowser(QWidget *parent = nullptr);
    ~FileBrowser();

public slots:
    void browseFolders();

signals:
    void folderSelected(const QString &folderPath);

private slots:
    void onFolderDoubleClicked(QTreeWidgetItem *item, int column);
    void onFolderClicked(QTreeWidgetItem *item, int column);
    void onContextMenuRequested(const QPoint &pos);

private:
    void setupUI();
    void createContextMenu();

    QTreeWidget *m_treeWidget;
    std::unique_ptr<QFileSystemModel> m_fileModel;
};

#endif // FILEBROWSER_H