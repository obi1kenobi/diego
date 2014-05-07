#ifndef __MAIN_WINDOW_H__
#define __MAIN_WINDOW_H__

#include "ui_MainWindow.h"

#include <QtWidgets/QMainWindow>

namespace Ui { class MainWindow;}

class LegoApp;
class QStackedWidget;

class LegoMainWindow : public QMainWindow
{
    Q_OBJECT
    
  public:
    explicit LegoMainWindow(QWidget *parent = 0);

    void SetApp(LegoApp *app);

    virtual ~LegoMainWindow();

  protected slots:
      void _AddBrick();

  private:
    void _Initialize();

    Ui::MainWindow             *_ui;
    LegoApp                    *_app;
    QStackedWidget             *_stackedViewWidget;
};

#endif // __MAIN_WINDOW_H__
