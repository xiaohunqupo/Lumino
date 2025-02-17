﻿
#include "Internal.hpp"
#include <LuminoEngine/Base/Serializer.hpp>
#include <LuminoEngine/Base/Task.hpp>
#include <LuminoEngine/Engine/Diagnostics.hpp>
#include <LuminoEngine/Engine/Application.hpp>
#include <LuminoEngine/Engine/Debug.hpp>
#include <LuminoGraphics/GPU/GraphicsCommandBuffer.hpp>
#include <LuminoEngine/UI/UIContext.hpp>
#include <LuminoEngine/UI/UIFrameWindow.hpp>
#include <LuminoEngine/UI/UIViewport.hpp>
#include <LuminoEngine/UI/UIRenderView.hpp>
#include <LuminoEngine/UI/UIContainerElement.hpp>
#include <LuminoEngine/Physics/PhysicsWorld.hpp>
#include <LuminoEngine/Physics/PhysicsWorld2D.hpp>
#include <LuminoEngine/Scene/World.hpp>
#include <LuminoEngine/Scene/Level.hpp>
#include <LuminoEngine/Scene/WorldRenderView.hpp>
#include <LuminoEngine/Scene/Light.hpp>
#include <LuminoEngine/Scene/Camera.hpp>
#include <LuminoEngine/Scene/CameraOrbitControlComponent.hpp>
//#include "../Graphics/RenderTargetTextureCache.hpp"

#include <LuminoEngine/Runtime/detail/RuntimeManager.hpp>
#include <LuminoPlatform/detail/PlatformManager.hpp>
#include <LuminoPlatform/PlatformWindow.hpp>
#include "../../Graphics/src/Animation/AnimationManager.hpp"
#include "../Input/InputManager.hpp"
#include "../Audio/AudioManager.hpp"
#include <LuminoGraphics/detail/GraphicsManager.hpp>
#include <LuminoGraphics/Font/detail/FontManager.hpp>
#include "../../../Graphics/src/Mesh/MeshManager.hpp"
#include <LuminoGraphics/Rendering/detail/RenderingManager.hpp>
#include "../../Graphics/src/Rendering/RenderingProfiler.hpp"
#include "../Effect/EffectManager.hpp"
#include "../Physics/PhysicsManager.hpp"
#include <LuminoEngine/Asset/detail/AssetManager.hpp>
#include "../Visual/VisualManager.hpp"
#include "../Scene/SceneManager.hpp"
#include "../UI/UIManager.hpp"
#include "../Editor/DevelopmentTool.hpp"
#include "EngineManager.hpp"
#include "EngineDomain.hpp"
#include "EngineProfiler.hpp"

#include <LuminoEngine/Scene/SceneConductor.hpp>

