#include "LegoApp.h"
#include "LegoMainWindow.h"
#include "LegoNotices.h"
#include "LegoTransactionMgr.h"
#include "LegoUniverse.h"

#include <cassert>

LegoApp::LegoApp(LegoMainWindow *mainWindow) :
    _mainWindow(mainWindow),
    _universe(NULL),
    _bedSize(64, 38, 1),
    _sceneRoot(NULL),
    _platformRoot(NULL)
{
    // Initialize the SoQt library first. 
    SoQt::init(mainWindow);

    _CreateUniverse();
    _CreateScene();
    _BuildBricks();

    _RegisterNoticeHandlers();
}

LegoApp::~LegoApp()
{
    _UnregisterNoticeHandlers();
    delete _universe; _universe = NULL;
}

bool
LegoApp::ProcessOp(const std::string &op)
{
    return _universe->ProcessOp(op);
}

void
LegoApp::InitializeViewers(QWidget *, QWidget *parentWidget)
{
    assert(_viewerWidgets.empty());
    _viewerWidgets.clear();

    SoQtExaminerViewer *viewer = 
        new SoQtExaminerViewer(parentWidget, NULL, true, 
                               SoQtFullViewer::BUILD_NONE, 
                               SoQtViewer::BROWSER);
    viewer->setCameraType(SoPerspectiveCamera::getClassTypeId());
    viewer->setBackgroundColor(SbColor(1, 1, 1));

    SoDirectionalLight *headlight = viewer->getHeadlight();
    headlight->intensity.setValue(0.5);

    // Careful; have to set up the scenegraph to a viewer before
    // retrieving the camera. It's the process of setting the
    // scenegraph that triggers the initial camera creation.
    viewer->setSceneGraph(_viewerRoots[0]);

    // Initialize camera differently for each viewer to set up the
    // following initial conditions:
    // - viewer1 through viewer4 are straight camera viewers
    // - fab preview viewer previews the results of fabrication
    // - viewer1 and fab preview show a perspective view
    // - viewer2-4 show an orthographic view
    // XXX-desai: doesn't seem to have an effect

    // Z-up
    SbRotation zUp(SbVec3f(1, 0, 0), M_PI / 2.0);

    SoCamera *camera = viewer->getCamera();
    assert(camera);
    SoPerspectiveCamera *pcamera = dynamic_cast<SoPerspectiveCamera*>(camera);
    pcamera->nearDistance.setValue(1);
    pcamera->orientation.setValue(zUp * SbRotation(SbVec3f(1, 0, 0), -M_PI / 8.0f));

    viewer->setDoubleBuffer(true);
    viewer->show();
    viewer->render();

    // No geometry at the beginning, so, frame the platform bed
    viewer->getCamera()->viewAll(_sceneEnv, viewer->getViewportRegion());

    _viewerWidgets.push_back(viewer->getWidget());
    _viewers.push_back(viewer);
}

void
LegoApp::_CreateUniverse()
{
    MfVec3i gridSize(100);
    _universe = new LegoUniverse(gridSize);
}

