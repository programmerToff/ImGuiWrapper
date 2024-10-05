#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui-docking/imgui.h>
#include <imgui-docking/imgui_impl_glfw.h>
#include <imgui-docking/imgui_impl_opengl3.h>
#include <imgui-docking/imgui_internal.h>

#include <string>
#include <vector>
#include <functional>
#include <algorithm>

namespace ImGui {

	struct SubWindow
	{
		std::string name;
		std::function<void()> func;

		ImGuiID execute(bool* open, bool* split)
		{
			ImGui::Begin(name.c_str(), open);


			if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
			{
				ImGui::OpenPopup("SplitOptions");
			}
			if (ImGui::BeginPopup("SplitOptions"))
			{
				ImGui::Text("Choose an action:");
				ImGui::Separator();
				if (ImGui::Selectable("Split"))
				{
					*split = true;
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}
			if (func) func();

			ImGuiID out = ImGui::GetWindowDockID();
			ImGui::End();
			return out;
		}

		SubWindow(std::string str, std::function<void()> function) : name(str), func(function) {}
		SubWindow() : name(" "), func([]() {}) {}

		bool operator==(const SubWindow& other)
		{
			return name == other.name;
		}
	};

	class Window
	{
		std::function<void()> splitCallback;
		SubWindow splitWindow;

		std::function<void(ImGuiID)> rearrangeFunction;
		bool should_rearrange;

		GLFWwindow* window;

        std::vector<SubWindow> winList;

        void windowContent();

		static void resizeCallbackStatic(GLFWwindow* window, int width, int height);

		std::string getNumberedString(std::string str, std::vector<SubWindow> winList);
	public:
		Window(std::string name, int w, int h);
		~Window();

		void frame();

		bool shouldClose();

		void pushWindow(std::string name, std::function<void()> function);

		void rearrangeWindows(std::function<void(ImGuiID)> func);
		std::function<void()> setSplitWindow(SubWindow win, std::function<void()> callback);
	};



    std::string Window::getNumberedString(std::string str, std::vector<SubWindow> winList)
    {
        std::string base = str;
        uint16_t suffix = 1;
        while (true)
        {
            if (std::any_of(winList.begin(), winList.end(), [str](const SubWindow& other)
                {
                    return str == other.name;
                }))
            {
                str = base + std::to_string(suffix++);
            }
            else break;
        }
        return str;
    }
    static ImGuiID _setupDockSpace()
    {
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        ImGui::Begin("Main Dockspace", nullptr, window_flags);
	ImGui::PopStyleVar(2);

	ImGuiIO& io = ImGui::GetIO();
	ImGuiID main_dockspace_id = ImGui::GetID("MainDockspace");
	ImGui::DockSpace(main_dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
	return main_dockspace_id;
    }

    Window::Window(std::string name, int w, int h)
    {
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        window = glfwCreateWindow(w, h, name.c_str(), NULL, NULL);
        glfwMakeContextCurrent(window);
        gladLoadGL();

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 460");

        glfwSetFramebufferSizeCallback(window, resizeCallbackStatic);
        glfwSetWindowUserPointer(window, this);

        should_rearrange = false;
        splitWindow = SubWindow("SplitWindow", []() {});
        splitCallback = nullptr;
    }
    Window::~Window()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void Window::windowContent()
    {
        ImGuiID dockspace_id = _setupDockSpace();

        if (should_rearrange)
        {
            ImGui::DockBuilderRemoveNode(dockspace_id);
            ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size);
            ImGuiID dock_main_id = dockspace_id;

            should_rearrange = false;
            if (rearrangeFunction) rearrangeFunction(dock_main_id);

            ImGui::DockBuilderFinish(dockspace_id);
        }

        std::vector<int> indices_to_delete;
        auto size = winList.size();
        for (int i = 0; i < size; i++)
        {
            bool open = true, split = false;
            auto it = winList[i];
            ImGuiID window_dock_id = it.execute(&open, &split);
            if (split)
            {
                pushWindow(splitWindow.name, splitWindow.func);
                if (splitCallback) splitCallback();

                ImGuiID new_window_id;

                ImVec2 size = ImGui::DockBuilderGetNode(window_dock_id)->Size;
                if (size.x > size.y)
                    ImGui::DockBuilderSplitNode(window_dock_id, ImGuiDir_Right, 0.5f, &new_window_id, &window_dock_id);
                else
                    ImGui::DockBuilderSplitNode(window_dock_id, ImGuiDir_Down, 0.5f, &new_window_id, &window_dock_id);

                ImGui::DockBuilderDockWindow(it.name.c_str(), window_dock_id);
                ImGui::DockBuilderDockWindow(winList.back().name.c_str(), new_window_id);

                ImGui::DockBuilderFinish(dockspace_id);
            }
            if (!open) indices_to_delete.push_back(i);
        }
        for (auto it = indices_to_delete.rbegin(); it != indices_to_delete.rend(); ++it) winList.erase(winList.begin() + *it);
    }
    void Window::frame()
    {
        auto& io = ImGui::GetIO();
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        windowContent();

        ImGui::End();
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);

        glfwSwapBuffers(window);
    }

    bool Window::shouldClose()
    {
        return glfwWindowShouldClose(window);
    }

    void Window::pushWindow(std::string name, std::function<void()> function)
    {
        winList.push_back(SubWindow(getNumberedString(name, winList), function));
    }

    void Window::rearrangeWindows(std::function<void(ImGuiID)> func)
    {
        should_rearrange = true;
        rearrangeFunction = func;
    }
    std::function<void()> Window::setSplitWindow(SubWindow win, std::function<void()> callback)
    {
        splitWindow = win;
        std::function<void()> oldCallback = splitCallback;
        splitCallback = callback;
        return oldCallback;
    }

    void Window::resizeCallbackStatic(GLFWwindow* window, int width, int height)
    {
        Window* windowPtr = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (windowPtr)
        {
            windowPtr->frame();
        }
    }

}
