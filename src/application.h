#ifndef APPLICATION_H
#define APPLICATION_H

#include <memory>
#include "mainwindow.h"

class Application
{
public:
    Application();
    ~Application();

private:
    std::unique_ptr<MainWindow> m_mainWindow;
};

#endif // APPLICATION_H