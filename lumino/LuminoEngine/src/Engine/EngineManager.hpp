﻿#pragma once
#include <LuminoEngine/Engine/Common.hpp>
#include <LuminoEngine/Asset/Common.hpp>
#include <LuminoEngine/Input/Common.hpp>
#include <LuminoEngine/Graphics/Common.hpp>
#include <LuminoPlatform/PlatformEvent.hpp>
#include <LuminoGraphics/Common.hpp>
#include "FpsController.hpp"

namespace ln {
class DiagnosticsManager;
class EngineContext;
class Application;
// class UIContext;
class UIMainWindow;
class UIViewport;
class UIRenderView;
class UIDomainProvidor;
class PhysicsWorld;
class PhysicsWorld2D;
class World;
class Level;
class WorldRenderView;
class Camera;
class AmbientLight;
class DirectionalLight;
class UIEventArgs;
class Dispatcher;

namespace detail {
class PlatformManager;
class AnimationManager;
class InputManager;
class AudioManager;
class GraphicsManager;
class RenderingManager;
class EffectManager;
class PhysicsManager;
class AssetManager;
class VisualManager;
class SceneManager;
class UIManager;
class DebugInterface;
class RuntimeEditor;
class AppDataInternal;
class EngineProfiler;

struct EngineSettingsAssetArchiveEntry {
    Path filePath;
    String password; // TODO: 平文保持はよくない。MD5
};

struct EngineSettings {
    //Application* application = nullptr;
    Flags<EngineFeature> features = EngineFeature::Experimental;
    String bundleIdentifier = _TT("lumino");
    SizeI mainWindowSize = SizeI(640, 480);
    SizeI mainWorldViewSize = SizeI(0, 0);
    String mainWindowTitle = _TT("Lumino");
    bool mainWindowResizable = false;
    AssetStorageAccessPriority assetStorageAccessPriority = AssetStorageAccessPriority::DirectoryFirst;
    List<EngineSettingsAssetArchiveEntry> assetArchives;
    List<Path> assetDirectories;

    GraphicsAPI graphicsAPI = GraphicsAPI::Default;
    bool graphicsDebugEnabled = false;

    bool debugMode = false;
    String engineLogFilePath;
    String engineResourcesPath;

    String priorityGPUName;

    intptr_t userMainWindow = 0;
    //bool useProxyPlatformWindow = false;

    InputBindingSet inputConfig = InputBindingSet::Keyboard;

    bool standaloneFpsControl = true;
    bool createMainLights = false;
    int frameRate = 60;
    bool debugToolEnabled = false;
    bool developmentToolsEnabled = false;

    String defaultUITheme;
    Path fontFile;

    bool defaultObjectsCreation = false;
    WindowSystem windowSystem = WindowSystem::GLFWWithOpenGL;
    bool graphicsContextManagement = true;
    bool externalMainLoop = true;
    bool useExternalSwapChain = false;
    bool autoCoInitialize = true;

    EngineSettings() {
#ifdef LN_DEBUG
        debugMode = true;
        debugToolEnabled = true;
#endif
    }

