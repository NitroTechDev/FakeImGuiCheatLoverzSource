struct g_p
{
	bool memory = true;
	bool corner = false;
	bool treedbox = false;

	bool lines = false;
	bool aimbot_fov = true;
	bool crosshair = true;
	bool silentAim = false;

	bool debug_info = false;
	bool loot_esp = true;

	bool box = false;
	bool outlined = false;

	bool distance = false;
	bool shadow = true;
	bool weapon = false;

	bool name = false;
	bool radar = true;
	bool SuS = false;

	float smoothness = 0;
	float radius = 250;

	bool menu = true;
	int tabs;

	bool debug = false;

	bool vischeck = true;

	bool skeleton = false;

	bool filled = false;

	bool nospread = false;
	bool speedhack = false;
	bool noreload = false;
	bool NoBloom = false;

	bool selfesp = false;
	bool items = false;

	bool llama = false;
	bool chest = false;
	bool vehicle = false;
	bool bandage = false;
	bool medkit = false;
	bool shieldpotion = false;
	bool supplydrop = false;

	bool botai = false;
	bool stickysilent = false;
	INT MinWeaponTier = 1;

	bool ammo = false;
	bool weakspotaim = false;

	bool AimWhileJumping = false;

	bool TEST = false;
	bool HitBoxPos = false;
	bool hitbox = false;

	bool InstantRevive = false;
};
g_p settings;
#include <sstream>

