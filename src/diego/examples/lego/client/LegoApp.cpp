#include "LegoApp.h"
#include "LegoMainWindow.h"
#include "LegoNotices.h"
#include "LegoTransactionMgr.h"
#include "LegoUniverse.h"
#include "LegoVoxelizer.h"
#include "STLReader.h"

#include <Inventor/events/SoMouseButtonEvent.h>

#include <cassert>

LegoApp::LegoApp(LegoMainWindow *mainWindow) :
    _mainWindow(mainWindow),
    _universe(NULL),
    _worldSize(64, 64, 64),
    _worldMin(-31, -31, 0),
    _worldMax(32, 32, 63),
    _bedSize(_worldSize[0], _worldSize[1], 1),
    _sceneRoot(NULL),
    _platformRoot(NULL),
    _flashAlarm(NULL),
    _flash(false),
    _shiftDown(false),
    _ctrlDown(false)
{
    // Initialize the SoQt library first. 
    SoDB::init();
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
    headlight->intensity.setValue(0.8);

    // Careful; have to set up the scenegraph to a viewer before
    // retrieving the camera. It's the process of setting the
    // scenegraph that triggers the initial camera creation.
    viewer->setSceneGraph(_viewerRoot);

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
    _universe = new LegoUniverse(_worldSize);
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
    _sceneRoot->ref();

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
    SoTexture2 *tex = new SoTexture2();
    tex->filename.setValue("textures/legoWhiteTop.jpg");
    _brickMaterial = new SoMaterial();
    _brickMaterial->specularColor.setValue(SbVec3f(1, 1, 1));
    _brickMaterial->shininess.setValue(1);
    _brickVP = new SoVertexProperty();
    _brickVP->materialBinding.setValue(SoMaterialBinding::PER_FACE_INDEXED);
    _brickIFS = new SoIndexedFaceSet();
    _brickIFS->vertexProperty.setValue(_brickVP);
    _sceneMeshes->addChild(tex);
    _sceneMeshes->addChild(_brickMaterial);
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

    // Create viewer root
    if (_viewerRoot) {
        _viewerRoot->removeAllChildren();
    } else {
        _viewerRoot = new SoSeparator();
        _viewerRoot->ref();
    }
    SoCamera *camera = new SoPerspectiveCamera();
    _viewerRoot->addChild(camera);

    // Listen to mouse events
    SoEventCallback *ecb = new SoEventCallback();
    ecb->addEventCallback(SoEvent::getClassTypeId(), _EventCB, this);
    // ecb->addEventCallback(SoKeyboardEvent::getClassTypeId(), _EventCB, this);
    _viewerRoot->addChild(ecb);

    _CallbackData *cbData = new _CallbackData();
    cbData->app = this;
    cbData->viewerIndex = 0;
    _cbData.push_back(cbData);
    SoCallback *beginRenderCB = new SoCallback();
    beginRenderCB->setCallback(_BeginRenderSceneCB, cbData);
    _viewerRoot->addChild(beginRenderCB);
    _viewerRoot->addChild(_sceneRoot);
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
        SoMaterial *mtl = new SoMaterial();
        mtl->shininess.setValue(1);
        mtl->specularColor.setValue(SbVec3f(1, 1, 1));
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
    MfVec3f tc;
    MfVec3f bc;
    if (_flash) {
        tc = MfVec3f(1.0);
        bc = MfVec3f(1.0);
    } else {
        tc = MfVec3f(116.0f / 255.0f, 146.0f / 255.0f, 164.0f / 255.0f);
        bc = MfVec3f( 24.0f / 255.0f,  27.0f / 255.0f,  29.0f / 255.0f);
    }

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
    _noticeKeys.push_back(
        SfNoticeMgr::Get().Register(this, 
            &LegoApp::_ProcessLegoConflictNotice));
}

void
LegoApp::_UnregisterNoticeHandlers()
{
    for (const auto &key : _noticeKeys) {
        SfNoticeMgr::Get().Cancel(key);
    }
}

static
void
_SetBrickVertex(int brickIndex,
                int vertexIndex,
                SbVec3f *coords,
                const SbVec3f &vertex)
{
    int index = brickIndex * 8 + vertexIndex;
    coords[index] = vertex;
}

static
void
_SetBrickColor(int brickIndex,
               uint32_t *colors,
               const SbVec3f &color)
{
    int index = brickIndex;
    colors[index] = 0;
    colors[index] = colors[index] << 8 | (unsigned char) (color[0] * 255);
    colors[index] = colors[index] << 8 | (unsigned char) (color[1] * 255);
    colors[index] = colors[index] << 8 | (unsigned char) (color[2] * 255);
    colors[index] = colors[index] << 8 | (unsigned char) 255;
}

