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
class LegoConflictNotice;
class LegoMainWindow;
class LegoUniverse;
class SoAlarmSensor;
class SoEventCallback;
class SoPickedPoint;
class SoSensor;

class LegoApp {
  public:
     enum ViewerMode {
       VIEWER_MODE_SELECT,
       VIEWER_MODE_VIEW,
     };

    LegoApp(LegoMainWindow *mainWindow);

    ~LegoApp();

    void InitializeViewers(QWidget *, QWidget *parentWidget);

    const std::vector<QWidget*> & GetViewerWidgets() const {
        return _viewerWidgets;
    }

    void NewUniverse();

    bool ProcessOp(const std::string &op);

    void ImportModels(const std::vector<std::string> &modelFileNames);

    const std::vector<LegoTransaction> & GetTransactionLog();

    void PollServer();

    void SetNetworkEnabled(bool enabled);

    bool IsNetworkEnabled() const;

    void SetViewerMode(ViewerMode viewerMode);

    void SetGravityEnabled(bool enabled);

    bool IsGravityEnabled() const;

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

    void _ProcessLegoConflictNotice(const LegoConflictNotice &n);

    void _AddBrick(LegoBrick *brick,
                   uint32_t brickIndex,
                   SbVec3f *coords,
                   uint32_t *colors,
                   int32_t *indices,
                   int32_t *matIndices,
                   int32_t *texIndices,
                   bool selected);

    static void _ToggleFlash(void *userData, SoSensor *sensor);

    static void _EventCB(void *userData, SoEventCallback *eventCB);

    void _HandleSelect(const SoPickedPoint *pickedPoint);

    void _HandleCreate(const SoPickedPoint *pickedPoint);

    void _HandleDelete(const SoPickedPoint *pickedPoint);

    static void _UpdateCB(void *userData, SoSensor *sensor);

    void _Update();

    std::vector<SfNoticeMgr::Key> _noticeKeys;

    MfVec3i                     _worldSize;
    MfVec3i                     _worldMin;
    MfVec3i                     _worldMax;

    MfVec3d                     _bedSize;
    LegoMainWindow             *_mainWindow;
    std::vector<QWidget*>       _viewerWidgets;
    std::vector<SoQtViewer*>    _viewers;
    SoSeparator                *_viewerRoot;
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
    SoVertexProperty           *_brickVP;
    SoIndexedFaceSet           *_brickIFS;
    SoAlarmSensor              *_flashAlarm;
    bool                        _flash;
    bool                        _shiftDown;
    bool                        _ctrlDown;
    SoOneShotSensor            *_updateSensor;
    bool                        _bricksDirty;
};

#endif // __LEGO_APP_H__
