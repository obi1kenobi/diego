#ifndef __MAIN_WINDOW_H__
#define __MAIN_WINDOW_H__

#include "ui_MainWindow.h"

#include <QtWidgets/QMainWindow>

namespace Ui { class MainWindow;}

class LegoApp;
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

  private:
    void _Initialize();

    Ui::MainWindow             *_ui;
    LegoApp                    *_app;
    QStackedWidget             *_stackedViewWidget;
    QTimer                     *_timer;
};

#endif // __MAIN_WINDOW_H__