inline
static
void
_SetTriangle(int brickIndex,
             int triIndex,
             int32_t *vindices, 
             int32_t *mindices,
             int32_t *tindices, 
             int v0, int v1, int v2,
             int color,
             int t0, int t1, int t2)
{
    int vindex = brickIndex * 12 * 4 + triIndex * 4;
    int mindex = brickIndex * 12 + triIndex;
    int tindex = brickIndex * 12 * 4 + triIndex * 4;

    vindices[vindex++] = v0;
    vindices[vindex++] = v1;
    vindices[vindex++] = v2;
    vindices[vindex++] = -1;

    mindices[mindex++] = color;

    tindices[tindex++] = t0;
    tindices[tindex++] = t1;
    tindices[tindex++] = t2;
    tindices[tindex++] = -1;
}

void
LegoApp::_AddBrick(LegoBrick *brick,
                   uint32_t brickIndex,
                   SbVec3f *coords,
                   uint32_t *colors,
                   int32_t *vindices,
                   int32_t *mindices,
                   int32_t *tindices,
                   bool selected)
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

    _SetBrickVertex(brickIndex, 0, coords, SbVec3f(xStart,
                                                   yStart,
                                                   zStart + zFaceSize));
    _SetBrickVertex(brickIndex, 1, coords, SbVec3f(xStart + xFaceSize,
                                                   yStart,
                                                   zStart + zFaceSize));
    _SetBrickVertex(brickIndex, 2, coords, SbVec3f(xStart,
                                                   yStart + yFaceSize,
                                                   zStart + zFaceSize));
    _SetBrickVertex(brickIndex, 3, coords, SbVec3f(xStart + xFaceSize,
                                                   yStart + yFaceSize,
                                                   zStart + zFaceSize));
    _SetBrickVertex(brickIndex, 4, coords, SbVec3f(xStart,
                                                   yStart,
                                                   zStart));
    _SetBrickVertex(brickIndex, 5, coords, SbVec3f(xStart + xFaceSize,
                                                   yStart,
                                                   zStart));
    _SetBrickVertex(brickIndex, 6, coords, SbVec3f(xStart,
                                                   yStart + yFaceSize,
                                                   zStart));
    _SetBrickVertex(brickIndex, 7, coords, SbVec3f(xStart + xFaceSize,
                                                   yStart + yFaceSize,
                                                   zStart));

    int matIndex = brickIndex;
    if (selected) {
        SbVec3f highlightColor(0, 1, 1);
        _SetBrickColor(brickIndex, colors, highlightColor);
    } else {
        const MfVec3f &color = brick->GetColor();
        _SetBrickColor(brickIndex, colors, SbVec3f(color[0], color[1], color[2]));
    }

    int sv = brickIndex * 8;
    _SetTriangle(brickIndex,  0, vindices, mindices, tindices, sv + 0, sv + 1, sv + 2, matIndex, 0, 1, 2);
    _SetTriangle(brickIndex,  1, vindices, mindices, tindices, sv + 2, sv + 1, sv + 3, matIndex, 2, 1, 3);
    _SetTriangle(brickIndex,  2, vindices, mindices, tindices, sv + 5, sv + 4, sv + 7, matIndex, 0, 0, 0);
    _SetTriangle(brickIndex,  3, vindices, mindices, tindices, sv + 7, sv + 4, sv + 6, matIndex, 0, 0, 0);
    _SetTriangle(brickIndex,  4, vindices, mindices, tindices, sv + 4, sv + 0, sv + 6, matIndex, 0, 0, 0);
    _SetTriangle(brickIndex,  5, vindices, mindices, tindices, sv + 6, sv + 0, sv + 2, matIndex, 0, 0, 0);
    _SetTriangle(brickIndex,  6, vindices, mindices, tindices, sv + 1, sv + 5, sv + 3, matIndex, 0, 0, 0);
    _SetTriangle(brickIndex,  7, vindices, mindices, tindices, sv + 3, sv + 5, sv + 7, matIndex, 0, 0, 0);
    _SetTriangle(brickIndex,  8, vindices, mindices, tindices, sv + 2, sv + 3, sv + 6, matIndex, 0, 0, 0);
    _SetTriangle(brickIndex,  9, vindices, mindices, tindices, sv + 6, sv + 3, sv + 7, matIndex, 0, 0, 0);
    _SetTriangle(brickIndex, 10, vindices, mindices, tindices, sv + 4, sv + 5, sv + 0, matIndex, 0, 0, 0);
    _SetTriangle(brickIndex, 11, vindices, mindices, tindices, sv + 0, sv + 5, sv + 1, matIndex, 0, 0, 0);
}

