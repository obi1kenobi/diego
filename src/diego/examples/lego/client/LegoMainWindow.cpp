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

    // Slots/signals
    connect(_ui->opBox, 
            SIGNAL(returnPressed()), this, 
            SLOT(_NewOp()));
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

    // Display transaction log from server
    const auto &xaLog = _app->GetTransactionLog();
    for (const auto &xa : xaLog) {
        const auto &ops = xa.GetOps();
        for (const auto op : ops) {
            std::ostringstream os;
            op.Serialize(os);
            _ui->logTextEdit->moveCursor(QTextCursor::End);
            _ui->logTextEdit->insertPlainText(os.str().c_str());
        }
    }
}

void
LegoMainWindow::_AddBrick()
{
}

void
LegoMainWindow::_NewOp()
{
    std::string op = _ui->opBox->text().toStdString();
    bool success = _app->ProcessOp(op);
    if (success) {
        _ui->logTextEdit->appendPlainText(op.c_str());
    }
    _ui->opBox->clear();
}