namespace fn
{
	namespace general_overlay
	{
		std::string string_To_UTF8(const std::string& str)
		{
			int nwLen = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);
			wchar_t* pwBuf = new wchar_t[nwLen + 1];
			ZeroMemory(pwBuf, nwLen * 2 + 2);
			::MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.length(), pwBuf, nwLen);
			int nLen = ::WideCharToMultiByte(CP_UTF8, 0, pwBuf, -1, NULL, NULL, NULL, NULL);
			char* pBuf = new char[nLen + 1];
			ZeroMemory(pBuf, nLen + 1);
			::WideCharToMultiByte(CP_UTF8, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);
			std::string retStr(pBuf);
			delete[]pwBuf;
			delete[]pBuf;
			pwBuf = NULL;
			pBuf = NULL;
			return retStr;
		}
		// Drawings for custom menus + Toggle Button's / Checkboxes ET
		void RegularRGBText(int x, int y, ImColor color, const char* str)
		{
			ImFont a;
			std::string utf_8_1 = std::string(str);
			std::string utf_8_2 = string_To_UTF8(utf_8_1);
			ImGui::GetOverlayDrawList()->AddText(ImVec2(x, y), ImColor(color), utf_8_2.c_str());
		}
		void Test(int x, int y, ImColor color, const char* str)
		{
			ImFont a;
			std::string utf_8_1 = std::string(str);
			std::string utf_8_2 = string_To_UTF8(utf_8_1);
			ImGui::GetOverlayDrawList()->AddText(ImVec2(x, y), ImGui::ColorConvertFloat4ToU32(ImVec4(color)), utf_8_2.c_str());
		}
		void ShadowRGBText(int x, int y, ImColor color, const char* str)
		{
			ImFont a;
			std::string utf_8_1 = std::string(str);
			std::string utf_8_2 = string_To_UTF8(utf_8_1);
			ImGui::GetOverlayDrawList()->AddText(ImVec2(x + 2, y + 2), ImGui::ColorConvertFloat4ToU32(ImColor(0, 0, 0, 240)), utf_8_2.c_str());
			ImGui::GetOverlayDrawList()->AddText(ImVec2(x + 2, y + 2), ImGui::ColorConvertFloat4ToU32(ImColor(0, 0, 0, 240)), utf_8_2.c_str());
			ImGui::GetOverlayDrawList()->AddText(ImVec2(x, y), ImGui::ColorConvertFloat4ToU32(ImVec4(color)), utf_8_2.c_str());
		}
		void OutlinedRBGText(int x, int y, ImColor color,std::string str)
		{
			ImFont a;
			std::string utf_8_1 = std::string(str);
			std::string utf_8_2 = string_To_UTF8(utf_8_1);
			ImGui::GetOverlayDrawList()->AddText(ImVec2(x + 1, y + 2), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 30 / 30.0)), utf_8_2.c_str());
			ImGui::GetOverlayDrawList()->AddText(ImVec2(x + 1, y + 2), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 30 / 30.0)), utf_8_2.c_str());
			ImGui::GetOverlayDrawList()->AddText(ImVec2(x - 1, y - 2), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 30 / 30.0)), utf_8_2.c_str());
			ImGui::GetOverlayDrawList()->AddText(ImVec2(x - 1, y - 2), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 30 / 30.0)), utf_8_2.c_str());
			ImGui::GetOverlayDrawList()->AddText(ImVec2(x, y), ImGui::ColorConvertFloat4ToU32(ImVec4(color)), utf_8_2.c_str());
			ImGui::GetOverlayDrawList()->AddText(ImVec2(x, y), ImGui::ColorConvertFloat4ToU32(ImVec4(color)), utf_8_2.c_str());
		}
		void Rect(int x, int y, int w, int h, ImColor color, int thickness)
		{
			ImGui::GetOverlayDrawList()->AddRect(ImVec2(x, y), ImVec2(x + w, y + h), ImGui::ColorConvertFloat4ToU32(ImColor(0, 0, 0)), 0, 0, thickness + 0.5f);
			ImGui::GetOverlayDrawList()->AddRect(ImVec2(x, y), ImVec2(x + w, y + h), ImGui::ColorConvertFloat4ToU32(ImColor(color)), 0, 0, thickness);
		}
		void FilledRect(int x, int y, int w, int h, ImColor color)
		{
			ImGui::GetOverlayDrawList()->AddRectFilled(ImVec2(x, y), ImVec2(x + w, y + h), ImGui::ColorConvertFloat4ToU32(ImVec4(color)), 0, 0);
		}

		void OutlinedString(std::string str, ImVec2 loc, ImU32 colr, bool centered = false)
		{
			ImGuiWindow* window = ImGui::GetCurrentWindow();
			ImVec2 size = { 0,0 };
			float minx = 0;
			float miny = 0;
			if (centered)
			{
				size = ImGui::GetFont()->CalcTextSizeA(window->DrawList->_Data->FontSize, 0x7FFFF, 0, str.c_str());
				minx = size.x / 2.f;
				miny = size.y / 2.f;
			}

			window->DrawList->AddText(ImVec2((loc.x - 1) - minx, (loc.y - 1) - miny), ImGui::GetColorU32({ 0.f, 0.f, 0.f, 1.f }), str.c_str());
			window->DrawList->AddText(ImVec2((loc.x + 1) - minx, (loc.y + 1) - miny), ImGui::GetColorU32({ 0.f, 0.f, 0.f, 1.f }), str.c_str());
			window->DrawList->AddText(ImVec2((loc.x + 1) - minx, (loc.y - 1) - miny), ImGui::GetColorU32({ 0.f, 0.f, 0.f, 1.f }), str.c_str());
			window->DrawList->AddText(ImVec2((loc.x - 1) - minx, (loc.y + 1) - miny), ImGui::GetColorU32({ 0.f, 0.f, 0.f, 1.f }), str.c_str());
			window->DrawList->AddText(ImVec2(loc.x - minx, loc.y - miny), colr, str.c_str());
		}
		float DrawOutlinedText(ImFont* pFont, const std::string& text, const ImVec2& pos, float size, ImU32 color, bool center)
		{
			ImGuiWindow* window = ImGui::GetCurrentWindow();

			std::stringstream stream(text);
			std::string line;

			float y = 0.0f;
			int i = 0;

			while (std::getline(stream, line))
			{
				ImVec2 textSize = pFont->CalcTextSizeA(size, FLT_MAX, 0.0f, line.c_str());

				if (center)
				{
					window->DrawList->AddText(pFont, size, ImVec2((pos.x - textSize.x / 2.0f) + 1, (pos.y + textSize.y * i) + 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), line.c_str());
					window->DrawList->AddText(pFont, size, ImVec2((pos.x - textSize.x / 2.0f) - 1, (pos.y + textSize.y * i) - 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), line.c_str());
					window->DrawList->AddText(pFont, size, ImVec2((pos.x - textSize.x / 2.0f) + 1, (pos.y + textSize.y * i) - 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), line.c_str());
					window->DrawList->AddText(pFont, size, ImVec2((pos.x - textSize.x / 2.0f) - 1, (pos.y + textSize.y * i) + 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), line.c_str());

					window->DrawList->AddText(pFont, size, ImVec2(pos.x - textSize.x / 2.0f, pos.y + textSize.y * i), ImGui::GetColorU32(color), line.c_str());
				}
				else
				{
					window->DrawList->AddText(pFont, size, ImVec2((pos.x) + 1, (pos.y + textSize.y * i) + 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), line.c_str());
					window->DrawList->AddText(pFont, size, ImVec2((pos.x) - 1, (pos.y + textSize.y * i) - 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), line.c_str());
					window->DrawList->AddText(pFont, size, ImVec2((pos.x) + 1, (pos.y + textSize.y * i) - 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), line.c_str());
					window->DrawList->AddText(pFont, size, ImVec2((pos.x) - 1, (pos.y + textSize.y * i) + 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), line.c_str());

					window->DrawList->AddText(pFont, size, ImVec2(pos.x, pos.y + textSize.y * i), ImGui::GetColorU32(color), line.c_str());
				}

				y = pos.y + textSize.y * (i + 1);
				i++;
			}
			return y;
		}
		float DrawNormalText(ImFont* pFont, const std::string& text, const ImVec2& pos, float size, ImU32 color, bool center)
		{
			ImGuiWindow* window = ImGui::GetCurrentWindow();

			std::stringstream stream(text);
			std::string line;

			float y = 0.0f;
			int i = 0;

			while (std::getline(stream, line))
			{
				ImVec2 textSize = pFont->CalcTextSizeA(size, FLT_MAX, 0.0f, line.c_str());

				if (center)
				{
					window->DrawList->AddText(pFont, size, ImVec2(pos.x - textSize.x / 2.0f, pos.y + textSize.y * i), ImGui::GetColorU32(color), line.c_str());
				}
				else
				{
					window->DrawList->AddText(pFont, size, ImVec2(pos.x, pos.y + textSize.y * i), ImGui::GetColorU32(color), line.c_str());
				}

				y = pos.y + textSize.y * (i + 1);
				i++;
			}
			return y;
		}
		void DrawLine(int x1, int y1, int x2, int y2, const ImU32 color, int thickness)
		{
			ImGui::GetOverlayDrawList()->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), ImGui::GetColorU32(color), thickness);
		}
		void DrawCorneredBox(int X, int Y, int W, int H, ImColor color, int thickness) {
			float lineW = (W / 3);
			float lineH = (H / 3);

			DrawLine(X, Y, X, Y + lineH, color, thickness);
			DrawLine(X, Y, X + lineW, Y, color, thickness);
			DrawLine(X + W - lineW, Y, X + W, Y, color, thickness);
			DrawLine(X + W, Y, X + W, Y + lineH, color, thickness);
			DrawLine(X, Y + H - lineH, X, Y + H, color, thickness);
			DrawLine(X, Y + H, X + lineW, Y + H, color, thickness);
			DrawLine(X + W - lineW, Y + H, X + W, Y + H, color, thickness);
			DrawLine(X + W, Y + H - lineH, X + W, Y + H, color, thickness);
		}
		void Outline(int X, int Y, int W, int H, ImColor color) {
			float lineW = (W / 3);
			float lineH = (H / 3);

			DrawLine(X, Y, X, Y + lineH, color, 2);
			DrawLine(X, Y, X + lineW, Y, color, 2);
			DrawLine(X + W - lineW, Y, X + W, Y, color, 2);
			DrawLine(X + W, Y, X + W, Y + lineH, color, 2);
			DrawLine(X, Y + H - lineH, X, Y + H, color, 2);
			DrawLine(X, Y + H, X + lineW, Y + H, color, 2);
			DrawLine(X + W - lineW, Y + H, X + W, Y + H, color, 2);
			DrawLine(X + W, Y + H - lineH, X + W, Y + H, color, 2);


		}
		void OutlineBlack(int X, int Y, int W, int H, ImColor color) {
			float lineW = (W / 3);
			float lineH = (H / 3);

			ImGui::GetOverlayDrawList()->AddRect(ImVec2(X, Y), ImVec2(X + W, Y + H), ImGui::ColorConvertFloat4ToU32(ImVec4(color)), 0, 0, 1.5f);
		}
	}
	void keys();
	bool actorloop(ImGuiWindow& window);
	void menu();
	void render(ImGuiWindow& window);
}


