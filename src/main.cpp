#include <QApplication>
#include <KAboutData>
#include <KLocalizedString>
#include "application.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    KAboutData aboutData(
        "tagQ",
        i18n("tagQ"),
        "1.0.0",
        i18n("Audio metadata tagger for KDE6"),
        KAboutLicense::GPL_V3,
        i18n("Copyright 2026, asmerovski"),
        QString(),
        "https://github.com/asmerovski/tagQ"
    );
    
    aboutData.addAuthor(
        i18n("asmerovski"),
        i18n("Developer"),
        "asmerovski@example.com",
        "https://github.com/asmerovski"
    );
    
    KAboutData::setApplicationData(aboutData);
    
    Application application;
    return app.exec();
}