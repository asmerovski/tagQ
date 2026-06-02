#ifndef AUDIOFILE_H
#define AUDIOFILE_H

#include <QString>
#include <QMap>

class AudioFile
{
public:
    explicit AudioFile(const QString &filePath);
    ~AudioFile();

    // File information
    QString filePath() const { return m_filePath; }
    QString fileName() const { return m_fileName; }
    QString fileFormat() const { return m_fileFormat; }
    qint64 fileSize() const { return m_fileSize; }

    // Audio properties
    int bitrate() const { return m_bitrate; }
    int sampleRate() const { return m_sampleRate; }
    int channels() const { return m_channels; }
    int duration() const { return m_duration; } // in seconds

    // Metadata getters
    QString title() const { return m_tags.value("TITLE"); }
    QString artist() const { return m_tags.value("ARTIST"); }
    QString album() const { return m_tags.value("ALBUM"); }
    QString genre() const { return m_tags.value("GENRE"); }
    QString year() const { return m_tags.value("DATE"); }
    QString trackNumber() const { return m_tags.value("TRACKNUMBER"); }
    QString albumArtist() const { return m_tags.value("ALBUMARTIST"); }
    QString comment() const { return m_tags.value("COMMENT"); }

    // Metadata setters
    void setTitle(const QString &title) { m_tags["TITLE"] = title; m_modified = true; }
    void setArtist(const QString &artist) { m_tags["ARTIST"] = artist; m_modified = true; }
    void setAlbum(const QString &album) { m_tags["ALBUM"] = album; m_modified = true; }
    void setGenre(const QString &genre) { m_tags["GENRE"] = genre; m_modified = true; }
    void setYear(const QString &year) { m_tags["DATE"] = year; m_modified = true; }
    void setTrackNumber(const QString &track) { m_tags["TRACKNUMBER"] = track; m_modified = true; }
    void setAlbumArtist(const QString &albumArtist) { m_tags["ALBUMARTIST"] = albumArtist; m_modified = true; }
    void setComment(const QString &comment) { m_tags["COMMENT"] = comment; m_modified = true; }

    // Generic tag access
    QMap<QString, QString> tags() const { return m_tags; }
    void setTag(const QString &key, const QString &value) { m_tags[key] = value; m_modified = true; }
    QString tag(const QString &key) const { return m_tags.value(key); }

    // File operations
    bool loadMetadata();
    bool saveMetadata();
    bool isModified() const { return m_modified; }
    void clearModified() { m_modified = false; }

private:
    QString m_filePath;
    QString m_fileName;
    QString m_fileFormat;
    qint64 m_fileSize;
    
    int m_bitrate;
    int m_sampleRate;
    int m_channels;
    int m_duration;
    
    QMap<QString, QString> m_tags;
    bool m_modified;

    void readAudioProperties();
    void readMetadata();
};

#endif // AUDIOFILE_H