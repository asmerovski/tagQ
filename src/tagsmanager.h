#ifndef TAGSMANAGER_H
#define TAGSMANAGER_H

#include <QString>
#include <QMap>
#include <QList>

class AudioFile;

class TagsManager
{
public:
    TagsManager();
    ~TagsManager();

    // Batch operations
    bool saveTags(const QList<std::shared_ptr<AudioFile>> &files);
    bool revertTags(const QList<std::shared_ptr<AudioFile>> &files);

    // Auto-tagging
    bool autoTagFromFilename(const QString &filePath, const QString &pattern = "{artist} - {title}");
    bool autoTagFromTemplate(const QList<std::shared_ptr<AudioFile>> &files, const QString &template_);

    // File renaming
    bool renameFile(const std::shared_ptr<AudioFile> &file, const QString &pattern);
    bool batchRename(const QList<std::shared_ptr<AudioFile>> &files, const QString &pattern);

    // Utility
    QString parseTemplate(const QString &template_, const QMap<QString, QString> &tags) const;
    QStringList getSupportedAudioFormats() const;

private:
    QString formatDuration(int seconds) const;
};

#endif // TAGSMANAGER_H