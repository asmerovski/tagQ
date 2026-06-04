#include "audiofile.h"

#include <QFileInfo>

#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/audioproperties.h>
#include <taglib/tstring.h>

static inline QString fromTStr(const TagLib::String &s)
{
    return QString::fromStdString(s.to8Bit(true));
}

static inline TagLib::String toTStr(const QString &s)
{
    return TagLib::String(s.toStdString(), TagLib::String::UTF8);
}

AudioFile::AudioFile(const QString &filePath)
    : m_filePath(filePath),
      m_fileSize(0),
      m_bitrate(0),
      m_sampleRate(0),
      m_channels(0),
      m_duration(0),
      m_modified(false)
{
    QFileInfo fi(filePath);
    m_fileName   = fi.fileName();
    m_fileFormat = fi.suffix().toUpper();
    m_fileSize   = fi.size();

    loadMetadata();
}

AudioFile::~AudioFile() = default;

bool AudioFile::loadMetadata()
{
    TagLib::FileRef ref(m_filePath.toLocal8Bit().constData());
    if (ref.isNull())
        return false;

    if (ref.tag()) {
        auto *tag = ref.tag();
        m_tags["TITLE"]       = fromTStr(tag->title());
        m_tags["ARTIST"]      = fromTStr(tag->artist());
        m_tags["ALBUM"]       = fromTStr(tag->album());
        m_tags["GENRE"]       = fromTStr(tag->genre());
        m_tags["DATE"]        = tag->year()  > 0 ? QString::number(tag->year())  : QString();
        m_tags["TRACKNUMBER"] = tag->track() > 0 ? QString::number(tag->track()) : QString();
        m_tags["COMMENT"]     = fromTStr(tag->comment());
        // TagLib generic doesn't expose ALBUMARTIST in the base class;
        // we leave it blank — format-specific code can extend later.
        m_tags["ALBUMARTIST"] = QString();
    }

    if (ref.audioProperties()) {
        auto *ap  = ref.audioProperties();
        m_bitrate    = ap->bitrate();
        m_sampleRate = ap->sampleRate();
        m_channels   = ap->channels();
        m_duration   = ap->lengthInSeconds();
    }

    m_modified = false;
    return true;
}

bool AudioFile::saveMetadata()
{
    TagLib::FileRef ref(m_filePath.toLocal8Bit().constData());
    if (ref.isNull() || !ref.tag())
        return false;

    auto *tag = ref.tag();
    tag->setTitle(toTStr(m_tags.value("TITLE")));
    tag->setArtist(toTStr(m_tags.value("ARTIST")));
    tag->setAlbum(toTStr(m_tags.value("ALBUM")));
    tag->setGenre(toTStr(m_tags.value("GENRE")));
    tag->setYear(m_tags.value("DATE").toUInt());
    tag->setTrack(m_tags.value("TRACKNUMBER").toUInt());
    tag->setComment(toTStr(m_tags.value("COMMENT")));

    if (!ref.save())
        return false;

    m_modified = false;
    return true;
}
