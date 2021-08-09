#pragma once
#include <random>

POINT cursor;

namespace Canvas
{
	void FrameworkInitialize(const char* str_id, bool* v);
	void CheckButton(const char* str_id, bool* v);
	bool Button(const char* label, const ImVec2& size_arg);
	void Text(const char* str_id);
	void SameLine();
	bool SmoothSlider(int x, int y, int position, std::string string, float& value, float min_value, float max_value);
	void CostumSlider(std::int32_t x, std::int32_t y, std::int32_t position);
	void Checkbox(const char* str_id, bool* v);
}

void Canvas::FrameworkInitialize(const char* str_id, bool* v)
{
	ImVec2 p = ImGui::GetCursorScreenPos();
	ImDrawList* draw_list = ImGui::GetWindowDrawList();

	float height = ImGui::GetFrameHeight() - 5;
	float width = height * 1.80f;
	float radius = height * 0.50f;

	ImGui::InvisibleButton(str_id, ImVec2(width, height));
	if (ImGui::IsItemClicked())
		*v = !*v;

	float t = *v ? 1.0f : 0.0f;

	ImGuiContext& g = *GImGui;
	float ANIM_SPEED = 0.05f;
	if (g.ActiveId == g.CurrentWindow->GetID(str_id))// && g.LastActiveIdTimer < ANIM_SPEED)
	{
		float t_anim = ImSaturate(g.ActiveIdTimer / ANIM_SPEED);
		t = *v ? (t_anim) : (1.0f - t_anim);
	}

	ImU32 col_bg;
	if (ImGui::IsItemHovered())
		col_bg = ImGui::GetColorU32(ImLerp(ImVec4(0.85f, 0.85f, 0.85f, 1.0f), ImVec4(0.98f, 0.26f, 0.26f, 0.30f), t));
	else
		col_bg = ImGui::GetColorU32(ImLerp(ImVec4(0.85f, 0.85f, 0.85f, 1.0f), ImVec4(0.98f, 0.26f, 0.26f, 0.30f), t));

	draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), col_bg, height * 0.5f);
	draw_list->AddCircleFilled(ImVec2(p.x + radius + t * (width - radius * 2.0f), p.y + radius), radius - 1.5f, IM_COL32(255, 255, 255, 255));
}

void Canvas::Text(const char* str_id)
{
	ImGui::Text(str_id);
}

void Canvas::SameLine()
{
	ImGui::SameLine();
}

void Canvas::CheckButton(const char* str_id, bool* v)
{
	ImGui::Text("  ");
	ImGui::SameLine();
	Canvas::FrameworkInitialize(str_id, v);
	ImGui::SameLine();
	ImGui::Text(str_id);
}

bool Canvas::Button(const char* label, const ImVec2& size_arg)
{
	ImGui::Button(label, size_arg);
	return false;
}

void Canvas::Checkbox(const char* str_id, bool* v)
{
	ImGui::Checkbox(str_id, v);
}