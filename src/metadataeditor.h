#ifndef METADATAEDITOR_H
#define METADATAEDITOR_H

#include <QWidget>
#include <QMap>
#include <QString>

class QLineEdit;
class QTextEdit;
class QLabel;
class QPushButton;
class QScrollArea;
class AudioFile;

class MetadataEditor : public QWidget
{
    Q_OBJECT

public:
    explicit MetadataEditor(QWidget *parent = nullptr);
    ~MetadataEditor();

public slots:
    void displayFile(const QString &filePath);
    void clearDisplay();

signals:
    void metadataChanged(const QString &filePath, const QMap<QString, QString> &tags);

private slots:
    void onApplyClicked();
    void onRevertClicked();
    void onCoverArtClicked();

private:
    void setupUI();
    void setupFields();
    void loadFileInfo(const std::shared_ptr<AudioFile> &audioFile);
    void updatePreview();

    // Metadata fields
    QLineEdit *m_titleEdit;
    QLineEdit *m_artistEdit;
    QLineEdit *m_albumEdit;
    QLineEdit *m_albumArtistEdit;
    QLineEdit *m_genreEdit;
    QLineEdit *m_yearEdit;
    QLineEdit *m_trackEdit;
    QTextEdit *m_commentEdit;

    // File info display
    QLabel *m_filenameLabel;
    QLabel *m_formatLabel;
    QLabel *m_durationLabel;
    QLabel *m_bitrateLabel;
    QLabel *m_sampleRateLabel;
    QLabel *m_coverArtLabel;

    // Buttons
    QPushButton *m_applyButton;
    QPushButton *m_revertButton;
    QPushButton *m_coverArtButton;

    QString m_currentFilePath;
    QMap<QString, QString> m_originalTags;
    QMap<QString, QString> m_currentTags;

    QString m_coverArtPath;
};

#endif // METADATAEDITOR_H