void RectFilled(int x, int y, int w, int h, ImColor color)
{
	ImGui::GetOverlayDrawList()->AddRectFilled(ImVec2(x, y), ImVec2(x + w, y + h), ImGui::ColorConvertFloat4ToU32(ImVec4(color)), 0, 0);
}
void ShadowText(int posx, int posy, ImColor clr, const char* text)
{
	ImGui::GetOverlayDrawList()->AddText(ImVec2(posx + 1, posy + 2), ImColor(0, 0, 0, 200), text);
	ImGui::GetOverlayDrawList()->AddText(ImVec2(posx + 1, posy + 2), ImColor(0, 0, 0, 200), text);
	ImGui::GetOverlayDrawList()->AddText(ImVec2(posx, posy), ImColor(clr), text);
}
void Rect(int x, int y, int w, int h, ImColor color, int thickness)
{
	ImGui::GetOverlayDrawList()->AddRect(ImVec2(x, y), ImVec2(x + w, y + h), ImGui::ColorConvertFloat4ToU32(ImVec4(color)), 0, 0, thickness);
}
namespace PS
{
	void CL_ToggleButton(const char* v, bool* option, float x, float y, float x2)
	{
		ImGui::SetCursorPos({ x + x2, y });

		ImVec2 pos = ImGui::GetWindowPos();
		float height = 20;
		float width = 40;

		if (ImGui::Button(v, ImVec2{ width, height }))
			*option = !*option;

		Rect(pos.x + x + x2, pos.y + y, 40, 20, ImColor(50, 50, 50), 2);
		RectFilled(pos.x + x + x2, pos.y + y, 40, 20, ImColor(100, 100, 100, 255));
		ShadowText(pos.x + x - 5, pos.y + y, ImColor(255, 187, 0, 200), v);

		if (*option)
		{
			RectFilled(pos.x + x + x2, pos.y + y, 20, 20, ImColor(0, 200, 0, 255));
		}
		else
		{
			RectFilled(pos.x + x + 20 + x2, pos.y + y, 20, 20, ImColor(200, 0, 0, 255));
		}
	}
}

