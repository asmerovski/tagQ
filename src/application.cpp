#include "application.h"

Application::Application()
    : m_mainWindow(std::make_unique<MainWindow>())
{
    m_mainWindow->show();
}

Application::~Application() = default;