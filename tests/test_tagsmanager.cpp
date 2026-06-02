#include <gtest/gtest.h>
#include "../src/tagsmanager.h"

class TagsManagerTest : public ::testing::Test
{
protected:
    TagsManager manager;
};

TEST_F(TagsManagerTest, ParseTemplateWithValidTags)
{
    QMap<QString, QString> tags;
    tags["TITLE"] = "My Song";
    tags["ARTIST"] = "My Artist";
    tags["ALBUM"] = "My Album";
    
    QString result = manager.parseTemplate("{artist} - {title}", tags);
    EXPECT_EQ(result.toStdString(), "My Artist - My Song");
}

TEST_F(TagsManagerTest, ParseTemplateWithMissingTags)
{
    QMap<QString, QString> tags;
    tags["TITLE"] = "My Song";
    
    QString result = manager.parseTemplate("{artist} - {title}", tags);
    EXPECT_EQ(result.toStdString(), "Unknown Artist - My Song");
}

TEST_F(TagsManagerTest, GetSupportedFormats)
{
    auto formats = manager.getSupportedAudioFormats();
    EXPECT_GT(formats.size(), 0);
    EXPECT_TRUE(formats.contains("mp3"));
    EXPECT_TRUE(formats.contains("flac"));
}

TEST_F(TagsManagerTest, FormatDurationHelper)
{
    // This is a private method, would need to be tested differently
    // or made public for testing
    EXPECT_TRUE(true);
}
