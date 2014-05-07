#include "LegoUniverse.h"
#include "MainWindow.h"
#include "LegoApp.h"

#include <QtWidgets/QApplication>
#include <QtWidgets/QStyleFactory>
#include <QtCore/QFile>

#include <cstdlib>

void
_SetStyle(QApplication *qapp)
{
    qapp->setStyle(QStyleFactory::create("fusion"));

    QPalette palette;
    palette.setColor(QPalette::Window, QColor(53,53,53));
    palette.setColor(QPalette::WindowText, Qt::white);
    palette.setColor(QPalette::Base, QColor(15,15,15));
    palette.setColor(QPalette::AlternateBase, QColor(53,53,53));
    palette.setColor(QPalette::ToolTipBase, Qt::white);
    palette.setColor(QPalette::ToolTipText, Qt::white);
    palette.setColor(QPalette::Text, Qt::white);
    palette.setColor(QPalette::Button, QColor(53,53,53));
    palette.setColor(QPalette::Active, QPalette::ButtonText, Qt::white);
    palette.setColor(QPalette::Inactive, QPalette::ButtonText, Qt::white);
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(128, 128, 128));
    palette.setColor(QPalette::BrightText, Qt::red);

    palette.setColor(QPalette::Highlight, QColor(142,45,197).lighter());
    palette.setColor(QPalette::HighlightedText, Qt::black);
    qapp->setPalette(palette);
}

int main(int argc, char *argv[])
{
    QApplication qapp(argc, argv);
    _SetStyle(&qapp);
    // QStyleFactory::create("Fusion"));
    // QFile qss("stylesheets/darkorange.qss");
    // qss.open(QFile::ReadOnly);
    // qapp.setStyleSheet(qss.readAll());
    // qss.close();

    // XXX: Work around Qt bug #28816.
    // https://bugreports.qt-project.org/browse/QTBUG-28816
    // Without this, in the presence of GL widgets, tree widgets go bonkers
    // and cause all sorts of crashes. Apparently this is because the GL
    // widget causes all sibling widgets to be native and Cocoa native
    // widgets are still buggy in 5.01.
    qapp.setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);

    LegoMainWindow mainWindow;
    LegoApp app(&mainWindow);

    mainWindow.SetApp(&app);
    mainWindow.show();
    
    return qapp.exec();
}