namespace signatures {
	const char* Uworld_Sig = "48 8B 05 ? ? ? ? 4D 8B C2"; //uworld

	const char* Gobject_Sig = "48 8B 05 ? ? ? ? 48 8B 0C C8 48 8B 04 D1";

	const char* Free_Sig = "48 85 C9 0F 84 ? ? ? ? 53 48 83 EC 20 48 89 7C 24 30 48 8B D9 48 8B 3D ? ? ? ? 48 85 FF 0F 84 ? ? ? ? 48 8B 07 4C 8B 40 30 48 8D 05 ? ? ? ? 4C 3B C0"; //Free

	const char* ProjectWorldToScreen_Sig = "E8 ? ? ? ? 41 88 07 48 83 C4 30"; //ProjectWorldToScreen

	const char* LineOfSightTo_Sig = "E8 ? ? ? ? 48 8B 0D ? ? ? ? 33 D2 40 8A F8"; //LineOfSight

	const char* GetNameByIndex_Sig = "48 89 5C 24 ? 48 89 74 24 ? 55 57 41 56 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 45 33 F6 48 8B F2 44 39 71 04 0F 85 ? ? ? ? 8B 19 0F B7 FB E8 ? ? ? ? 8B CB 48 8D 54 24"; //GetNameByIndex

	const char* BoneMatrix_Sig = "E8 ? ? ? ? 48 8B 47 30 F3 0F 10 45"; //BoneMatrix

    const char* DiscordPresentScene_sig = "56 57 53 48 83 EC 30 44 89 C6"; //Discord PresentScene
}