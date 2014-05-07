#ifndef __LEGO_APP_H__
#define __LEGO_APP_H__

#include "Vec3d.h"

#include <Inventor/So.h>
#include <Inventor/Qt/SoQt.h>
#include <Inventor/Qt/viewers/SoQtExaminerViewer.h>
#include <Inventor/Qt/viewers/SoQtPlaneViewer.h>
#include <Inventor/annex/FXViz/nodes/SoShadowGroup.h>
#include <Inventor/annex/FXViz/nodes/SoShadowStyle.h>
#include <Inventor/annex/FXViz/nodes/SoShadowDirectionalLight.h>

#include <vector>

class QWidget;
class LegoMainWindow;
class LegoUniverse;

class LegoApp {
  public:
    LegoApp(LegoMainWindow *mainWindow);

    ~LegoApp();

    void InitializeViewers(QWidget *, QWidget *parentWidget);

    const std::vector<QWidget*> & GetViewerWidgets() const {
        return _viewerWidgets;
    }

  private:
    struct _CallbackData {
        LegoApp *app;
        int viewerIndex;
    };

    void _CreateUniverse();
    void _CreateScene();
    SoSeparator * _CreatePlatformBed();
    static void _BeginRenderSceneCB(void *userData, SoAction *action);
    void _DrawBackground();

    MfVec3d                     _bedSize;
    LegoMainWindow             *_mainWindow;
    std::vector<QWidget*>       _viewerWidgets;
    std::vector<SoQtViewer*>    _viewers;
    std::vector<SoSeparator*>   _viewerRoots;
    LegoUniverse               *_universe;
    std::vector<_CallbackData*> _cbData;

    SoSeparator                *_sceneRoot;
    SoShadowGroup              *_shadowGroup;
    SoSeparator                *_sceneGroup;
    SoSeparator                *_sceneMeshes;
    SoShapeHints               *_sceneShapeHints;
    SoSeparator                *_sceneEnv;
    SoShadowDirectionalLight   *_shadowLight;
    SoSeparator                *_platformRoot;
    SoTransform                *_platformXf;
    SoCube                     *_platformCube;
    SoSwitch                   *_sceneEnvSwitch;
    SoSwitch                   *_previewEnvSwitch;
    SoDrawStyle                *_sceneDrawStyle;
};

#endif // __LEGO_APP_H__
