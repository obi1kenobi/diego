#include "LegoMainWindow.h"
#include "moc_LegoMainWindow.cpp"
#include "ui_MainWindow.h"

#include "LegoApp.h"
#include "LegoNotices.h"

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
    _timer(new QTimer(this)),
    _polling(false)
{
    // Set up
    _ui->setupUi(this);

    // We will store the viewers in here (if we end up with more than one)
    _stackedViewWidget = new QStackedWidget();
    _ui->viewportsLayout->addWidget(_stackedViewWidget, 0, 0);

    // Slots/signals
    connect(_ui->opBox, 
            SIGNAL(returnPressed()), this, 
            SLOT(_NewOp()));
    connect(_ui->actionNewUniverse,
            SIGNAL(triggered(bool)), this,
            SLOT(_NewUniverse()));
    connect(_ui->actionImportModels,
            SIGNAL(triggered(bool)), this,
            SLOT(_ImportModels()));
    connect(_ui->actionSelect,
            SIGNAL(triggered(bool)), this,
            SLOT(_SelectMode()));
    connect(_ui->actionView,
            SIGNAL(triggered(bool)), this,
            SLOT(_ViewMode()));
    connect(_ui->actionDumpScenegraph,
            SIGNAL(triggered(bool)), this,
            SLOT(_DumpScenegraph()));
    connect(_ui->actionNetwork, 
            SIGNAL(triggered(bool)), this,
            SLOT(_SetNetworkEnabled(bool)));
    connect(_ui->actionGravity, 
            SIGNAL(triggered(bool)), this,
            SLOT(_SetGravityEnabled(bool)));

    _RegisterNoticeHandlers();
}

LegoMainWindow::~LegoMainWindow()
{
    _UnregisterNoticeHandlers();
}

void
LegoMainWindow::_RegisterNoticeHandlers()
{
    _noticeKeys.push_back(
        SfNoticeMgr::Get().Register(this, 
            &LegoMainWindow::_ProcessTransactionProcessedNotice));
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

    // Timer for polling
    connect(_timer, SIGNAL(timeout()), this, SLOT(_PollServer()));
    _timer->start(LEGO_POLL_INTERVAL);
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
    // Not thread safe but it won't be called outside of the main thread
    if (_app && !_polling) {
        _polling = true;
        _app->PollServer();
        _polling = false;
    }
}

void
LegoMainWindow::_SetNetworkEnabled(bool enabled)
{
    _app->SetNetworkEnabled(enabled);
}

void
LegoMainWindow::_ProcessTransactionProcessedNotice(
    const LegoTransactionProcessed &n)
{
    _ui->logTextEdit->moveCursor(QTextCursor::End);
    _ui->logTextEdit->insertPlainText(n.Get().c_str());
}

void
LegoMainWindow::_SelectMode()
{
    _app->SetViewerMode(LegoApp::VIEWER_MODE_SELECT);
}

void
LegoMainWindow::_ViewMode()
{
    _app->SetViewerMode(LegoApp::VIEWER_MODE_VIEW);
}

void
LegoMainWindow::_NewUniverse()
{
    _app->NewUniverse();
}

void
LegoMainWindow::_SetGravityEnabled(bool gravity)
{
    _app->SetGravityEnabled(gravity);
}