void
LegoApp::_CreateScene()
{
    // Scene structure:
    //
    // Root (_sceneRoot)
    //   Shadow group
    //     Scene meshes (_sceneGroup)
    //     Scene environment (_sceneEnvSwitch)
    //     Light (_shadowLight)
    //
    // Viewers
    //   Viewers 1-4:
    //   Root
    //     Camera
    //     Render Callback (draw background)
    //     _sceneRoot
    //
    //   Preview viewer:
    //     Camera
    //     Preview Switch (_previewSwitch)
    //       Render Callback (draw slice if not in volume mode or else draw background)
    //       Preview Volume Root
    //         

    // The whole scene
    _sceneRoot = new SoSeparator();

    _shadowGroup = new SoShadowGroup();
    _shadowGroup->quality.setValue(1);
    _shadowGroup->precision.setValue(1);
    _sceneRoot->addChild(_shadowGroup);

    // Meshes will only cast shadows
    SoShadowStyle *meshesShadowStyle = new SoShadowStyle();
    meshesShadowStyle->style.setValue(SoShadowStyle::CASTS_SHADOW);
    _shadowGroup->addChild(meshesShadowStyle);

    // The scene meshes
    _sceneGroup = new SoSeparator();
    _shadowGroup->addChild(_sceneGroup);

    // Default material for meshes
    SoMaterial *meshMtl = new SoMaterial();
    meshMtl->diffuseColor.setValue(SbColor(1, 1, 1));
    _sceneGroup->addChild(meshMtl);

    // Default draw style
    _sceneDrawStyle = new SoDrawStyle();
    _sceneGroup->addChild(_sceneDrawStyle);

    // Default pick style
    SoPickStyle *pickStyle = new SoPickStyle();
    pickStyle->style.setValue(SoPickStyle::UNPICKABLE);
    _sceneGroup->addChild(pickStyle);

    // Enable backface culling
    //
    // XXX: Disabled because when we display the slice view, if there are any
    // empty voxels, we want them to appear black (i.e., see the back of
    // the object). We may want to disable/enable culling based on whether
    // slice preview is on or off, though.
    _sceneShapeHints = new SoShapeHints();
    _sceneShapeHints->creaseAngle.setValue(M_PI);
    _sceneShapeHints->vertexOrdering.setValue(SoShapeHints::UNKNOWN_ORDERING);
    _sceneShapeHints->shapeType.setValue(SoShapeHints::UNKNOWN_SHAPE_TYPE);
    _sceneShapeHints->faceType.setValue(SoShapeHints::UNKNOWN_FACE_TYPE);
    _sceneShapeHints->creaseAngle.setValue(0);
    _sceneGroup->addChild(_sceneShapeHints);

    // Highest texture quality (i.e., best filter available, etc.)
    SoComplexity *complexity = new SoComplexity();
    complexity->textureQuality.setValue(1);

    // This is where the actual meshes go
    SoSeparator *sceneMeshesRoot = new SoSeparator();
    SoMaterial *defaultMeshMaterial = new SoMaterial();
    defaultMeshMaterial->diffuseColor.setValue(0.2, 0.5, 0.75);
    defaultMeshMaterial->specularColor.setValue(0.2, 0.5, 0.75);
    defaultMeshMaterial->shininess.setValue(1.0);
    sceneMeshesRoot->addChild(defaultMeshMaterial);
    _sceneGroup->addChild(sceneMeshesRoot);  
    _sceneMeshes = new SoSeparator();
    sceneMeshesRoot->addChild(_sceneMeshes);

    // Lego bricks
    _brickCoords = new SoCoordinate3();
    _brickIFS = new SoIndexedFaceSet();
    _sceneMeshes->addChild(_brickCoords);
    _sceneMeshes->addChild(_brickIFS);

    // The scene "environment", i.e., ground plane, etc.
    // It will only receive shadows
    SoShadowStyle *envShadowStyle = new SoShadowStyle();
    envShadowStyle->style.setValue(SoShadowStyle::SHADOWED);
    _shadowGroup->addChild(envShadowStyle);
    _sceneEnvSwitch = new SoSwitch(); 
    _sceneEnvSwitch->whichChild.setValue(SO_SWITCH_ALL);
    _sceneEnv = new SoSeparator();
    _sceneEnvSwitch->addChild(_sceneEnv);
    _shadowGroup->addChild(_sceneEnvSwitch);

    // Create spot light
    _shadowLight = new SoShadowDirectionalLight();
    _shadowLight->direction.setValue(-1, -1, -1);
    _shadowLight->intensity.setValue(4.5);
    _shadowGroup->addChild(_shadowLight);

    SoMaterial *mtlEnv = new SoMaterial();
    mtlEnv->diffuseColor.setValue(SbColor(1, 1, 1));
    _sceneEnv->addChild(mtlEnv);

    // Create scene environment: a ground plane
    SoSeparator *platformRoot = _CreatePlatformBed();
    _sceneEnv->addChild(platformRoot);

    SoShadowStyle *envNoShadow = new SoShadowStyle();
    envNoShadow->style.setValue(SoShadowStyle::NO_SHADOWING);
    _sceneEnv->addChild(envNoShadow);

    // Create viewer roots
    SoSeparator *viewerRoot = new SoSeparator();
    SoCamera *camera = new SoPerspectiveCamera();
    viewerRoot->addChild(camera);

    _CallbackData *cbData = new _CallbackData();
    cbData->app = this;
    cbData->viewerIndex = 0;
    _cbData.push_back(cbData);
    SoCallback *beginRenderCB = new SoCallback();
    beginRenderCB->setCallback(_BeginRenderSceneCB, cbData);
    viewerRoot->addChild(beginRenderCB);
    viewerRoot->addChild(_sceneRoot);
    _viewerRoots.push_back(viewerRoot);
}

