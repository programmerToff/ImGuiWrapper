# ImGui Wrapper

A simple and customizable C++ wrapper for Dear ImGui, designed to streamline the creation of ImGui-based applications with dockable windows and easy layout management.

## Features

- Easy window creation and management
- Dockable windows with automatic arrangement
- Split window functionality
- Simple API for adding and organizing windows

## Dependencies

- Dear ImGui
- GLFW
- OpenGL (and GLAD for OpenGL function loading)

## Example Usage

```cpp
#include "ImGuiWrapper.h"
#include <iostream>

int main() {
    // Create a window with a title and dimensions
    ImGui::Window mainWindow("ImGui Wrapper Demo", 1280, 720);

    // Add windows with their content
    mainWindow.pushWindow("Console", []() {
        ImGui::TextWrapped("This is the console window. You can add logging or debug information here.");
    });

    mainWindow.pushWindow("Properties", []() {
        ImGui::Text("Object Properties");
        ImGui::Separator();
        static float value = 0.5f;
        ImGui::SliderFloat("Value", &value, 0.0f, 1.0f);
    });

    mainWindow.pushWindow("Viewport", []() {
        ImGui::Text("3D Viewport");
        ImGui::Separator();
        ImGui::Button("Render", ImVec2(100, 30));
    });

    // Arrange windows
    mainWindow.rearrangeWindows([](ImGuiID dockspaceId) {
        ImGuiID leftId, rightId, bottomId;
        ImGui::DockBuilderSplitNode(dockspaceId, ImGuiDir_Left, 0.2f, &leftId, &rightId);
        ImGui::DockBuilderSplitNode(rightId, ImGuiDir_Down, 0.3f, &bottomId, &rightId);

        ImGui::DockBuilderDockWindow("Console", bottomId);
        ImGui::DockBuilderDockWindow("Properties", leftId);
        ImGui::DockBuilderDockWindow("Viewport", rightId);
    });

    // Set a split window
    mainWindow.setSplitWindow(ImGui::SubWindow("New Window", []() {
        ImGui::Text("This is a new split window");
        ImGui::Separator();
        ImGui::Button("Do Something", ImVec2(120, 30));
    }), []() {
        std::cout << "Split window created!" << std::endl;
    });

    // Main loop
    while (!mainWindow.shouldClose()) {
        mainWindow.frame();
    }

    return 0;
}
```

## Design Philosophy

This wrapper is designed to be very simple and easily editable. The goal is to provide a straightforward interface for creating ImGui-based applications while allowing users to customize and extend the functionality as needed. The code is intentionally kept minimal and clear, making it easy for developers to understand and modify according to their specific requirements.

## Getting Started

1. Clone this repository.
2. Include the necessary dependencies (Dear ImGui, GLFW, and OpenGL).
3. Include "ImGuiWrapper.h" in your project.
4. Start building your ImGui application using the provided wrapper functions.

## Contributing

Contributions are welcome! Feel free to submit pull requests or open issues to improve the wrapper or suggest new features.

## License

This project is licensed under the MIT License - see the LICENSE file for details.
