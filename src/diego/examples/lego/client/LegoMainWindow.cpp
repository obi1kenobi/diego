#include "LegoMainWindow.h"
#include "moc_LegoMainWindow.cpp"
#include "ui_MainWindow.h"

#include "LegoApp.h"

#include <QtWidgets/QActionGroup>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStackedWidget>

LegoMainWindow::LegoMainWindow(QWidget *parent) :
    QMainWindow(parent),
    _ui(new Ui::MainWindow),
    _app(NULL)
{
    // Set up
    _ui->setupUi(this);

    // We will store the viewers in here
    _stackedViewWidget = new QStackedWidget();
    _ui->viewportsLayout->addWidget(_stackedViewWidget, 0, 0);
}

LegoMainWindow::~LegoMainWindow()
{
}

void
LegoMainWindow::SetApp(LegoApp *app)
{
    _app = app;

    _Initialize();
}

void
LegoMainWindow::_Initialize()
{
    // Create and add viewer widgets to UI
    _app->InitializeViewers(this, _stackedViewWidget);
    const std::vector<QWidget*> &viewerWidgets = _app->GetViewerWidgets();
    for (auto *viewerWidget : viewerWidgets) {
        _stackedViewWidget->addWidget(viewerWidget);
    }
}

void
LegoMainWindow::_AddBrick()
{
}