    bool tryLoad();
};

class EngineManager
    : public RefObject
    , public IPlatforEventListener {
public:
    static EngineSettings s_settings;

    EngineManager();
    virtual ~EngineManager();

    void init(const EngineSettings& settings);
    void dispose();

    void initializeAllManagers();
    void initializeCommon();
    void initializePlatformManager();
    void initializeAnimationManager();
    void initializeInputManager();
    void initializeAudioManager();
    void initializeFontManager();
    void initializeGraphicsManager();
    void initializeRenderingManager();
    void initializeEffectManager();
    void initializePhysicsManager();
    void initializeVisualManager();
    void initializeSceneManager();
    void initializeUIManager();
	
    void craeteDefaultObjectsIfNeeded(UIMainWindow* userMainWindow);

    bool updateUnitily();
    void updateFrame();
    void presentFrame(GraphicsCommandList* commandList, RenderTargetTexture* renderTarget); // swap。renderFrame() と分けているのは、間に コールバック以外の Engine 外部のレンダリングを許可するため
    void resetFrameDelay();
    bool isExitRequested() const { return m_exitRequested; }
    void quit();
    int runApplication();

    const EngineSettings& settings() { return m_settings; }
    const Ref<InputManager>& inputManager() const { return m_inputManager; }
    const Ref<AudioManager>& audioManager() const { return m_audioManager; }
    const Ref<EffectManager>& effectManager() const { return m_effectManager; }
    const Ref<PhysicsManager>& physicsManager() const { return m_physicsManager; }
    const Ref<VisualManager>& visualManager() const { return m_visualManager; }
    const Ref<SceneManager>& sceneManager() const { return m_sceneManager; }
    const Ref<UIManager>& uiManager() const { return m_uiManager; }

    const FpsController& fpsController() const { return m_fpsController; }

    static ln::Path findRepositoryRootForTesting();
    static ln::Path findParentDirectoryContainingSpecifiedFile(StringView file);
    const Path& persistentDataPath() const;
    void setTimeScale(float value) { m_timeScale = value; }
    // void setShowDebugFpsEnabled(bool value) { m_showDebugFpsEnabled = value; }
    void setupMainWindow(UIMainWindow* window, bool createBasicObjects);
    void setupApplication(Application* app);

    //void setApplication(Application* app);
    const Ref<Application>& application() const;
    const Ref<AppDataInternal>& appData() const { return m_appData; }
    // const Ref<UIContext>& mainUIContext() const { return m_mainUIContext; }
    const Ref<UIMainWindow>& mainWindow() const { return m_mainWindow; }
    const Ref<UIViewport>& mainViewport() const { return m_mainViewport; }
    const Ref<UIDomainProvidor>& mainUIView() const { return m_mainUIRoot; }
    const Ref<World>& mainWorld() const { return m_mainWorld; }
    const Ref<Camera>& mainCamera() const { return m_mainCamera; }
    const Ref<WorldRenderView>& mainRenderView() const { return m_mainWorldRenderView; }
    const Ref<PhysicsWorld>& mainPhysicsWorld() const { return m_mainPhysicsWorld; }
    const Ref<PhysicsWorld2D>& mainPhysicsWorld2D() const { return m_mainPhysicsWorld2D; }

    const Ref<DebugInterface>& debugInterface() const { return m_debugInterface; }
    const std::unique_ptr<EngineProfiler>& engineProfiler() const { return m_engineProfiler; }
    const Ref<detail::RuntimeEditor>& runtimeEditor() const { return m_runtimeEditor; }

    std::function<void(void)> preUpdateCallback;

private:
    enum class DebugToolMode {
        Disable,
        Hidden,
        Minimalized,
        Activated,
    };


    void resolveActiveGraphicsAPI();
    virtual bool onPlatformEvent(const PlatformEventArgs& e) override;
    void handleImGuiDebugLayer(UIEventArgs* e);
    bool toggleDebugToolMode();
    void setDebugToolMode(DebugToolMode mode);

    EngineSettings m_settings;
    Ref<AppDataInternal> m_appData;
    GraphicsAPI m_activeGraphicsAPI;

    bool m_commonInitialized = false;
    Ref<InputManager> m_inputManager;
    Ref<AudioManager> m_audioManager;
    Ref<EffectManager> m_effectManager;
    Ref<PhysicsManager> m_physicsManager;
    Ref<VisualManager> m_visualManager;
    Ref<SceneManager> m_sceneManager;
    Ref<UIManager> m_uiManager;
    FpsController m_fpsController;

    Path m_persistentDataPath;
    Path m_engineResourcesPath;

    //Ref<Application> m_application;
    Ref<UIMainWindow> m_mainWindow;
    Ref<UIViewport> m_mainViewport;
    Ref<UIRenderView> m_mainUIRenderView; // m_mainViewport の ViewBox 内部に配置する
    Ref<UIDomainProvidor> m_mainUIRoot;   // m_mainUIRenderView の RootElement
    Ref<World> m_mainWorld;
    Ref<Level> m_mainScene;
    Ref<Camera> m_mainCamera;
    Ref<WorldRenderView> m_mainWorldRenderView;
    Ref<PhysicsWorld> m_mainPhysicsWorld;
    Ref<PhysicsWorld2D> m_mainPhysicsWorld2D;
    Ref<DebugInterface> m_debugInterface;
    Ref<Camera> m_debugCamera;
    Ref<WorldRenderView> m_debugWorldRenderView;
    std::unique_ptr<EngineProfiler> m_engineProfiler;

    float m_timeScale;
    bool m_exitRequested;
    //   bool m_showDebugFpsEnabled;
    // bool m_debugToolEnabled;
    DebugToolMode m_debugToolMode;
    Ref<detail::RuntimeEditor> m_runtimeEditor;

#if defined(LN_OS_WIN32)
    bool m_comInitialized;
    bool m_oleInitialized;
#endif
};

} // namespace detail
} // namespace ln