void
LegoApp::_ProcessLegoBricksChangedNotice(const LegoBricksChangedNotice &)
{
    _BuildBricks();
}

void
LegoApp::_BuildBricks()
{
    if (_brickVP->texCoord.getNum() == 0) {
        _brickVP->texCoord.setNum(4);
        SbVec2f *texCoords = _brickVP->texCoord.startEditing();
        texCoords[0] =  SbVec2f(0, 0);
        texCoords[1] =  SbVec2f(1, 0);
        texCoords[2] =  SbVec2f(0, 1);
        texCoords[3] =  SbVec2f(1, 1);
        _brickVP->texCoord.finishEditing();
    }

    const auto &bricks = _universe->GetBricks();
    const auto &selection = _universe->GetSelection();

    int numBricks = bricks.size();
    int numVertexIndices = 12 * 4 * numBricks;
    int numTriangleIndices = 12 * numBricks;

    _brickVP->vertex.setNum(8 * numBricks);
    SbVec3f *coords = _brickVP->vertex.startEditing();

    _brickVP->orderedRGBA.setNum(numBricks);
    uint32_t *colors = _brickVP->orderedRGBA.startEditing();

    _brickIFS->coordIndex.setNum(numVertexIndices);
    int32_t *indices = _brickIFS->coordIndex.startEditing();

    _brickIFS->materialIndex.setNum(numTriangleIndices);
    int32_t *matIndices = _brickIFS->materialIndex.startEditing();

    _brickIFS->textureCoordIndex.setNum(numVertexIndices);
    int32_t *texIndices = _brickIFS->textureCoordIndex.startEditing();

    for (uint32_t i = 0; i < bricks.size(); ++i) {
        auto *brick = bricks[i];
        bool selected = selection.find(brick->GetID()) != selection.end();
        _AddBrick(brick, i, coords, colors, indices, matIndices, texIndices, selected);
    }

    _brickVP->vertex.finishEditing();
    _brickVP->orderedRGBA.finishEditing();
    _brickIFS->coordIndex.finishEditing();
    _brickIFS->textureCoordIndex.finishEditing();
}

const std::vector<LegoTransaction> &
LegoApp::GetTransactionLog()
{
    return _universe->GetTransactionMgr()->GetLog();
}

void
LegoApp::DumpScenegraph()
{
    SoOutput output;
    output.openFile("lego.iv");
    SoWriteAction wa(&output);
    wa.apply(_sceneRoot);
    output.closeFile();
}

void
LegoApp::ImportModels(const std::vector<std::string> &modelFileNames)
{
    for (const auto &modelPath : modelFileNames) {
        GfSTLReader stlReader;
        GfTriangleMesh *mesh = stlReader.Read(modelPath);
        if (mesh) {
            LegoVoxelizer voxelizer(_universe);
            voxelizer.Voxelize(mesh, 64, _worldMin);
        }
    }
}

void
LegoApp::PollServer()
{
    _universe->CatchupWithServer();
}

void 
LegoApp::SetNetworkEnabled(bool enabled)
{
    _universe->SetNetworkEnabled(enabled);
}

bool
LegoApp::IsNetworkEnabled() const
{
    return _universe->IsNetworkEnabled();
}

void
LegoApp::_ProcessLegoConflictNotice(const LegoConflictNotice &)
{
    if (_flashAlarm) {
        return;
    }

    // Flash screen
    _flash = true;
    _sceneRoot->touch();

    // Schedule alarm to turn off flashing
    _flashAlarm = new SoAlarmSensor(&LegoApp::_ToggleFlash, this);
    _flashAlarm->setTimeFromNow(SbTime(0.5));
    _flashAlarm->schedule();
}

void
LegoApp::_ToggleFlash(void *userData, SoSensor *sensor)
{
    LegoApp *This = reinterpret_cast<LegoApp*>(userData);
    This->_flash = false;
    This->_sceneRoot->touch();
    delete This->_flashAlarm; This->_flashAlarm = NULL;
}

