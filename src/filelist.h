#ifndef FILELIST_H
#define FILELIST_H

#include <QWidget>
#include <QTableWidget>
#include <QString>
#include <QMap>
#include <memory>

class AudioFile;

class FileList : public QWidget
{
    Q_OBJECT

public:
    explicit FileList(QWidget *parent = nullptr);
    ~FileList();

public slots:
    void loadFiles(const QString &folderPath);
    void selectAll();
    void deselectAll();
    void batchRename();
    void autoTagFromFilename();
    void saveTags();
    void refresh();
    void updateFileMetadata(const QString &filePath, const QMap<QString, QString> &tags);

signals:
    void fileSelected(const QString &filePath);

private slots:
    void onCellClicked(int row, int column);
    void onCellChanged(int row, int column);
    void onContextMenuRequested(const QPoint &pos);

private:
    void setupUI();
    void createContextMenu();
    void loadAudioFiles(const QString &folderPath, bool recursive = true);
    void setupColumns();
    QString formatDuration(int seconds) const;

    QTableWidget *m_tableWidget;
    QMap<int, QString> m_rowToFilePath;
    QMap<QString, std::shared_ptr<AudioFile>> m_audioFiles;
    QString m_currentFolder;
    bool m_skipCellChangedSignal;
};

#endif // FILELIST_H