SoSeparator *
LegoApp::_CreatePlatformBed()
{
    if (!_platformRoot) {
        _platformRoot = new SoSeparator();
        SoComplexity *complexity = new SoComplexity();
        complexity->textureQuality.setValue(1);
        _platformRoot->addChild(complexity);
        SoDirectionalLight *dlight = new SoDirectionalLight();
        dlight->direction.setValue(-1, -1, -1);
        dlight->intensity.setValue(4.5);
        _platformRoot->addChild(dlight);
        _platformXf = new SoTransform();
        _platformXf->translation.setValue(0, 0, -_bedSize[2] / 2.0f);
        SoTexture2 *platformTex = new SoTexture2();
        platformTex->filename.setValue("textures/groundTexture.jpg");
        _platformCube = new SoCube();
        _platformCube->width.setValue(_bedSize[0]);
        _platformCube->height.setValue(_bedSize[1]);
        _platformCube->depth.setValue(_bedSize[2]);
        _platformRoot->addChild(_platformXf);
        _platformRoot->addChild(platformTex);
        _platformRoot->addChild(_platformCube);
    }
    return _platformRoot;
}

void
LegoApp::_BeginRenderSceneCB(void *userData, SoAction *action)
{
    if (action->isOfType(SoGLRenderAction::getClassTypeId())) {
        SoCacheElement::invalidate(action->getState());

        _CallbackData *cbData = reinterpret_cast<_CallbackData*>(userData);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();

        glPushAttrib(GL_DEPTH_BUFFER_BIT | 
                     GL_COLOR_BUFFER_BIT |
                     GL_LIGHTING_BIT | 
                     GL_SCISSOR_BIT | 
                     GL_POLYGON_BIT |
                     GL_CURRENT_BIT);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_ALPHA_TEST);
        glDisable(GL_LIGHTING);
        glDisable(GL_COLOR_MATERIAL);
        glDisable(GL_SCISSOR_TEST);
        glDisable(GL_CULL_FACE);

        cbData->app->_DrawBackground();

        glPopAttrib();

        glMatrixMode(GL_PROJECTION);
        glPopMatrix();

        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
    }
}

void
LegoApp::_DrawBackground()
{
    MfVec3f tc(116.0f / 255.0f, 146.0f / 255.0f, 164.0f / 255.0f);
    MfVec3f bc( 24.0f / 255.0f,  27.0f / 255.0f,  29.0f / 255.0f);

    glBegin(GL_QUADS);
    glColor3f(bc[0], bc[1], bc[2]);
    glVertex2f(-1, -1);

    glColor3f(bc[0], bc[1], bc[2]);
    glVertex2f( 1, -1);

    glColor3f(tc[0], tc[1], tc[2]);
    glVertex2f( 1,  1);

    glColor3f(tc[0], tc[1], tc[2]);
    glVertex2f(-1,  1);

    glEnd();
}

void
LegoApp::_RegisterNoticeHandlers()
{
    _noticeKeys.push_back(
        SfNoticeMgr::Get().Register(this, 
            &LegoApp::_ProcessLegoBricksChangedNotice));
}

void
LegoApp::_UnregisterNoticeHandlers()
{
    for (const auto &key : _noticeKeys) {
        SfNoticeMgr::Get().Cancel(key);
    }
}

