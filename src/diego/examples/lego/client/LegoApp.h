#ifndef __LEGO_APP_H__
#define __LEGO_APP_H__

#include "LegoTransaction.h"
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

    void InitializeViewers(QWidget *, QWidget *parentWidget);

    const std::vector<QWidget*> & GetViewerWidgets() const {
        return _viewerWidgets;
    }

    bool ProcessOp(const std::string &op);

    void ImportModels(const std::vector<std::string> &modelFileNames);

    const std::vector<LegoTransaction> & GetTransactionLog();

    void PollServer();

    void SetNetworkEnabled(bool enabled);

    bool IsNetworkEnabled() const;

    void DumpScenegraph();

  private:
    struct _CallbackData {
        LegoApp *app;
        int viewerIndex;
    };

    void _CreateUniverse();

    void _CreateScene();

    SoSeparator * _CreatePlatformBed();

    void _BuildBricks();

    static void _BeginRenderSceneCB(void *userData, SoAction *action);

    void _DrawBackground();

    void _RegisterNoticeHandlers();

    void _UnregisterNoticeHandlers();

    void _ProcessLegoBricksChangedNotice(const LegoBricksChangedNotice &n);

    void _AddBrick(LegoBrick *brick,
                   uint32_t brickIndex,
                   SbVec3f *coords,
                   SbVec3f *colors,
                   int32_t *indices,
                   int32_t *matIndices,
                   int32_t *texIndices);

    std::vector<SfNoticeMgr::Key> _noticeKeys;

    MfVec3i                     _worldSize;
    MfVec3i                     _worldMin;
    MfVec3i                     _worldMax;

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
    SoMaterial                 *_brickMaterial;
    SoTextureCoordinate2       *_brickTexCoords;
    SoIndexedFaceSet           *_brickIFS;
};

#endif // __LEGO_APP_H__
