#include "tagsmanager.h"
#include "audiofile.h"

#include <QFileInfo>
#include <QDir>
#include <QFile>
#include <QRegularExpression>

TagsManager::TagsManager() = default;
TagsManager::~TagsManager() = default;

bool TagsManager::saveTags(const QList<std::shared_ptr<AudioFile>> &files)
{
    bool allOk = true;
    for (auto &f : files) {
        if (f->isModified())
            allOk &= f->saveMetadata();
    }
    return allOk;
}

bool TagsManager::revertTags(const QList<std::shared_ptr<AudioFile>> &files)
{
    bool allOk = true;
    for (auto &f : files)
        allOk &= f->loadMetadata();
    return allOk;
}

bool TagsManager::autoTagFromFilename(const QString &filePath, const QString &pattern)
{
    Q_UNUSED(pattern);
    QFileInfo fi(filePath);
    const QString base = fi.completeBaseName();
    const QStringList parts = base.split(" - ");

    AudioFile af(filePath);
    if (parts.size() >= 4) {
        af.setArtist(parts[0].trimmed());
        af.setAlbum(parts[1].trimmed());
        af.setTrackNumber(parts[2].trimmed());
        af.setTitle(parts[3].trimmed());
    } else if (parts.size() == 3) {
        af.setArtist(parts[0].trimmed());
        af.setAlbum(parts[1].trimmed());
        af.setTitle(parts[2].trimmed());
    } else if (parts.size() == 2) {
        bool isNum;
        parts[0].trimmed().toInt(&isNum);
        if (isNum) {
            af.setTrackNumber(parts[0].trimmed());
            af.setTitle(parts[1].trimmed());
        } else {
            af.setArtist(parts[0].trimmed());
            af.setTitle(parts[1].trimmed());
        }
    } else {
        af.setTitle(base);
    }
    return af.saveMetadata();
}

bool TagsManager::autoTagFromTemplate(const QList<std::shared_ptr<AudioFile>> &files,
                                       const QString &tmpl)
{
    bool allOk = true;
    for (auto &f : files)
        allOk &= autoTagFromFilename(f->filePath(), tmpl);
    return allOk;
}

QString TagsManager::parseTemplate(const QString &tmpl,
                                    const QMap<QString, QString> &tags) const
{
    QString result = tmpl;
    result.replace("{title}",   tags.value("TITLE",       "Unknown"));
    result.replace("{artist}",  tags.value("ARTIST",      "Unknown"));
    result.replace("{album}",   tags.value("ALBUM",       "Unknown"));
    result.replace("{year}",    tags.value("DATE",        "0000"));
    result.replace("{track}",   tags.value("TRACKNUMBER", "00").rightJustified(2, '0'));
    result.replace("{genre}",   tags.value("GENRE",       "Unknown"));
    // Also support %X style tokens
    result.replace("%T", tags.value("TITLE",       "Unknown"));
    result.replace("%A", tags.value("ARTIST",      "Unknown"));
    result.replace("%L", tags.value("ALBUM",       "Unknown"));
    result.replace("%Y", tags.value("DATE",        "0000"));
    result.replace("%N", tags.value("TRACKNUMBER", "00").rightJustified(2, '0'));
    result.replace("%G", tags.value("GENRE",       "Unknown"));
    return result;
}

bool TagsManager::renameFile(const std::shared_ptr<AudioFile> &file,
                              const QString &pattern)
{
    const QString newBase = parseTemplate(pattern, file->tags());
    const QFileInfo fi(file->filePath());
    const QString newPath = fi.dir().filePath(newBase + "." + fi.suffix());
    return QFile::rename(file->filePath(), newPath);
}

bool TagsManager::batchRename(const QList<std::shared_ptr<AudioFile>> &files,
                               const QString &pattern)
{
    bool allOk = true;
    for (auto &f : files)
        allOk &= renameFile(f, pattern);
    return allOk;
}

QStringList TagsManager::getSupportedAudioFormats() const
{
    return {"*.mp3", "*.flac", "*.ogg", "*.m4a", "*.mp4",
            "*.wma", "*.asf", "*.aiff", "*.aif", "*.wav",
            "*.ape", "*.mpc", "*.tta", "*.wv", "*.opus"};
}

QString TagsManager::formatDuration(int seconds) const
{
    if (seconds <= 0) return "0:00";
    const int h = seconds / 3600;
    const int m = (seconds % 3600) / 60;
    const int s = seconds % 60;
    if (h > 0)
        return QString("%1:%2:%3").arg(h)
               .arg(m, 2, 10, QChar('0'))
               .arg(s, 2, 10, QChar('0'));
    return QString("%1:%2").arg(m).arg(s, 2, 10, QChar('0'));
}