inline
static
void
_AddTriangle(int32_t *indices, uint32_t *startIndex, uint32_t v0, uint32_t v1, uint32_t v2)
{
    indices[(*startIndex)++] = v0;
    indices[(*startIndex)++] = v1;
    indices[(*startIndex)++] = v2;
    indices[(*startIndex)++] = -1;
}

void
LegoApp::_AddBrick(LegoBrick *brick,
                   uint32_t brickIndex,
                   SbVec3f *coords,
                   int32_t *indices)
{
    const MfVec3i &pos = brick->GetPosition();
    const MfVec3i &size = brick->GetSize();

    //         6-----7
    //       / |   / |
    //     2------3  |
    //     |   4- |- 5
    //     | /    | / 
    //     0------1
    float xStart = pos[0];
    float yStart = pos[1];
    float zStart = pos[2];
    float xFaceSize = size[0];
    float yFaceSize = size[1];
    float zFaceSize = size[2];

    uint32_t startVertex = brickIndex * 8;
    uint32_t vindex = startVertex;
    coords[vindex++] = SbVec3f(xStart,             yStart,             zStart + zFaceSize);
    coords[vindex++] = SbVec3f(xStart + xFaceSize, yStart,             zStart + zFaceSize);
    coords[vindex++] = SbVec3f(xStart,             yStart + yFaceSize, zStart + zFaceSize);
    coords[vindex++] = SbVec3f(xStart + xFaceSize, yStart + yFaceSize, zStart + zFaceSize);
    coords[vindex++] = SbVec3f(xStart,             yStart,             zStart);
    coords[vindex++] = SbVec3f(xStart + xFaceSize, yStart,             zStart);
    coords[vindex++] = SbVec3f(xStart,             yStart + yFaceSize, zStart);
    coords[vindex++] = SbVec3f(xStart + xFaceSize, yStart + yFaceSize, zStart);

    uint32_t iindex = brickIndex * 12 * 4;
    _AddTriangle(indices, &iindex, startVertex + 0, startVertex + 1, startVertex + 2);
    _AddTriangle(indices, &iindex, startVertex + 2, startVertex + 1, startVertex + 3);
    _AddTriangle(indices, &iindex, startVertex + 5, startVertex + 4, startVertex + 7);
    _AddTriangle(indices, &iindex, startVertex + 7, startVertex + 4, startVertex + 6);
    _AddTriangle(indices, &iindex, startVertex + 4, startVertex + 0, startVertex + 6);
    _AddTriangle(indices, &iindex, startVertex + 6, startVertex + 0, startVertex + 2);
    _AddTriangle(indices, &iindex, startVertex + 1, startVertex + 5, startVertex + 3);
    _AddTriangle(indices, &iindex, startVertex + 3, startVertex + 5, startVertex + 7);
    _AddTriangle(indices, &iindex, startVertex + 2, startVertex + 3, startVertex + 6);
    _AddTriangle(indices, &iindex, startVertex + 6, startVertex + 3, startVertex + 7);
    _AddTriangle(indices, &iindex, startVertex + 4, startVertex + 5, startVertex + 0);
    _AddTriangle(indices, &iindex, startVertex + 0, startVertex + 5, startVertex + 1);
}

void
LegoApp::_ProcessLegoBricksChangedNotice(const LegoBricksChangedNotice &)
{
    _BuildBricks();
}

void
LegoApp::_BuildBricks()
{
    const auto &bricks = _universe->GetBricks();

    _brickCoords->point.setNum(8 * bricks.size());
    SbVec3f *coords = _brickCoords->point.startEditing();

    _brickIFS->coordIndex.setNum(12 * 4 * bricks.size());
    int32_t *indices = _brickIFS->coordIndex.startEditing();

    for (uint32_t i = 0; i < bricks.size(); ++i) {
        auto *brick = bricks[i];
        _AddBrick(brick, i, coords, indices);
    }

    _brickCoords->point.finishEditing();
    _brickIFS->coordIndex.finishEditing();
}

const std::vector<LegoTransaction> &
LegoApp::GetTransactionLog()
{
    return _universe->GetTransactionMgr()->GetLog();
}
