#pragma once

#include <imgui.h>
#include <imgui_internal.h>
#include <algorithm>
#include <cmath>

namespace OS::Ui {

inline constexpr float TaskbarHeight = 44.0f;

// Apply bounds before Begin(), including positions loaded from imgui.ini.
inline void prepareAppWindow(const char* name, ImVec2 preferredSize, ImVec2 preferredOffset,
                             ImVec2 minimumSize = ImVec2(360.0f, 280.0f)) {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const ImVec2 origin(viewport->Pos.x + 8.0f, viewport->Pos.y + 8.0f);
    const ImVec2 maximum(std::max(1.0f, viewport->Size.x - 16.0f),
                         std::max(1.0f, viewport->Size.y - TaskbarHeight - 16.0f));
    minimumSize.x = std::min(minimumSize.x, maximum.x);
    minimumSize.y = std::min(minimumSize.y, maximum.y);
    ImGui::SetNextWindowSizeConstraints(minimumSize, maximum);

    ImGuiWindow* window = ImGui::FindWindowByName(name);
    const ImGuiWindowSettings* settings = window ? nullptr : ImGui::FindWindowSettingsByID(ImHashStr(name));
    ImVec2 size = window ? window->SizeFull : settings ? ImVec2(settings->Size.x, settings->Size.y) : preferredSize;
    size.x = std::clamp(size.x, minimumSize.x, maximum.x);
    size.y = std::clamp(size.y, minimumSize.y, maximum.y);
    ImVec2 position = window ? window->Pos : settings ? ImVec2(settings->Pos.x, settings->Pos.y)
        : ImVec2(viewport->Pos.x + preferredOffset.x, viewport->Pos.y + preferredOffset.y);
    position.x = std::clamp(position.x, origin.x, origin.x + maximum.x - size.x);
    position.y = std::clamp(position.y, origin.y, origin.y + maximum.y - size.y);
    if (!window) {
        ImGui::SetNextWindowSize(size);
        ImGui::SetNextWindowPos(position);
    } else {
        if (size.x != window->SizeFull.x || size.y != window->SizeFull.y)
            ImGui::SetNextWindowSize(size);
        if (position.x != window->Pos.x || position.y != window->Pos.y)
            ImGui::SetNextWindowPos(position);
    }
}

enum class Icon { Start, Tasks, Folder, Terminal, System, Note, Volume, Network, Power };

inline void drawIcon(ImDrawList* draw, Icon icon, ImVec2 position, float size, ImU32 color) {
    const auto point = [&](float x, float y) { return ImVec2(position.x + x * size, position.y + y * size); };
    const float stroke = std::max(1.5f, size / 13.0f);
    const auto line = [&](float x1, float y1, float x2, float y2) {
        draw->AddLine(point(x1, y1), point(x2, y2), color, stroke);
    };
    switch (icon) {
    case Icon::Start:
        for (int row = 0; row < 2; ++row)
            for (int column = 0; column < 2; ++column)
                draw->AddRectFilled(point(.12f + column * .43f, .12f + row * .43f),
                                    point(.45f + column * .43f, .45f + row * .43f), color, 1.0f);
        break;
    case Icon::Tasks:
        draw->AddRect(point(.08f, .12f), point(.92f, .88f), color, 2.0f, 0, stroke);
        line(.26f, .72f, .26f, .48f); line(.5f, .72f, .5f, .3f); line(.74f, .72f, .74f, .4f);
        break;
    case Icon::Folder:
        line(.08f, .3f, .08f, .82f); line(.08f, .82f, .92f, .82f);
        line(.92f, .82f, .92f, .3f); line(.92f, .3f, .48f, .3f);
        line(.48f, .3f, .38f, .18f); line(.38f, .18f, .08f, .18f); line(.08f, .18f, .08f, .3f);
        line(.08f, .4f, .92f, .4f);
        break;
    case Icon::Terminal:
        draw->AddRect(point(.06f, .14f), point(.94f, .86f), color, 2.0f, 0, stroke);
        line(.23f, .36f, .4f, .5f); line(.4f, .5f, .23f, .64f); line(.52f, .65f, .77f, .65f);
        break;
    case Icon::System:
        draw->AddRect(point(.08f, .12f), point(.92f, .7f), color, 2.0f, 0, stroke);
        line(.5f, .7f, .5f, .9f); line(.28f, .9f, .72f, .9f);
        draw->AddCircleFilled(point(.5f, .3f), stroke * .65f, color);
        line(.5f, .42f, .5f, .56f);
        break;
    case Icon::Note:
        draw->AddRect(point(.18f, .08f), point(.82f, .92f), color, 1.0f, 0, stroke);
        line(.32f, .33f, .68f, .33f); line(.32f, .5f, .68f, .5f); line(.32f, .67f, .59f, .67f);
        break;
    case Icon::Volume:
        line(.12f, .38f, .3f, .38f); line(.3f, .38f, .5f, .2f); line(.5f, .2f, .5f, .8f);
        line(.5f, .8f, .3f, .62f); line(.3f, .62f, .12f, .62f); line(.12f, .62f, .12f, .38f);
        draw->PathArcTo(point(.5f, .5f), size * .25f, -.8f, .8f, 12); draw->PathStroke(color, 0, stroke);
        draw->PathArcTo(point(.5f, .5f), size * .39f, -.8f, .8f, 12); draw->PathStroke(color, 0, stroke);
        break;
    case Icon::Network:
        draw->AddRect(point(.31f, .08f), point(.69f, .35f), color, 1.0f, 0, stroke);
        line(.5f, .35f, .5f, .57f); line(.2f, .57f, .8f, .57f);
        line(.2f, .57f, .2f, .68f); line(.8f, .57f, .8f, .68f);
        draw->AddRect(point(.06f, .68f), point(.34f, .91f), color, 1.0f, 0, stroke);
        draw->AddRect(point(.66f, .68f), point(.94f, .91f), color, 1.0f, 0, stroke);
        break;
    case Icon::Power:
        draw->PathArcTo(point(.5f, .53f), size * .34f, -.95f, 4.09f, 24);
        draw->PathStroke(color, 0, stroke);
        line(.5f, .08f, .5f, .49f);
        break;
    }
}

// A real ImGui button retains mouse, keyboard navigation, and focus feedback.
inline bool iconButton(const char* id, const char* label, Icon icon, ImVec2 size,
                       bool active = false, bool showLabel = false) {
    if (active) ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive));
    const bool pressed = ImGui::Button(id, size);
    if (active) ImGui::PopStyleColor();
    const ImVec2 minimum = ImGui::GetItemRectMin();
    const ImVec2 maximum = ImGui::GetItemRectMax();
    ImDrawList* draw = ImGui::GetWindowDrawList();
    const float iconSize = std::min(22.0f, size.y - 10.0f);
    drawIcon(draw, icon, ImVec2(minimum.x + (showLabel ? 9.0f : (size.x - iconSize) * .5f),
                              minimum.y + (size.y - iconSize) * .5f), iconSize, ImGui::GetColorU32(ImGuiCol_Text));
    if (showLabel) {
        draw->AddText(ImVec2(minimum.x + 38.0f, minimum.y + (size.y - ImGui::GetTextLineHeight()) * .5f),
                      ImGui::GetColorU32(ImGuiCol_Text), label);
    }
    if (active) draw->AddLine(ImVec2(minimum.x + 7.0f, maximum.y - 2.0f),
                              ImVec2(maximum.x - 7.0f, maximum.y - 2.0f),
                              ImGui::GetColorU32(ImGuiCol_Text), 2.0f);
    if (ImGui::IsItemHovered() || ImGui::IsItemFocused()) ImGui::SetTooltip("%s", label);
    return pressed;
}

} // namespace OS::Ui
