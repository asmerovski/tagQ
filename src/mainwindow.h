#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>

class FileBrowser;
class FileList;
class MetadataEditor;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void setupUI();
    void createMenuBar();
    void createToolBar();
    void createConnections();
    void loadSettings();
    void saveSettings();

    std::unique_ptr<FileBrowser> m_fileBrowser;
    std::unique_ptr<FileList> m_fileList;
    std::unique_ptr<MetadataEditor> m_metadataEditor;
};

#endif // MAINWINDOW_H