#include <SDL.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_sdlrenderer2.h>
#include <cstdlib>
#include <imgui.h>
#include <iostream>

#include "samples/Samples.h"

using namespace pockets;
using namespace choreograph;
using namespace std;

class SamplesApp {
public:
    void update();

    void loadSample(int index);

private:
    pk::SceneRef _current_scene;
    pk::SceneRef _previous_scene;
    ch::Timeline _timeline;
    int _scene_index = 0;
    string _scene_name;
    float _controls_height = 50;
};

void SamplesApp::update() {
    if (_scene_name != SampleNames[_scene_index]) {
        loadSample(_scene_index);
    }

    ImGuiIO &io = ImGui::GetIO();
    ch::Time dt = (Time)io.DeltaTime;
    _timeline.step(dt);

    ImGui::SetNextWindowSize(
        {ImGui::GetMainViewport()->Size.x,
         ImGui::GetMainViewport()->Size.y - _controls_height});
    _current_scene->baseDraw(io.DeltaTime);

    ImGui::SetNextWindowPos(
        {0, ImGui::GetMainViewport()->Size.y - _controls_height});
    ImGui::SetNextWindowSize(
        {ImGui::GetMainViewport()->Size.x, _controls_height});
    ImGui::Begin("Hello, world!", NULL,
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration);

    if (ImGui::Button("PREVIOUS")) {
        loadSample(_scene_index - 1);
    }
    ImGui::SameLine();
    if (ImGui::Button("NEXT")) {
        loadSample(_scene_index + 1);
    }

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                1000.0f / io.Framerate, io.Framerate);
    ImGui::End();
}

void SamplesApp::loadSample(int index) {
    bool do_animate = (index != _scene_index);
    int width = ImGui::GetMainViewport()->Size.x;
    const int start_x = (index < _scene_index) ? -width : width;
    const int vanish_x = -start_x;

    float cooldown =
        (_current_scene && _current_scene->timeline().empty()) ? 0 : 0.25f;

    if (index < 0) {
        index = SampleList.size() - 1;
    }
    index %= SampleList.size();

    _scene_index = index;
    _scene_name = SampleNames[_scene_index];

    std::cout << "Loading Sample: " << _scene_name << endl;

    if (_current_scene && do_animate) {
        _previous_scene = _current_scene;
        // Decelerate animation of previous scene down to zero.
        _timeline.apply(_previous_scene->getAnimationSpeedOutput())
            .then<RampTo>(0, 0.4f);

        // Slide previous scene off screen after/during deceleration.
        _timeline.apply(_previous_scene->getOffsetOutput())
            .hold(cooldown)
            .then<RampTo>(vec2(vanish_x, 0.0f), 0.4f, EaseInQuad())
            .finishFn([this] {
                _previous_scene.reset(); // get rid of previous scene
            });
    }

    _current_scene = SampleList[_scene_index].second();

    _current_scene->setSize(
        {double(width), ImGui::GetMainViewport()->Size.y - _controls_height});
    _current_scene->setup();
    _current_scene->show();

    // animate current on.
    if (do_animate) {
        _current_scene->setOffset(vec2(start_x, 0.0f));
        _current_scene->pause();

        _timeline.apply(_current_scene->getOffsetOutput())
            .hold(0.2f + cooldown)
            .then<RampTo>(vec2(0), 0.66f, EaseOutQuint())
            .finishFn([this] { _current_scene->resume(); });
    }
}

int main(int argc, char **argv) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) !=
        0) {
        SDL_Log("Error: %s", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_Window *window =
        SDL_CreateWindow("Choreograph samples", SDL_WINDOWPOS_CENTERED,
                         SDL_WINDOWPOS_CENTERED, 800, 600, 0);
    if (!window) {
        SDL_Log("Error: SDL_CreateWindow(): %s", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer) {
        SDL_Log("Error creating SDL_Renderer!");
        return EXIT_FAILURE;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    SamplesApp app;

    bool done = false;
    while (!done) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT) done = true;
        }

        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        app.update();

        ImGui::Render();
        SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x,
                           io.DisplayFramebufferScale.y);
        SDL_SetRenderDrawColor(renderer, (Uint8)(clear_color.x * 255),
                               (Uint8)(clear_color.y * 255),
                               (Uint8)(clear_color.z * 255),
                               (Uint8)(clear_color.w * 255));
        SDL_RenderClear(renderer);
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
        SDL_RenderPresent(renderer);
    }

    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}
