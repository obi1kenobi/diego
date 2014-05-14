#include "LegoMainWindow.h"
#include "moc_LegoMainWindow.cpp"
#include "ui_MainWindow.h"

#include "LegoApp.h"

#include <QtCore/QTimer>
#include <QtWidgets/QActionGroup>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStackedWidget>

static const int LEGO_POLL_INTERVAL = 200; // ms

LegoMainWindow::LegoMainWindow(QWidget *parent) :
    QMainWindow(parent),
    _ui(new Ui::MainWindow),
    _app(NULL),
    _timer(new QTimer(this))
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
    connect(_ui->actionImportModels,
            SIGNAL(triggered(bool)), this,
            SLOT(_ImportModels()));
    connect(_ui->actionDumpScenegraph,
            SIGNAL(triggered(bool)), this,
            SLOT(_DumpScenegraph()));
    connect(_ui->actionNetwork, 
            SIGNAL(triggered(bool)), this,
            SLOT(_SetNetworkEnabled(bool)));

    _RegisterNoticeHandlers();
}

LegoMainWindow::~LegoMainWindow()
{
    _UnregisterNoticeHandlers();
}

void
LegoMainWindow::_RegisterNoticeHandlers()
{
}

void
LegoMainWindow::_UnregisterNoticeHandlers()
{
    for (const auto &key : _noticeKeys) {
        SfNoticeMgr::Get().Cancel(key);
    }
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

    // Timer for polling
    connect(_timer, SIGNAL(timeout()), this, SLOT(_PollServer()));
    _timer->start(LEGO_POLL_INTERVAL);
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

void
LegoMainWindow::_DumpScenegraph()
{
    _app->DumpScenegraph();
}

void
LegoMainWindow::_ImportModels()
{
    _timer->stop();

    QStringList paths = 
        QFileDialog::getOpenFileNames(this, 
                                     tr("Import Models"), 
                                     ".", 
                                     tr("Models (*.stl)"));
    std::vector<std::string> models;
    QStringList pathList = paths;
    for (QStringList::Iterator it = pathList.begin(); it != pathList.end(); ++it) {
        models.push_back(it->toStdString());
    }

    if (models.empty()) {
        return;
    }

    _app->ImportModels(models);

    _timer->start(LEGO_POLL_INTERVAL);
}

void
LegoMainWindow::_PollServer()
{
    if (_app) {
        _app->PollServer();
    }
}

void
LegoMainWindow::_SetNetworkEnabled(bool enabled)
{
    _app->SetNetworkEnabled(enabled);
}
