﻿
#pragma once
#include "Common.hpp"

namespace ln {

class Platform {
public:
    /**
	 * Processes event queue.
	 */
    static bool processEvents();

    static bool shouldQuit();

    /**
	 * Get MainWindow.
	 */
    static PlatformWindow* mainWindow();

    static Ref<PlatformWindow> createWindow(const WindowCreationSettings& settings);

};

} // namespace ln
