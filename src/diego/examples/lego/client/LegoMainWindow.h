#ifndef __MAIN_WINDOW_H__
#define __MAIN_WINDOW_H__

#include "ui_MainWindow.h"

#include "LegoNotices.h"
#include "NoticeMgr.h"

#include <QtWidgets/QMainWindow>

namespace Ui { class MainWindow;}

class LegoApp;
class LegoTransactionProcessed;
class QStackedWidget;
class QTimer;

class LegoMainWindow : public QMainWindow
{
    Q_OBJECT
    
  public:
    explicit LegoMainWindow(QWidget *parent = 0);

    void SetApp(LegoApp *app);

    virtual ~LegoMainWindow();

  protected slots:
    void _ImportModels();
    void _AddBrick();
    void _NewOp();
    void _DumpScenegraph();
    void _PollServer();
    void _SetNetworkEnabled(bool enabled);
    void _SelectMode();
    void _ViewMode();
    void _NewUniverse();

  private:
    void _RegisterNoticeHandlers();
    void _UnregisterNoticeHandlers();
    void _Initialize();
    void _ProcessTransactionProcessedNotice(const LegoTransactionProcessed &n);

    Ui::MainWindow             *_ui;
    LegoApp                    *_app;
    QStackedWidget             *_stackedViewWidget;
    QTimer                     *_timer;
    std::vector<SfNoticeMgr::Key> _noticeKeys;
};

#endif // __MAIN_WINDOW_H__