void
LegoApp::_EventCB(void *userData, SoEventCallback *eventCB)
{
    LegoApp *This = reinterpret_cast<LegoApp*>(userData);

    const SoEvent *event = eventCB->getEvent();
    if (SO_KEY_PRESS_EVENT(event, ANY)) {
        const SoKeyboardEvent *keyEvent = 
            dynamic_cast<const SoKeyboardEvent*>(event);
        SoKeyboardEvent::Key key = keyEvent->getKey();
        switch (key) {
        case SoKeyboardEvent::NUMBER_1:
        case SoKeyboardEvent::NUMBER_2:
        case SoKeyboardEvent::NUMBER_3:
        case SoKeyboardEvent::NUMBER_4:
        case SoKeyboardEvent::NUMBER_5: {
            LegoUniverse::Color brickColor = 
                LegoUniverse::Color(key - SoKeyboardEvent::NUMBER_1);
            This->_universe->ModifyColorForSelectedBricks(brickColor);
            This->_universe->ClearSelection();
            LegoBricksChangedNotice().Send();
        } break;
        case SoKeyboardEvent::LEFT_SHIFT:
        case SoKeyboardEvent::RIGHT_SHIFT:
            This->_shiftDown = true;
            break;
        case SoKeyboardEvent::LEFT_CONTROL:
        case SoKeyboardEvent::RIGHT_CONTROL:
        case SoKeyboardEvent::LEFT_ALT:
        case SoKeyboardEvent::RIGHT_ALT:
            This->_ctrlDown = true;
            break;
        default:
            break;
        }
        eventCB->setHandled();
    } else if (SO_KEY_RELEASE_EVENT(event, ANY)) {
        const SoKeyboardEvent *keyEvent = 
            dynamic_cast<const SoKeyboardEvent*>(event);
        SoKeyboardEvent::Key key = keyEvent->getKey();
        switch (key) {
        case SoKeyboardEvent::LEFT_SHIFT:
        case SoKeyboardEvent::RIGHT_SHIFT:
            This->_shiftDown = false;
            break;
        case SoKeyboardEvent::LEFT_CONTROL:
        case SoKeyboardEvent::RIGHT_CONTROL:
        case SoKeyboardEvent::LEFT_ALT:
        case SoKeyboardEvent::RIGHT_ALT:
            This->_ctrlDown = false;
            break;
        default:
            break;
        }
    } else if (SO_MOUSE_PRESS_EVENT(event, BUTTON1)) {
        const SoPickedPoint *pickedPoint = eventCB->getPickedPoint();

        if (This->_shiftDown) {
            // Create brick
            This->_HandleCreate(pickedPoint);
        } else if (This->_ctrlDown) {
            // Delete brick
            This->_HandleDelete(pickedPoint);
        } else {
            // Select brick
            This->_HandleSelect(pickedPoint);
        }

        eventCB->setHandled();
    } else if (SO_MOUSE_PRESS_EVENT(event, BUTTON2)) {
        eventCB->setHandled();
    } else if (SO_MOUSE_PRESS_EVENT(event, BUTTON3)) {
        eventCB->setHandled();
    }
}

static
MfVec3d
_GetPickCenter(const SoPickedPoint *pickedPoint)
{
    const SbVec3f &point = pickedPoint->getPoint();
    SbVec3f normal = pickedPoint->getNormal();

    normal.normalize();
    MfVec3d centerPoint = 
        MfVec3d(point[0], point[1], point[2]) - 
        MfVec3d(normal[0], normal[1], normal[2]) * 0.5;
    return centerPoint;
}

void
LegoApp::_HandleSelect(const SoPickedPoint *pickedPoint)
{
    if (pickedPoint) {
        MfVec3d centerPoint = _GetPickCenter(pickedPoint);
        if (centerPoint[2] < 0.0) {
            _universe->ClearSelection();
        } else {
            LegoBrick *brick = _universe->GetBrick(centerPoint);
            _universe->Select(brick);
        }
    } else {
        _universe->ClearSelection();
    }

    LegoBricksChangedNotice().Send();
}

void
LegoApp::_HandleCreate(const SoPickedPoint *pickedPoint)
{
    if (!pickedPoint) {
        return;
    }

    MfVec3i position;

    MfVec3d centerPoint = _GetPickCenter(pickedPoint);
    if (centerPoint[2] < 0.0) {
        // Add on ground plane
        position = MfVec3i(centerPoint[0] - 1, centerPoint[1] - 1, 0);
    } else {
        LegoBrick *brick = _universe->GetBrick(centerPoint);
        assert(brick);
        position = brick->GetPosition() + MfVec3i(0, 0, 1);
    }

    LegoUniverse::Color colorIndex = 
        (LegoUniverse::Color) (drand48() * LegoUniverse::NUM_COLORS);
    MfVec3f brickColor = LegoUniverse::COLORS[colorIndex];
    _universe->CreateBrick(position, MfVec3i(2, 2, 1), LegoBrick::EAST, brickColor);

    LegoBricksChangedNotice().Send();
}

void
LegoApp::_HandleDelete(const SoPickedPoint *pickedPoint)
{
    if (!pickedPoint) {
        return;
    }

    MfVec3i position;

    MfVec3d centerPoint = _GetPickCenter(pickedPoint);
    if (centerPoint[2] < 0.0) {
        // Ground plane
        return;
    }

    LegoBrick *brick = _universe->GetBrick(centerPoint);
    assert(brick);
    brick->Destroy();

    LegoBricksChangedNotice().Send();
}
