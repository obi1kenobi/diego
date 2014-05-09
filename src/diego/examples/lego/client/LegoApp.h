#ifndef __LEGO_APP_H__
#define __LEGO_APP_H__

#include "NoticeMgr.h"
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
class LegoBrick;
class LegoBricksChangedNotice;
class LegoMainWindow;
class LegoUniverse;

class LegoApp {
  public:
    LegoApp(LegoMainWindow *mainWindow);

    ~LegoApp();

    bool ProcessOp(const std::string &op);

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

    void _RegisterNoticeHandlers();

    void _UnregisterNoticeHandlers();

    void _ProcessLegoBricksChangedNotice(const LegoBricksChangedNotice &n);

    void _AddBrick(LegoBrick *brick,
                   uint32_t brickIndex,
                   SbVec3f *coords,
                   int32_t *indices);

    std::vector<SfNoticeMgr::Key> _noticeKeys;

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
    SoCoordinate3              *_brickCoords;
    SoIndexedFaceSet           *_brickIFS;
};

#endif // __LEGO_APP_H__