namespace ln {
namespace detail {

//==============================================================================
// EngineSettings

// void EngineSettings::setMainWindowSize(int width, int height)
//{
//	detail::EngineSettings::instance.mainWindowSize.set(width, height);
// }
//
// void EngineSettings::setMainBackBufferSize(int width, int height)
//{
//	detail::EngineSettings::instance.mainBackBufferSize.set(width, height);
// }
//
// void EngineSettings::setMainWindowTitle(const StringView& title)
//{
//	detail::EngineSettings::instance.mainWindowTitle = title;
// }

//==============================================================================
// EngineManager

EngineSettings EngineManager::s_settings;

EngineManager::EngineManager()
    : m_settings()
    //, m_animationManager(nullptr)
    //, m_inputManager(nullptr)
    , m_audioManager(nullptr)
    //, m_physicsManager(nullptr)
    //, m_graphicsManager(nullptr)
    //, m_effectManager(nullptr)
    //, m_modelManager(nullptr)
    //, m_uiManager(nullptr)
    , m_timeScale(1.0f)
    , m_exitRequested(false)
    //   , m_showDebugFpsEnabled(false)
    //, m_debugToolEnabled(false)
    , m_debugToolMode(DebugToolMode::Disable)
    , m_engineProfiler(std::make_unique<EngineProfiler>())
#if defined(LN_OS_WIN32)
    , m_comInitialized(false)
    , m_oleInitialized(false)
#endif
{
}

EngineManager::~EngineManager() {
}

void EngineManager::init(const EngineSettings& settings) {
    LN_LOG_DEBUG("EngineManager Initialization started.");

    m_settings = settings;
    m_settings.tryLoad();

    {
        RuntimeModuleSettings settings2;
        settings2.assetStorageAccessPriority = m_settings.assetStorageAccessPriority;
        if (!EngineContext2::initialize(settings2)) return;

        for (auto& e : m_settings.assetArchives) {
            AssetManager::instance()->mountAssetArchive(e.filePath, e.password);
        }
        for (auto& e : m_settings.assetDirectories) {
            AssetManager::instance()->addAssetDirectory(e);
        }
    }



    // check settings
    {
        if (m_settings.bundleIdentifier.isEmpty()) {
            m_settings.bundleIdentifier = _TT("lumino");
        }

#if defined(LN_OS_IOS)
        if (m_settings.bundleIdentifier.contains(_TT("lumino"), CaseSensitivity::CaseInsensitive)) {
            LN_WARNING(_TT("Bundle Identifier It may not be set."));
        }
#endif
    }

    // setup pathes
    {
#if defined(LN_OS_WIN32)
        m_persistentDataPath = Path(Environment::specialFolderPath(SpecialFolder::ApplicationData), m_settings.bundleIdentifier);
#else
        m_persistentDataPath = _TT(""); // TODO:
#endif
        if (!m_settings.engineResourcesPath.isEmpty()) {
            m_engineResourcesPath = Path(m_settings.engineResourcesPath).canonicalize();
        }

        // Find folder in install.
        if (m_settings.engineResourcesPath.isEmpty()) {
            if (auto path = ln::Environment::getEnvironmentVariable(_TT("LUMINO_PATH"))) {
                auto dir = Path::combine(*path, _TT("Tools"), _TT("EngineResources"));
                if (FileSystem::existsDirectory(dir)) {
                    m_engineResourcesPath = dir;
                }
            }
        }

        // Find folder in repository.
        if (m_engineResourcesPath.isEmpty()) {
            auto repo = ln::detail::EngineManager::findRepositoryRootForTesting();
            m_engineResourcesPath = Path::combine(repo, _TT("tools"), _TT("EngineResources"));
        }

        LN_LOG_INFO(U"EngineResourcesPath: {}", m_engineResourcesPath.str());
    }

    // register types
    {
        auto* context = RuntimeContext::current();
        context->registerType<Application>();
    }

    m_fpsController.setFrameRate(m_settings.frameRate);
    m_fpsController.setMeasurementEnabled(true);

    LN_LOG_DEBUG("EngineManager Initialization ended.");
}

void EngineManager::dispose() {
    LN_LOG_DEBUG("EngineManager finalization started.");

    if (m_debugWorldRenderView) {
        m_mainViewport->removeRenderView(m_debugWorldRenderView);
        m_debugWorldRenderView = nullptr;
    }
    m_debugCamera = nullptr;

    if (m_runtimeEditor) {
        m_runtimeEditor->dispose();
        m_runtimeEditor = nullptr;
    }

    if (m_uiManager) {
        m_uiManager->setPrimaryElement(nullptr);
        // m_uiManager->setMainContext(nullptr);
    }

    if (PlatformManager::instance()) {
        PlatformManager::instance()->mainWindow()->detachEventListener(this);
    }

    m_mainPhysicsWorld = nullptr;
    m_mainPhysicsWorld2D = nullptr;

    // Main world contents.
    {
        if (m_mainWorldRenderView) {
            m_mainWorldRenderView->dispose();
            m_mainWorldRenderView = nullptr;
        }
        if (m_mainCamera) {
            m_mainCamera->dispose();
            m_mainCamera = nullptr;
        }
        if (m_mainWorld) {
            m_mainWorld->dispose();
            m_mainWorld = nullptr;
        }
    }

    // Main UI contents.
    {
        if (m_mainUIRoot) {
            m_mainUIRoot->dispose();
            m_mainUIRoot = nullptr;
        }
        if (m_mainUIRenderView) {
            m_mainUIRenderView->dispose();
            m_mainUIRenderView = nullptr;
        }
        if (m_mainViewport) {
            m_mainViewport->dispose();
            m_mainViewport = nullptr;
        }
        if (m_mainWindow) {
            m_mainWindow->dispose();
            m_mainWindow = nullptr;
        }
        // if (m_mainUIContext) {
        //     m_mainUIContext->dispose();
        //     m_mainUIContext = nullptr;
        // }
    }

    // if (!m_settings.externalRenderingManagement) {
    //     if (m_graphicsManager) {
    //         m_graphicsManager->leaveRendering();
    //     }
    // }

    if (m_uiManager) m_uiManager->dispose();
    if (m_sceneManager) m_sceneManager->dispose();
    if (m_visualManager) m_visualManager->dispose();
    if (m_physicsManager) m_physicsManager->dispose();
    if (m_effectManager) m_effectManager->dispose();
    RenderingManager::terminate();
    FontManager::terminate();
    GraphicsManager::terminate();
    if (m_audioManager) m_audioManager->dispose();
    if (m_inputManager) m_inputManager->dispose();
    AnimationManager::terminate();
    PlatformManager::terminate();

#if defined(LN_OS_WIN32)
    if (m_oleInitialized) {
        ::OleUninitialize();
        m_oleInitialized = false;
    }

    if (m_comInitialized) {
        ::CoUninitialize();
        m_comInitialized = false;
    }
#endif

    if (m_appData) {
        m_appData->attemptSave();
        m_appData = nullptr;
    }

    RuntimeManager::terminate();
    EngineContext2::terminate();
    LN_LOG_DEBUG("EngineManager finalization ended.");
}

void EngineManager::initializeAllManagers() {
    initializeCommon();
    initializePlatformManager();
    initializeAnimationManager();
    initializeInputManager();
    initializeAudioManager();
    initializeGraphicsManager();
    initializeFontManager();
    initializePhysicsManager();
    initializeRenderingManager();
    initializeEffectManager();
    initializeVisualManager();
    initializeSceneManager();
    initializeUIManager();
}

void EngineManager::initializeCommon() {
    if (!m_commonInitialized) {
        LN_LOG_DEBUG("EngineManager common initialization started.");

#if defined(LN_OS_DESKTOP)
        {
            if (m_settings.debugMode) {
                // engineLogFilePath 未指定の場合、スクリプト系言語だとそのランタイム実行ファイルを指してしまうので、カレントディレクトリに出力するようにする。
                auto logfile = (m_settings.engineLogFilePath.isEmpty()) ? Path(_TT("lumino.log")) : Path(m_settings.engineLogFilePath);
                Logger::addFileAdapter(logfile.str().toStdString());
                ln::Console::allocate();
                ln::Logger::addStdErrAdapter();

                if (Logger::level() > LogLevel::Debug) {
                    Logger::setLevel(LogLevel::Debug);
                }
            }

            m_appData = makeRef<AppDataInternal>();
            m_appData->attemptLoad();
        }
#endif

#if defined(LN_OS_WIN32)

        // CoInitializeEx は ShowWindow() ～ DestroyWindow() の外側で呼び出さなければならない。
        // http://blog.techlab-xe.net/archives/400
        // 例えば ウィンドウ作成→DirectInput初期化みたいにするとき、Input モジュールの中で CoInitializeEx しているとこの罠にはまる。
        // とりあえず、Platform モジュールでは COM は使わないが、他のモジュールとの連携に備え、初期化しておく。

        if (m_settings.autoCoInitialize && SUCCEEDED(::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED))) {
            // エラーにはしない。別の設定で COM が初期化済みだったりすると失敗することがあるが、COM 自体は使えるようになっている
            m_comInitialized = true;
        }

        // OleInitialize() するためには、CoInitializeEx() が STA(COINIT_APARTMENTTHREADED) で初期化されている必要がある
        if (SUCCEEDED(::OleInitialize(NULL))) {
            m_oleInitialized = true;
        }
#endif

#ifdef __EMSCRIPTEN_PTHREADS__
#else
        LN_LOG_ERROR("__EMSCRIPTEN_PTHREADS__ disabled.");
#endif

        resolveActiveGraphicsAPI();

        m_commonInitialized = true;

        LN_LOG_DEBUG("EngineManager common initialization ended.");
    }
}

void EngineManager::initializePlatformManager() {
    if (!PlatformManager::instance() && m_settings.features.hasFlag(EngineFeature::Application)) {
        initializeCommon();

        PlatformManager::Settings settings;
        settings.windowSystem = m_settings.windowSystem;

        if (settings.windowSystem == WindowSystem::GLFWWithOpenGL) {
            if (m_activeGraphicsAPI == GraphicsAPI::Vulkan ||
                m_activeGraphicsAPI == GraphicsAPI::DirectX12) {
                settings.windowSystem = WindowSystem::GLFWWithoutOpenGL;
            }
        }

        settings.mainWindowSettings.title = m_settings.mainWindowTitle;
        settings.mainWindowSettings.clientWidth = m_settings.mainWindowSize.width;
        settings.mainWindowSettings.clientHeight = m_settings.mainWindowSize.height;
        settings.mainWindowSettings.fullscreen = false;
        settings.mainWindowSettings.resizable = m_settings.mainWindowResizable;
        settings.mainWindowSettings.userWindow = m_settings.userMainWindow;

        PlatformManager::initialize(settings);

        //auto mainWindow = PlatformManager::instance()->createWindow(mainWindowSettings);
        //PlatformManager::instance()->setMainWindow(mainWindow);
        PlatformManager::instance()->mainWindow()->attachEventListener(this);
    }
}

void EngineManager::initializeAnimationManager() {
    if (!AnimationManager::instance() && m_settings.features.hasFlag(EngineFeature::Rendering)) {

        AnimationManager::Settings settings;
        settings.assetManager = AssetManager::instance();
        AnimationManager::initialize(settings);
    }
}

void EngineManager::initializeInputManager() {
    if (!m_inputManager && m_settings.features.hasFlag(EngineFeature::Input)) {
        initializePlatformManager();

        InputManager::Settings settings;
        settings.mainWindow = PlatformManager::instance()->mainWindow();
        settings.inputConfig = m_settings.inputConfig;
        m_inputManager = ln::makeRef<InputManager>();
        m_inputManager->init(settings);
    }
}

void EngineManager::initializeAudioManager() {
    if (!m_audioManager && m_settings.features.hasFlag(EngineFeature::Audio)) {
        initializeCommon();

        AudioManager::Settings settings;
        settings.assetManager = AssetManager::instance();

        m_audioManager = ln::makeRef<AudioManager>();
        m_audioManager->init(settings);
    }
}

void EngineManager::initializeFontManager() {
    if (!FontManager::instance() && m_settings.features.hasFlag(EngineFeature::Graphics)) {

        FontManager::Settings settings;
        settings.assetManager = AssetManager::instance();
        settings.engineAssetPath = m_engineResourcesPath;
        settings.fontFile = m_settings.fontFile;

        FontManager::initialize(settings);
    }
}

void EngineManager::initializeGraphicsManager() {
    if (!GraphicsManager::instance() && m_settings.features.hasFlag(EngineFeature::Graphics)) {
        initializePlatformManager();

        GraphicsManager::Settings settings;
        settings.graphicsAPI = m_activeGraphicsAPI;
        settings.priorityGPUName = m_settings.priorityGPUName;
        settings.debugMode = m_settings.graphicsDebugEnabled;

        GraphicsManager::initialize(settings);
    }
}

void EngineManager::initializeRenderingManager() {
    if (!RenderingManager::instance() && m_settings.features.hasFlag(EngineFeature::Rendering)) {
        initializeGraphicsManager();

        RenderingManager::Settings settings;
        settings.graphicsManager = GraphicsManager::instance();
        settings.fontManager = FontManager::instance();
        RenderingManager::initialize(settings);
    }
}

void EngineManager::initializeEffectManager() {
    if (!m_effectManager && m_settings.features.hasFlag(EngineFeature::Rendering)) {
        initializeGraphicsManager();
        initializeRenderingManager();

        EffectManager::Settings settings;
        settings.graphicsManager = GraphicsManager::instance();
        settings.assetManager = AssetManager::instance();
        settings.renderingManager = RenderingManager::instance();

        m_effectManager = ln::makeRef<EffectManager>();
        m_effectManager->init(settings);
    }
}

void EngineManager::initializePhysicsManager() {
    if (!m_physicsManager && m_settings.features.hasFlag(EngineFeature::Rendering)) {
        PhysicsManager::Settings settings;

        m_physicsManager = ln::makeRef<PhysicsManager>();
        m_physicsManager->init(settings);
    }
}

void EngineManager::initializeVisualManager() {
    if (!m_visualManager && m_settings.features.hasFlag(EngineFeature::Rendering)) {
        initializeGraphicsManager();

        VisualManager::Settings settings;
        settings.graphicsManager = GraphicsManager::instance();

        m_visualManager = ln::makeRef<VisualManager>();
        m_visualManager->init(settings);
    }
}

void EngineManager::initializeSceneManager() {
    if (!m_sceneManager && m_settings.features.hasFlag(EngineFeature::Rendering)) {
        initializeAnimationManager();

        m_sceneManager = ln::makeRef<SceneManager>();
        m_sceneManager->init();

        //m_animationManager->setSceneManager(m_sceneManager);
    }
}

void EngineManager::initializeUIManager() {
    if (!m_uiManager && m_settings.features.hasFlag(EngineFeature::Application)) {
        initializeGraphicsManager();

        UIManager::Settings settings;
        settings.graphicsManager = GraphicsManager::instance();
        settings.defaultThemeName = m_settings.defaultUITheme;

        m_uiManager = makeRef<UIManager>();
        m_uiManager->init(settings);

        // m_mainUIContext = makeObject_deprecated<UIContext>();
        // m_uiManager->setMainContext(m_mainUIContext);

        if (m_settings.debugToolEnabled) {
            setDebugToolMode(DebugToolMode::Minimalized);
        }
        else {
            setDebugToolMode(DebugToolMode::Disable);
        }
    }
}

void EngineManager::craeteDefaultObjectsIfNeeded(UIMainWindow* userMainWindow) {
    if (m_settings.defaultObjectsCreation) {
        if (userMainWindow) {
            setupMainWindow(userMainWindow, true);
        }
        else {
            setupMainWindow(makeObject_deprecated<UIMainWindow>(m_settings.useExternalSwapChain), true);
        }
    }
}

bool EngineManager::updateUnitily() {
    updateFrame();
    presentFrame(nullptr, nullptr);
    return !isExitRequested();
}

void EngineManager::updateFrame() {
    if (m_engineProfiler) {
        m_engineProfiler->beginFrame(m_settings.frameRate);
        m_engineProfiler->lapBeginUpdate();
    }

    if (preUpdateCallback) {
        preUpdateCallback();
    }

    float elapsedSeconds = 0.016 * m_timeScale; // TODO: time

    //------------------------------------------------
    // Pre update phase
    //   このフレームのウィンドウサイズ (バックバッファサイズ) はこのフェーズで確定させる

    if (m_inputManager) {
        m_inputManager->preUpdateFrame();
    }

    if (PlatformManager::instance()) {
        PlatformManager::instance()->processSystemEventQueue();
    }
    if (m_uiManager) {
        m_uiManager->dispatchPostedEvents();
    }

    const auto& mainThreadTaskDispatcher = EngineContext2::instance()->mainThreadTaskDispatcher();
    if (mainThreadTaskDispatcher) {
        mainThreadTaskDispatcher->executeTasks(1);
    }

    //------------------------------------------------
    // Main update phase

    if (m_debugInterface) {
        m_debugInterface->update(elapsedSeconds);
    }

    // いくつかの入力状態は onEvent 経由で Platform モジュールから Input モジュールに伝えられる。
    // このときはまだ押されているかどうかだけを覚えておく。
    // 次に InputManager::updateFrame で、現在時間を考慮して各種状態を更新する。
    if (m_inputManager) {
        m_inputManager->updateFrame(elapsedSeconds);
    }

    if (m_audioManager) {
        m_audioManager->update(elapsedSeconds);
    }

    // Application::onUpdate の呼び出しはここから。
    if (m_uiManager) {
        m_uiManager->updateFrame(elapsedSeconds);
    }

    if (m_mainWorld) {
        m_mainWorld->updateFrame(elapsedSeconds);
    }

    if (m_sceneManager) {
        m_sceneManager->updateFrame();
    }

    //------------------------------------------------
    // Post update phase

    if (m_engineProfiler) {
        m_engineProfiler->lapEndUpdate();
    }
}

void EngineManager::presentFrame(GraphicsCommandList* commandList, RenderTargetTexture* renderTarget) {
    // 大体他の updateFrame で要素位置の調整が入るので、その後にレイアウトする。
    if (m_mainWindow) {
        if (m_engineProfiler) m_engineProfiler->lapBeginUIUpdate();

        // onUpdate のユーザー処理として、2D <-> 3D 変換したいことがあるが、それには ViewPixelSize が必要になる。
        // 初期化直後や、Platform からの SizeChanged イベントの直後に一度レイアウトを更新することで、
        // ユーザー処理の前に正しい ViewPixelSize を計算しておく。
        m_mainWindow->updateLayoutIfNeeded();

        if (m_engineProfiler) m_engineProfiler->lapEndUIUpdate();
    }

    if (m_runtimeEditor) {
        m_runtimeEditor->updateFrame();
    }

    // m_effectManager->testDraw();

    if (m_engineProfiler) {
        m_engineProfiler->lapBeginRendering();
    }

    if (m_mainWindow) {
        if (commandList) {
            m_mainWindow->renderContents(commandList, renderTarget);
        }
        else {
            m_mainWindow->present();
        }
    }

    // if (m_renderingManager) {
    //     m_renderingManager->frameBufferCache()->endRenderSection();
    // }

    if (m_engineProfiler) {
        m_engineProfiler->lapEndRendering();
        m_engineProfiler->endFrame();
    }

    if (auto* m = RenderingManager::instance()) {
        m->profiler()->reset();
    }

    //
    //--------------------

    if (m_settings.standaloneFpsControl) {
        m_fpsController.process();
    }
    else {
        m_fpsController.processForMeasure();
    }

    if (m_debugToolMode == DebugToolMode::Minimalized || m_debugToolMode == DebugToolMode::Activated) {
        PlatformManager::instance()->mainWindow()->setWindowTitle(ln::format(_TT("FPS:{0:.1f}({1:.1f}), F8:Debug tool."), m_fpsController.totalFps(), m_fpsController.externalFps()));
    }

    // TODO: Editor モードの時にも呼び出せるようにしないとだめそう
    if (auto* meshManager = MeshManager::instance()) {
        meshManager->collectUnreferenceObjects();
    }
}

void EngineManager::resetFrameDelay() {
    m_fpsController.refreshSystemDelay();
}

void EngineManager::quit() {
    m_exitRequested = true;
}

int EngineManager::runApplication() {
    Application* app = application();
	
    while (isExitRequested()) {
        updateFrame();
        presentFrame(nullptr, nullptr);
    }

    return 0;
}

ln::Path EngineManager::findRepositoryRootForTesting() {
    return findParentDirectoryContainingSpecifiedFile(_TT("build.csproj"));
}

ln::Path EngineManager::findParentDirectoryContainingSpecifiedFile(StringView file) {
    ln::Path path = ln::Path(ln::Environment::executablePath()).parent();
    ln::Path luminoRepoRoot;
    while (!path.isRoot()) {
        auto files = ln::FileSystem::getFiles(path, file);
        if (!files.isEmpty()) {
            luminoRepoRoot = path;
            break;
        }
        // if (ln::FileSystem::existsFile(ln::Path(path, file))) {
        //	luminoRepoRoot = path;
        //	break;
        // }
        path = path.parent();
    }
    return luminoRepoRoot;
}

const Path& EngineManager::persistentDataPath() const {
#if !defined(LN_OS_WIN32)
    LN_NOTIMPLEMENTED();
#endif
    return m_persistentDataPath;
}

void EngineManager::setupMainWindow(UIMainWindow* window, bool createBasicObjects) {
    if (LN_REQUIRE(!m_mainWindow)) return;

	if (window) {
        m_mainWindow = window;
    }
    else {
        m_mainWindow = *makeObject<UIMainWindow>();
    }

    // m_mainWindow->setupPlatformWindow(m_platformManager->mainWindow(), m_settings.mainWindowSize);
    // m_mainUIContext->setLayoutRootElement(m_mainWindow);

    // TODO: SwapChain だけでいいはず
    // m_mainWindow->m_graphicsContext = m_graphicsManager->mainWindowGraphicsContext();

    m_mainWindow->m_onImGuiLayer.connect(ln::bind(this, &EngineManager::handleImGuiDebugLayer));

    if (createBasicObjects) {
        if (m_uiManager) {
            m_mainViewport = makeObject_deprecated<UIViewport>();
            m_mainWindow->addElement(m_mainViewport);
        }

        if (m_sceneManager) {
            m_mainWorld = makeObject_deprecated<World>();
            m_sceneManager->setActiveWorld(m_mainWorld);

            m_mainScene = m_mainWorld->masterScene();

            m_mainCamera = makeObject_deprecated<Camera>();
            m_mainWorld->add(m_mainCamera);
            if (m_sceneManager) {
                m_sceneManager->setDefaultCamera(m_mainCamera);
            }

            if (m_settings.createMainLights) {

                // auto mainAmbientLight = makeObject_deprecated<AmbientLight>();
                // m_mainWorld->add(mainAmbientLight);
                // m_mainWorld->setMainAmbientLight(mainAmbientLight);

                // auto mainDirectionalLight = makeObject_deprecated<DirectionalLight>();
                // m_mainWorld->add(mainDirectionalLight);
                // m_mainWorld->setMainDirectionalLight(mainDirectionalLight);
            }

            m_mainWorldRenderView = makeObject_deprecated<WorldRenderView>();
            m_mainWorldRenderView->setTargetWorld(m_mainWorld);
            m_mainWorldRenderView->setCamera(m_mainCamera);
            m_mainWorldRenderView->setClearMode(SceneClearMode::ColorAndDepth);
            m_mainViewport->addRenderView(m_mainWorldRenderView);

            m_mainUIRenderView = makeObject_deprecated<UIRenderView>();
            m_mainViewport->addRenderView(m_mainUIRenderView);
            const SizeI& viewboxSize = (m_settings.mainWorldViewSize.isAnyZero()) ? m_settings.mainWindowSize : m_settings.mainWorldViewSize;
            m_mainViewport->setViewBoxSize(viewboxSize.toFloatSize());

            m_mainUIRoot = makeObject_deprecated<UIDomainProvidor>();
            m_mainUIRoot->setupNavigator();
            m_mainUIRoot->setAlignments(UIAlignment::Stretch);
            m_mainUIRoot->m_hitTestMode = detail::UIHitTestMode::InvisiblePanel; // main の WorldView 全体に覆いかぶせるように配置するので、false にしておかないと CameraControl などにイベントが行かなくなる
            m_mainUIRenderView->setRootElement(m_mainUIRoot);
            m_uiManager->setPrimaryElement(m_mainUIRoot);

            m_mainPhysicsWorld = m_mainWorld->physicsWorld();
            m_mainPhysicsWorld2D = m_mainWorld->physicsWorld2D();

            m_physicsManager->setActivePhysicsWorld(m_mainPhysicsWorld);
            m_physicsManager->setActivePhysicsWorld2D(m_mainPhysicsWorld2D);

            m_debugInterface = makeObject_deprecated<DebugInterface>();
            m_mainWindow->m_debugInterface = m_debugInterface;

            // m_debugCamera = makeObject_deprecated<Camera>();
            ////m_mainWorld->add(m_mainCamera);
            ////Ref<Camera> m_debugCamera;
            // m_debugWorldRenderView = makeObject_deprecated<WorldRenderView>();
            // m_debugWorldRenderView->setTargetWorld(m_mainWorld);
            // m_debugWorldRenderView->setCamera(m_debugCamera);
            // m_debugWorldRenderView->setClearMode(SceneClearMode::ColorAndDepth);
            // m_debugCamera->addComponent(makeObject_deprecated<CameraOrbitControlComponent>());
            // m_mainViewport->addRenderView(m_debugWorldRenderView);
            // m_debugCamera->setPosition(10, 10, -10);
            // m_debugCamera->lookAt(Vector3(0, 0, 0));
        }
    }

    // init 直後にウィンドウサイズを取得したり、Camera Matrix を計算するため、ViewSize を確定させる
    if (m_mainWindow) {
        m_mainWindow->updateLayoutIfNeeded();
    }

    if (m_settings.developmentToolsEnabled && m_mainWindow) {
        m_runtimeEditor = makeRef<detail::RuntimeEditor>();
        m_runtimeEditor->init(this, m_mainWindow);
    }
}

void EngineManager::setupApplication(Application* app) {
    // Reset Scene
    {
        m_mainWorld->removeAllObjects();
        m_mainUIRoot->removeAllChildren();
    }

    m_uiManager->resetApp(app);
}

const Ref<Application>& EngineManager::application() const {
    return m_uiManager->application();
}

void EngineManager::resolveActiveGraphicsAPI() {
    m_activeGraphicsAPI = m_settings.graphicsAPI;

    if (m_activeGraphicsAPI == GraphicsAPI::Default || m_activeGraphicsAPI == GraphicsAPI::Vulkan) {
        if (GraphicsManager::checkVulkanSupported()) {
            m_activeGraphicsAPI = GraphicsAPI::Vulkan;
        }
        else {
            m_activeGraphicsAPI = GraphicsAPI::OpenGL;
        }
    }
}

bool EngineManager::onPlatformEvent(const PlatformEventArgs& e) {
    switch (e.type) {
        case PlatformEventType::close:
            quit();
            break;
        case PlatformEventType::KeyDown:
            if (e.key.keyCode == Keys::F8) {
                // toggleDebugToolMode();
                if (m_runtimeEditor) {
                    m_runtimeEditor->toggleMode();
                }
            }
            break;
        default:
            break;
    }

    // 他のモジュールにイベントを渡した後、まだ処理されていなければ input へ回す
    if (m_inputManager) {
        m_inputManager->onEvent(e);
    }

    return false;
}

void EngineManager::handleImGuiDebugLayer(UIEventArgs* e) {
    // ImGui::Begin("Statistics");
    // ImGui::Text("FPS: %.2f, Ext: %.2f", m_fpsController.totalFps(), m_fpsController.externalFps());
    // ImGui::Text("Min: %.2f, Max: %.2f", m_fpsController.minTimePerSeconds(), m_fpsController.maxTimePerSeconds());
    // ImGui::Separator();

    // if (ImGui::CollapsingHeader("RenderView debug"))
    //{
    //	{
    //		bool check = m_mainWorldRenderView->guideGridEnabled();
    //		ImGui::Checkbox("Grid", &check);
    //		m_mainWorldRenderView->setGuideGridEnabled(check);
    //	}
    //	{
    //		bool check = m_mainWorldRenderView->physicsDebugDrawEnabled();
    //		ImGui::Checkbox("Physics", &check);
    //		m_mainWorldRenderView->setPhysicsDebugDrawEnabled(check);
    //	}
    // }

    // if (m_mainWorld) {
    //	//ImGui::BeginChild("Levels");
    //	Level* level = m_mainWorld->sceneConductor()->activeScene();
    //	ImGui::Text("ActiveScene"); ImGui::SameLine(150);
    //	if (ImGui::Button("Reload")) {
    //		level->reloadAsset();
    //	}
    //	if (ImGui::Button("Save")) {
    //		m_assetManager->saveAssetModelToLocalFile(makeObject_deprecated<AssetModel>(level));
    //	}

    //	//ImGui::EndChild();
    //}

    // ImGui::End();
}

bool EngineManager::toggleDebugToolMode() {
    switch (m_debugToolMode) {
        case DebugToolMode::Disable:
            return false;
        case DebugToolMode::Hidden:
            setDebugToolMode(DebugToolMode::Minimalized);
            return true;
        case DebugToolMode::Minimalized:
            setDebugToolMode(DebugToolMode::Activated);
            return true;
        case DebugToolMode::Activated:
            setDebugToolMode(DebugToolMode::Hidden);
            return true;
        default:
            LN_UNREACHABLE();
            return false;
    }
}

void EngineManager::setDebugToolMode(DebugToolMode mode) {
    m_debugToolMode = mode;

    if (m_mainWindow) {
        switch (m_debugToolMode) {
            case DebugToolMode::Disable:
                m_mainWindow->setImGuiLayerEnabled(false);
                m_mainWindow->platformWindow()->setWindowTitle(m_settings.mainWindowTitle);
                break;
            case DebugToolMode::Hidden:
                m_mainWindow->setImGuiLayerEnabled(false);
                m_mainWindow->platformWindow()->setWindowTitle(m_settings.mainWindowTitle);
                break;
            case DebugToolMode::Minimalized:
                m_mainWindow->setImGuiLayerEnabled(false);
                break;
            case DebugToolMode::Activated:
                m_mainWindow->setImGuiLayerEnabled(true);
                break;
            default:
                LN_UNREACHABLE();
                break;
        }
    }
}

//==============================================================================
// EngineDomain

class EngineContextWrap {
public:
    // グローバル変数にすると、別のグローバル変数の初期化から EngineContext にアクセスするときに、まだ EngineContext のコンストラクタが呼ばれていないことが起こる。
    // ローカルの static にしておくことで、アクセスした時点で初期化が走ることを保証する。
    static EngineContext* getInstance() {
        static EngineContext instance;
        return &instance;
    }
};

void EngineDomain::release() {
    EngineContextWrap::getInstance()->disposeEngineManager();
    // EngineContextWrap::getInstance()->disposeRuntimeManager();

    // if (g_engineManager) {
    //	g_engineManager->dispose();
    //	RefObjectHelper::release(g_engineManager);
    //	g_engineManager = nullptr;
    // }
    // if (g_engineContext) {
    // RefObjectHelper::release(g_engineContext);
    //	g_engineContext = nullptr;
    //}
}

EngineContext* EngineDomain::engineContext() {
    // if (!g_engineContext) {
    //	g_engineContext = std::make_unique<EngineContext>();
    // g_engineContext = makeRef<EngineContext>();
    // g_engineContext = LN_NEW EngineContext();
    // g_engineContext->init(g_globalRuntimeManagerSettings);
    //}
    // return g_engineContext.get();
    return EngineContextWrap::getInstance();
}

EngineManager* EngineDomain::engineManager() {
    //   EngineDomain::runtimeManager();
    // if (!g_engineManager) {
    //	g_engineManager = LN_NEW EngineManager();
    //}
    // return g_engineManager;

    return engineContext()->engineManager();
}

// PlatformManager* EngineDomain::platformManager()
//{
//	return engineManager()->PlatformManager::instance()();
// }

InputManager* EngineDomain::inputManager() {
    return engineManager()->inputManager();
}

AudioManager* EngineDomain::audioManager() {
    return engineManager()->audioManager();
}

EffectManager* EngineDomain::effectManager() {
    return engineManager()->effectManager();
}

PhysicsManager* EngineDomain::physicsManager() {
    return engineManager()->physicsManager();
}

VisualManager* EngineDomain::visualManager() {
    return engineManager()->visualManager();
}

SceneManager* EngineDomain::sceneManager() {
    return engineManager()->sceneManager();
}

UIManager* EngineDomain::uiManager() {
    return engineManager()->uiManager();
}

World* EngineDomain::mainWorld() {
    return engineManager()->mainWorld();
}

} // namespace detail
} // namespace ln
