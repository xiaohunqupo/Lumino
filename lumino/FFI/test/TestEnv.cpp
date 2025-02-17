﻿#include "Common.hpp"
#include "../src/Engine/EngineManager.hpp"
#include "../src/Scene/SceneManager.hpp"
#include <LuminoEngine/Runtime/detail/RuntimeManager.hpp>
#include "TestEnv.hpp"

String TestEnv::LuminoCLI;
Ref<DepthBuffer> TestEnv::depthBuffer;
//RenderTargetTexture* TestEnv::lastBackBuffer;

void TestEnv::setup() {
    EngineFeature feature = EngineFeature::Experimental; //EngineFeature::Public; //

    //Logger::addStdErrAdapter();
    EngineSettings::setMainWindowSize(160, 120);
    //EngineSettings::setMainBackBufferSize(160, 120);
    EngineSettings::setGraphicsAPI(GraphicsAPI::Vulkan); //GraphicsAPI::OpenGL);//GraphicsAPI::DirectX12);//
    EngineSettings::setPriorityGPUName(_TT("Microsoft Basic Render Driver"));
    EngineSettings::setGraphicsDebugEnabled(true);
    EngineSettings::setEngineFeatures(feature);
    EngineSettings::addAssetDirectory(LN_LOCALFILE(_TT("Assets")));
    //EngineSettings::setAssetStorageAccessPriority(AssetStorageAccessPriority::AllowLocalDirectory);
    EngineContext::current()->initializeEngineManager();
    detail::EngineDomain::engineManager()->initializeAllManagers();
    detail::EngineDomain::engineManager()->sceneManager()->autoAddingToActiveWorld = true;

    if (feature == EngineFeature::Experimental) // Experimental
    {
        Font::registerFontFromFile(LN_LOCALFILE("../../../tools/VLGothic/VL-Gothic-Regular.ttf"));
        Engine::mainCamera()->setBackgroundColor(Color(0.5, 0.5, 0.5, 1.0));

        //Engine::mainAmbientLight()->setColor(Color::White);
        //Engine::mainAmbientLight()->setIntensity(0.5);
        //Engine::mainDirectionalLight()->setColor(Color::White);
        //Engine::mainDirectionalLight()->setIntensity(0.5);
        //Engine::mainDirectionalLight()->setPosition(10, 10, -10);
        //Engine::mainDirectionalLight()->lookAt(Vector3(0, 0, 0));
    }

    const auto backbufferSize = Engine::mainWindow()->swapChain()->backbufferSize();
    depthBuffer = DepthBuffer::create(backbufferSize.width, backbufferSize.height);

#ifdef LN_OS_WIN32
    LuminoCLI = Path::combine(Path(ln::Environment::executablePath()).parent().parent().parent().parent(), _TT("tools"), _TT("LuminoCLI"), _TT("Debug"), _TT("lumino-cli.exe"));
#else
#endif
    LN_TEST_CLEAN_SCENE;
}

void TestEnv::teardown() {
    depthBuffer = nullptr;
    detail::EngineDomain::release();
}

void TestEnv::updateFrame() {
    detail::EngineDomain::engineManager()->updateFrame();
    detail::EngineDomain::engineManager()->presentFrame(nullptr, nullptr);
}

void TestEnv::resetScene() {
    Engine::world()->removeAllObjects();
    Engine::mainUIView()->removeAllChildren();
#ifdef LN_COORD_RH
    Engine::mainCamera()->setPosition(0, 0, 10);
#else
    Engine::mainCamera()->setPosition(0, 0, -10);
#endif
    Engine::mainCamera()->lookAt(Vector3(0, 0, 0));
    Engine::mainLight()->lookAt(0, -1, 0);
    Engine::mainLight()->setEnabled(true);
    Engine::mainLight()->setSkyColor(Color::Black);
    Engine::mainLight()->setGroundColor(Color::Black);
    Engine::mainLight()->setAmbientColor(Color(0.5, 0.5, 0.5));
    Engine::mainLight()->setColor(Color::White);
    Engine::mainLight()->setIntensity(1.0f);
}

GraphicsCommandList* TestEnv::graphicsContext() {
    return Engine::graphicsContext();
}

SwapChain* TestEnv::mainWindowSwapChain() {
    return Engine::mainWindow()->swapChain();
}

void TestEnv::resetGraphicsContext(GraphicsCommandList* context) {
    context->resetState();
    //context->setRenderTarget(0, Engine::mainWindow()->swapChain()->currentBackbuffer());
    //context->setDepthBuffer(depthBuffer);
}

GraphicsCommandList* TestEnv::beginFrame() {
    auto commandList = TestEnv::mainWindowSwapChain()->currentCommandList2();
    commandList->beginCommandRecoding();
    return commandList;
}

void TestEnv::endFrame() {
    auto commandList = TestEnv::mainWindowSwapChain()->currentCommandList2();
    commandList->endCommandRecoding();
    TestEnv::mainWindowSwapChain()->present();
}

RenderPass* TestEnv::renderPass() {
    return TestEnv::mainWindowSwapChain()->currentRenderPass();
}

bool TestEnv::checkCurrentBackbufferScreenShot(const Path& filePath, int passRate, bool save) {
    return GraphicsTestHelper::checkScreenShot(filePath, TestEnv::mainWindowSwapChain()->currentBackbuffer(), passRate, save);
}
