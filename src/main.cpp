#include <QApplication>
#include <QStyleFactory>
#include <QPalette>
#include "application.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("tagQ");
    app.setApplicationVersion("0.1");
    app.setOrganizationName("tagQ");

    // Fusion style — clean cross-platform look
    app.setStyle(QStyleFactory::create("Fusion"));

    QPalette pal;
    pal.setColor(QPalette::Window,          QColor(240, 240, 240));
    pal.setColor(QPalette::WindowText,      QColor(30,  30,  30));
    pal.setColor(QPalette::Base,            QColor(255, 255, 255));
    pal.setColor(QPalette::AlternateBase,   QColor(245, 248, 255));
    pal.setColor(QPalette::ToolTipBase,     QColor(255, 255, 220));
    pal.setColor(QPalette::ToolTipText,     QColor(0,   0,   0));
    pal.setColor(QPalette::Text,            QColor(20,  20,  20));
    pal.setColor(QPalette::Button,          QColor(225, 228, 235));
    pal.setColor(QPalette::ButtonText,      QColor(20,  20,  20));
    pal.setColor(QPalette::Highlight,       QColor(42,  130, 218));
    pal.setColor(QPalette::HighlightedText, QColor(255, 255, 255));
    app.setPalette(pal);

    Application application;
    return app.exec();
}
