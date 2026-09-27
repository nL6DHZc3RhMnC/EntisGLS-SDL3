#include "psb_key_dialog.h"
#include "psb_key_settings.h"
#include <SDL3/SDL.h>
#include <memory>
#include <stdexcept>

namespace entis::launcher {
bool ShowPsbKeySettings(const std::filesystem::path& directory, const std::string& title) {
    const auto previous = ReadPsbKeyOverride(directory);
    std::string input = previous ? std::to_string(*previous) : "";
    using Window = std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)>;
    using Renderer = std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)>;
    Window window(SDL_CreateWindow((title + " - PSB settings").c_str(), 800, 340, SDL_WINDOW_HIGH_PIXEL_DENSITY), SDL_DestroyWindow);
    if (!window) throw std::runtime_error(SDL_GetError());
    Renderer renderer(SDL_CreateRenderer(window.get(), nullptr), SDL_DestroyRenderer);
    if (!renderer) throw std::runtime_error(SDL_GetError());
    SDL_SetRenderLogicalPresentation(renderer.get(), 400, 170, SDL_LOGICAL_PRESENTATION_LETTERBOX);
    SDL_StartTextInput(window.get());
    std::string error;
    const SDL_FRect save{12, 108, 95, 25}, cancel{117, 108, 95, 25}, automatic{222, 108, 165, 25};
    auto accept = [&] {
        try { WritePsbKeyOverride(directory, ParsePsbKey(input)); return true; }
        catch (const std::exception& e) { error = e.what(); return false; }
    };
    for (;;) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) return false;
            if (event.type == SDL_EVENT_TEXT_INPUT) {
                for (const char* c = event.text.text; *c && input.size() < 20; ++c)
                    if ((*c >= '0' && *c <= '9') || (*c >= 'a' && *c <= 'f') || (*c >= 'A' && *c <= 'F') || *c == 'x' || *c == 'X') input += *c;
                error.clear();
            } else if (event.type == SDL_EVENT_KEY_DOWN) {
                if (event.key.key == SDLK_ESCAPE) return false;
                if (event.key.key == SDLK_BACKSPACE && !input.empty()) { input.pop_back(); error.clear(); }
                if (event.key.key == SDLK_RETURN && accept()) return true;
                if (event.key.key == SDLK_V && (event.key.mod & (SDL_KMOD_CTRL | SDL_KMOD_GUI))) {
                    char* clipboard = SDL_GetClipboardText();
                    if (clipboard) { input = std::string(clipboard).substr(0, 20); SDL_free(clipboard); }
                }
            } else if (event.type == SDL_EVENT_MOUSE_BUTTON_UP && event.button.button == SDL_BUTTON_LEFT) {
                SDL_FPoint point;
                SDL_RenderCoordinatesFromWindow(renderer.get(), event.button.x, event.button.y, &point.x, &point.y);
                if (SDL_PointInRectFloat(&point, &save) && accept()) return true;
                if (SDL_PointInRectFloat(&point, &cancel)) return false;
                if (SDL_PointInRectFloat(&point, &automatic)) { input.clear(); error.clear(); }
            }
        }
        SDL_SetRenderDrawColor(renderer.get(), 29, 32, 38, 255); SDL_RenderClear(renderer.get());
        SDL_SetRenderDrawColor(renderer.get(), 235, 237, 242, 255);
        SDL_RenderDebugText(renderer.get(), 12, 12, "PSB key: decimal or 0x hexadecimal");
        SDL_RenderDebugText(renderer.get(), 12, 28, "Blank = XML setting or driver detection");
        const SDL_FRect field{12, 49, 375, 27};
        SDL_SetRenderDrawColor(renderer.get(), 70, 76, 86, 255); SDL_RenderFillRect(renderer.get(), &field);
        SDL_SetRenderDrawColor(renderer.get(), 250, 250, 250, 255);
        SDL_RenderDebugText(renderer.get(), 18, 58, (input + "_").c_str());
        SDL_RenderDebugText(renderer.get(), 12, 87, "Enter: save    Escape: cancel");
        SDL_SetRenderDrawColor(renderer.get(), 63, 79, 103, 255);
        for (const auto& rect : {save, cancel, automatic}) SDL_RenderFillRect(renderer.get(), &rect);
        SDL_SetRenderDrawColor(renderer.get(), 250, 250, 250, 255);
        SDL_RenderDebugText(renderer.get(), 38, 117, "Save");
        SDL_RenderDebugText(renderer.get(), 140, 117, "Cancel");
        SDL_RenderDebugText(renderer.get(), 251, 117, "Automatic");
        SDL_SetRenderDrawColor(renderer.get(), 255, 160, 150, 255);
        SDL_RenderDebugText(renderer.get(), 12, 143, error.substr(0, 47).c_str());
        SDL_RenderPresent(renderer.get());
        SDL_Delay(16);
    }
}
}
