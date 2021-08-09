#include "UnrealStructs.h"
#include "Canvas.h"

#define PI (3.141592653589793f)

#include "detours.h"
#include "UnrealStructs.h"

#include <intrin.h>

#pragma comment(lib, "detours.lib")

//#include "SpeedHack.h"
// TODO: put in another file, and rename to something better
template<class DataType>
class SpeedHack {
	DataType time_offset;
	DataType time_last_update;

	double speed_;

public:
	SpeedHack(DataType currentRealTime, double initialSpeed) {
		time_offset = currentRealTime;
		time_last_update = currentRealTime;

		speed_ = initialSpeed;
	}

	// TODO: put lock around for thread safety
	void setSpeed(DataType currentRealTime, double speed) {
		time_offset = getCurrentTime(currentRealTime);
		time_last_update = currentRealTime;

		speed_ = speed;
	}

	// TODO: put lock around for thread safety
	DataType getCurrentTime(DataType currentRealTime) {
		DataType difference = currentRealTime - time_last_update;

		return (DataType)(speed_ * difference) + time_offset;
	}
};


// function signature typedefs
typedef DWORD(WINAPI* GetTickCountType)(void);
typedef ULONGLONG(WINAPI* GetTickCount64Type)(void);

typedef BOOL(WINAPI* QueryPerformanceCounterType)(LARGE_INTEGER* lpPerformanceCount);

// globals
GetTickCountType   g_GetTickCountOriginal;
GetTickCount64Type g_GetTickCount64Original;
GetTickCountType   g_TimeGetTimeOriginal;    // Same function signature as GetTickCount

QueryPerformanceCounterType g_QueryPerformanceCounterOriginal;


const double kInitialSpeed = 1.0; // initial speed hack speed

//                                  (initialTime,      initialSpeed)
SpeedHack<DWORD>     g_speedHack(GetTickCount(), kInitialSpeed);
SpeedHack<ULONGLONG> g_speedHackULL(GetTickCount64(), kInitialSpeed);
SpeedHack<LONGLONG>  g_speedHackLL(0, kInitialSpeed); // Gets set properly in DllMain

// function prototypes

DWORD     WINAPI GetTickCountHacked(void);
ULONGLONG WINAPI GetTickCount64Hacked(void);

BOOL      WINAPI QueryPerformanceCounterHacked(LARGE_INTEGER* lpPerformanceCount);

DWORD     WINAPI KeysThread(LPVOID lpThreadParameter);

// functions

namespace L {
	void MainGay()
	{
		// TODO: split up this function for readability.

		HMODULE kernel32 = GetModuleHandleA(xorthis("Kernel32.dll"));
		HMODULE winmm = GetModuleHandleA(xorthis("Winmm.dll"));

		// TODO: check if the modules are even loaded.

		// Get all the original addresses of target functions
		g_GetTickCountOriginal = (GetTickCountType)GetProcAddress(kernel32, xorthis("GetTickCount"));
		g_GetTickCount64Original = (GetTickCount64Type)GetProcAddress(kernel32, xorthis("GetTickCount64"));

		g_TimeGetTimeOriginal = (GetTickCountType)GetProcAddress(winmm, xorthis("timeGetTime"));

		g_QueryPerformanceCounterOriginal = (QueryPerformanceCounterType)GetProcAddress(kernel32, xorthis("QueryPerformanceCounter"));

		// Setup the speed hack object for the Performance Counter
		LARGE_INTEGER performanceCounter;
		g_QueryPerformanceCounterOriginal(&performanceCounter);

		g_speedHackLL = SpeedHack<LONGLONG>(performanceCounter.QuadPart, kInitialSpeed);

		// Detour functions
		DetourTransactionBegin();

		DetourAttach((PVOID*)&g_GetTickCountOriginal, (PVOID)GetTickCountHacked);
		DetourAttach((PVOID*)&g_GetTickCount64Original, (PVOID)GetTickCount64Hacked);

		// Detour timeGetTime to the hacked GetTickCount (same signature)
		DetourAttach((PVOID*)&g_TimeGetTimeOriginal, (PVOID)GetTickCountHacked);

		DetourAttach((PVOID*)&g_QueryPerformanceCounterOriginal, (PVOID)QueryPerformanceCounterHacked);

		DetourTransactionCommit();
	}
}

void setAllToSpeed(double speed) {
	g_speedHack.setSpeed(g_GetTickCountOriginal(), speed);

	g_speedHackULL.setSpeed(g_GetTickCount64Original(), speed);

	LARGE_INTEGER performanceCounter;
	g_QueryPerformanceCounterOriginal(&performanceCounter);

	g_speedHackLL.setSpeed(performanceCounter.QuadPart, speed);
}

DWORD WINAPI GetTickCountHacked(void) {
	return g_speedHack.getCurrentTime(g_GetTickCountOriginal());
}

ULONGLONG WINAPI GetTickCount64Hacked(void) {
	return g_speedHackULL.getCurrentTime(g_GetTickCount64Original());
}

BOOL WINAPI QueryPerformanceCounterHacked(LARGE_INTEGER* lpPerformanceCount) {
	LARGE_INTEGER performanceCounter;

	BOOL result = g_QueryPerformanceCounterOriginal(&performanceCounter);

	lpPerformanceCount->QuadPart = g_speedHackLL.getCurrentTime(performanceCounter.QuadPart);

	return result;
}

int x = 30;
int y = 20;

void fn::keys()
{
	if (GetAsyncKeyState(VK_F8) & 1) {
		settings.menu = !settings.menu;
	}
	if (GetAsyncKeyState(VK_PRIOR) & 1) {
		if (settings.MinWeaponTier == 5)
			return;
		settings.MinWeaponTier += 1;
	}
	if (GetAsyncKeyState(VK_NEXT) & 1) {
		if (settings.MinWeaponTier == 1)
			return;
		settings.MinWeaponTier -= 1;
	}
}

bool firstS = false;

BOOL valid_pointer(DWORD64 address)
{
	if (!IsBadWritePtr((LPVOID)address, (UINT_PTR)8)) return TRUE;
	else return FALSE;
}

void RadarRange(float* x, float* y, float range)
{
	if (fabs((*x)) > range || fabs((*y)) > range)
	{
		if ((*y) > (*x))
		{
			if ((*y) > -(*x))
			{
				(*x) = range * (*x) / (*y);
				(*y) = range;
			}
			else
			{
				(*y) = -range * (*y) / (*x);
				(*x) = -range;
			}
		}
		else
		{
			if ((*y) > -(*x))
			{
				(*y) = range * (*y) / (*x);
				(*x) = range;
			}
			else
			{
				(*x) = -range * (*x) / (*y);
				(*y) = -range;
			}
		}
	}
}

void CalcRadarPoint(SDK::Structs::FVector vOrigin, int& screenx, int& screeny)
{
	SDK::Structs::FRotator vAngle = SDK::Structs::FRotator{ SDK::Utilities::CamRot.x, SDK::Utilities::CamRot.y, SDK::Utilities::CamRot.z };
	auto fYaw = vAngle.Yaw * PI / 180.0f;
	float dx = vOrigin.X - SDK::Utilities::CamLoc.x;
	float dy = vOrigin.Y - SDK::Utilities::CamLoc.y;

	float fsin_yaw = sinf(fYaw);
	float fminus_cos_yaw = -cosf(fYaw);

	float x = dy * fminus_cos_yaw + dx * fsin_yaw;
	x = -x;
	float y = dx * fminus_cos_yaw - dy * fsin_yaw;

	float range = (float)15.f;

	RadarRange(&x, &y, range);

	ImVec2 DrawPos = ImGui::GetCursorScreenPos();
	ImVec2 DrawSize = ImGui::GetContentRegionAvail();

	int rad_x = (int)DrawPos.x;
	int rad_y = (int)DrawPos.y;

	float r_siz_x = DrawSize.x;
	float r_siz_y = DrawSize.y;

	int x_max = (int)r_siz_x + rad_x - 5;
	int y_max = (int)r_siz_y + rad_y - 5;

	screenx = rad_x + ((int)r_siz_x / 2 + int(x / range * r_siz_x));
	screeny = rad_y + ((int)r_siz_y / 2 + int(y / range * r_siz_y));

	if (screenx > x_max)
		screenx = x_max;

	if (screenx < rad_x)
		screenx = rad_x;

	if (screeny > y_max)
		screeny = y_max;

	if (screeny < rad_y)
		screeny = rad_y;
}

SDK::Structs::FVector* GetPawnRootLocations(uintptr_t pawn) {
	auto root = SDK::Utilities::read<uintptr_t>(pawn + SDK::Classes::StaticOffsets::RootComponent);
	if (!root) {
		return nullptr;
	}
	return reinterpret_cast<SDK::Structs::FVector*>(reinterpret_cast<PBYTE>(root) + SDK::Classes::StaticOffsets::RelativeLocation);

}

void renderRadar(uintptr_t CurrentActor, ImColor PlayerPointColor) {

	auto player = CurrentActor;

	int screenx = 0;
	int screeny = 0;

	SDK::Structs::FVector pos = *GetPawnRootLocations(CurrentActor);

	CalcRadarPoint(pos, screenx, screeny);

	ImDrawList* Draw = ImGui::GetOverlayDrawList();

	SDK::Structs::FVector viewPoint = { 0 };
	Draw->AddRectFilled(ImVec2((float)screenx, (float)screeny), ImVec2((float)screenx + 5, (float)screeny + 5), ImColor(PlayerPointColor));
}

inline bool custom_UseFontShadow;
inline unsigned int custom_FontShadowColor;

inline static void PushFontShadow(unsigned int col)
{
	custom_UseFontShadow = true;
	custom_FontShadowColor = col;
}

inline static void PopFontShadow(void)
{
	custom_UseFontShadow = false;
}

void RadarLoop(uintptr_t CurrentActor, ImColor PlayerPointColor)
{
	ImGuiWindowFlags TargetFlags;
	if (settings.menu)
		TargetFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;
	else
		TargetFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;
	
	float radarWidth = 200;
	float PosDx = 1200;
	float PosDy = 60;

	if (!firstS) {
		ImGui::SetNextWindowPos(ImVec2{ 1200, 60 }, ImGuiCond_Once);
		firstS = true;
	}

	auto* colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.53f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
	colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.29f, 0.48f, 0.54f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.29f, 0.48f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_Separator] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.20f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowRounding = 0.0f;
	style.ChildRounding = 0.0f;
	style.FrameRounding = 0.0f;


	if (ImGui::Begin("black", 0, ImVec2(250, 255), -1.f, TargetFlags)) {

		ImDrawList* Draw = ImGui::GetOverlayDrawList();
		ImVec2 DrawPos = ImGui::GetCursorScreenPos();
		ImVec2 DrawSize = ImGui::GetContentRegionAvail();

		ImVec2 midRadar = ImVec2(DrawPos.x + (DrawSize.x / 2), DrawPos.y + (DrawSize.y / 2));
		ImGui::GetWindowDrawList()->AddLine(ImVec2(midRadar.x - DrawSize.x / 2.f, midRadar.y), ImVec2(midRadar.x + DrawSize.x / 2.f, midRadar.y), ImColor(95, 95, 95));
		ImGui::GetWindowDrawList()->AddLine(ImVec2(midRadar.x, midRadar.y - DrawSize.y / 2.f), ImVec2(midRadar.x, midRadar.y + DrawSize.y / 2.f), ImColor(95, 95, 95));

		if (valid_pointer(PlayerController) && valid_pointer(PlayerCameraManager) && SDK::Utilities::CheckInScreen(CurrentActor, SDK::Utilities::CamLoc.x, SDK::Utilities::CamLoc.y)) { renderRadar(CurrentActor, PlayerPointColor); }
	}
	ImGui::End();
}

void DrawLine(int x1, int y1, int x2, int y2, ImColor color, int thickness)
{
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), ImGui::ColorConvertFloat4ToU32(ImColor(color)), thickness);
}

std::string TxtFormat(const char* format, ...)
{
	va_list argptr;
	va_start(argptr, format);

	char buffer[2048];
	vsprintf(buffer, format, argptr);

	va_end(argptr);

	return buffer;
}

float width;
float height;
float X1 = GetSystemMetrics(0) / 2 - 1;
float Y1 = GetSystemMetrics(1) / 2 - 1;

#define ITEM_COLOR_MEDS ImVec4{ 0.9f, 0.55f, 0.55f, 0.95f }
#define ITEM_COLOR_SHIELDPOTION ImVec4{ 0.35f, 0.55f, 0.85f, 0.95f }
#define ITEM_COLOR_CHEST ImVec4{ 0.95f, 0.95f, 0.0f, 0.95f }
#define ITEM_COLOR_SUPPLYDROP ImVec4{ 0.9f, 0.1f, 0.1f, 0.9f }

inline float calc_distance(SDK::Structs::Vector3 camera_location, SDK::Structs::FVector pawn)
{
	float x = camera_location.x - pawn.X;
	float y = camera_location.y - pawn.Y;
	float z = camera_location.z - pawn.Z;
	float distance = sqrtf((x * x) + (y * y) + (z * z)) / 100.0f;
	return distance;
}

#define ReadPointer(base, offset) (*(PVOID *)(((PBYTE)base + offset)))
#define ReadDWORD(base, offset) (*(DWORD *)(((PBYTE)base + offset)))
#define ReadDWORD_PTR(base, offset) (*(DWORD_PTR *)(((PBYTE)base + offset)))
#define ReadFTRANSFORM(base, offset) (*(FTransform *)(((PBYTE)base + offset)))
#define ReadInt(base, offset) (*(int *)(((PBYTE)base + offset)))
#define ReadFloat(base, offset) (*(float *)(((PBYTE)base + offset)))
#define Readuintptr_t(base, offset) (*(uintptr_t *)(((PBYTE)base + offset)))
#define Readuint64_t(base, offset) (*(uint64_t *)(((PBYTE)base + offset)))
#define ReadVector3(base, offset) (*(Vector3 *)(((PBYTE)base + offset))

float ReloadNormal = 0.0f;
float ReloadTime = 0.0f;

float DrawOutlinedText(const std::string& text, const ImVec2& pos, ImU32 color, bool center)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();

	std::stringstream stream(text);
	std::string line;

	float y = 0.0f;
	int i = 0;

	while (std::getline(stream, line))
	{
		ImVec2 textSize = ImGui::GetFont()->CalcTextSizeA(ImGui::GetOverlayDrawList()->_Data->FontSize, FLT_MAX, 0, line.c_str());


		if (center)
		{
			window->DrawList->AddText(ImVec2((pos.x - textSize.x / 2.0f) + 1, (pos.y + textSize.y * i) + 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), line.c_str());
			window->DrawList->AddText(ImVec2((pos.x - textSize.x / 2.0f) - 1, (pos.y + textSize.y * i) - 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), line.c_str());
			window->DrawList->AddText(ImVec2((pos.x - textSize.x / 2.0f) + 1, (pos.y + textSize.y * i) - 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), line.c_str());
			window->DrawList->AddText(ImVec2((pos.x - textSize.x / 2.0f) - 1, (pos.y + textSize.y * i) + 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), line.c_str());

			window->DrawList->AddText(ImVec2(pos.x - textSize.x / 2.0f, pos.y + textSize.y * i), ImGui::GetColorU32(color), line.c_str());
		}
		else
		{
			window->DrawList->AddText(ImVec2((pos.x) + 1, (pos.y + textSize.y * i) + 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), line.c_str());
			window->DrawList->AddText(ImVec2((pos.x) - 1, (pos.y + textSize.y * i) - 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), line.c_str());
			window->DrawList->AddText(ImVec2((pos.x) + 1, (pos.y + textSize.y * i) - 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), line.c_str());
			window->DrawList->AddText(ImVec2((pos.x) - 1, (pos.y + textSize.y * i) + 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), line.c_str());

			window->DrawList->AddText(ImVec2(pos.x, pos.y + textSize.y * i), ImGui::GetColorU32(color), line.c_str());
		}

		y = pos.y + textSize.y * (i + 1);
		i++;
	}
	return y;
}

bool distancegay = true;

#define ITEM_COLOR_TIER_WHITE ImVec4{ 0.8f, 0.8f, 0.8f, 0.95f }
#define ITEM_COLOR_TIER_GREEN ImVec4{ 0.0f, 0.95f, 0.0f, 0.95f }
#define ITEM_COLOR_TIER_BLUE ImVec4{ 0.2f, 0.4f, 1.0f, 0.95f }
#define ITEM_COLOR_TIER_PURPLE ImVec4{ 0.7f, 0.25f, 0.85f, 0.95f }
#define ITEM_COLOR_TIER_ORANGE ImVec4{ 0.85f, 0.65f, 0.0f, 0.95f }
#define ITEM_COLOR_TIER_GOLD ImVec4{ 0.95f, 0.85f, 0.45f, 0.95f }
#define ITEM_COLOR_TIER_UNKNOWN ImVec4{ 1.0f, 0.0f, 1.0f, 0.95f }

ImVec4 GetItemColor(BYTE tier)
{
	switch (tier)
	{
	case 1:
		return ITEM_COLOR_TIER_WHITE;
	case 2:
		return ITEM_COLOR_TIER_GREEN;
	case 3:
		return ITEM_COLOR_TIER_BLUE;
	case 4:
		return ITEM_COLOR_TIER_PURPLE;
	case 5:
		return ITEM_COLOR_TIER_ORANGE;
	case 6:
	case 7:
		return ITEM_COLOR_TIER_GOLD;
	case 8:
	case 9:
		return ImVec4{ 200 / 255.f, 0 / 255.f, 0 / 255.f, 0.95f };
	case 10:
		return ITEM_COLOR_TIER_UNKNOWN;
	default:
		return ITEM_COLOR_TIER_WHITE;
	}
}

int __forceinline GetDistanceMeters(SDK::Structs::Vector3 location, SDK::Structs::Vector3 CurrentActor)
{
	return (int)(location.Distance(CurrentActor) / 100);
}

bool fn::actorloop(ImGuiWindow& window)
{
	uintptr_t MyTeamIndex;
	uintptr_t EnemyTeamIndex;
	float FOVmax = 9999.f;

	X = SDK::Utilities::SpoofCall(GetSystemMetrics, SM_CXSCREEN);
	Y = SDK::Utilities::SpoofCall(GetSystemMetrics, SM_CYSCREEN);

	uintptr_t GWorld = SDK::Utilities::read<uintptr_t>(Details.UWORLD);
	if (!GWorld) return false;

	uintptr_t Gameinstance = SDK::Utilities::read<uint64_t>(GWorld + SDK::Classes::StaticOffsets::OwningGameInstance);
	if (!Gameinstance) return false;

	uintptr_t LocalPlayer = SDK::Utilities::read<uint64_t>(Gameinstance + SDK::Classes::StaticOffsets::LocalPlayers);
	if (!LocalPlayer) return false;

	uintptr_t LocalPlayers = SDK::Utilities::read<uint64_t>(LocalPlayer);
	if (!LocalPlayers) return false;

	PlayerController = SDK::Utilities::read<uint64_t>(LocalPlayers + SDK::Classes::StaticOffsets::PlayerController);
	if (!PlayerController) return false;

	PlayerCameraManager = SDK::Utilities::read<uintptr_t>(PlayerController + SDK::Classes::StaticOffsets::PlayerCameraManager);
	if (!PlayerCameraManager) return false;

	LocalPawn = SDK::Utilities::read<uint64_t>(PlayerController + SDK::Classes::StaticOffsets::AcknowledgedPawn);
	//if (!LocalPawn) return false;

	uintptr_t Ulevel = SDK::Utilities::read<uintptr_t>(GWorld + SDK::Classes::StaticOffsets::PersistentLevel);
	if (!Ulevel) return false;

	uintptr_t AActors = SDK::Utilities::read<uintptr_t>(Ulevel + SDK::Classes::StaticOffsets::AActors);
	if (!AActors) return false;

	uintptr_t ActorCount = SDK::Utilities::read<int>(Ulevel + SDK::Classes::StaticOffsets::ActorCount);
	if (!ActorCount) return false;

	GetPlayerViewPoint(PlayerCameraManager, &SDK::Utilities::CamLoc, &SDK::Utilities::CamRot);

	for (int i = 0; i < ActorCount; i++) {

		uintptr_t CurrentActor = SDK::Utilities::read<uint64_t>(AActors + i * sizeof(uintptr_t));
		uintptr_t PlayerState = SDK::Utilities::read<uintptr_t>(CurrentActor + SDK::Classes::StaticOffsets::PlayerState);

		auto pawn = reinterpret_cast<SDK::Structs::UObject*>(SDK::Utilities::read<uint64_t>(AActors, i * sizeof(PVOID)));
		if (!pawn || pawn == (PVOID)LocalPawn) continue;

		std::string NameCurrentActor = GetObjectName(CurrentActor);
		std::string NameItemActor = GetObjectName((uintptr_t)pawn);

		if (settings.loot_esp && strstr(NameCurrentActor.c_str(), xorthis("Tiered_Ammo")) && !((ReadBYTE(pawn, SDK::Classes::StaticOffsets::bAlreadySearched) >> 0xC61)))
		{
			auto AmmoBoxRoot = GetPawnRootLocations(LocalPawn);
			uintptr_t VehicleRootComponent = SDK::Utilities::read<uintptr_t>(CurrentActor + 0x130);
			auto itemPlayerLocation = reinterpret_cast<float*>(reinterpret_cast<PBYTE>(VehicleRootComponent) + SDK::Classes::StaticOffsets::RelativeLocation);
			if (AmmoBoxRoot) {
				auto AmmoBoxPos = *AmmoBoxRoot;
				float dx = itemPlayerLocation[0] - AmmoBoxPos.X;
				float dy = itemPlayerLocation[1] - AmmoBoxPos.Y;
				float dz = itemPlayerLocation[2] - AmmoBoxPos.Z;

				ImFont* font_current = ImGui::GetFont();
				float dist = SDK::Utilities::SpoofCall(sqrtf, dx * dx + dy * dy + dz * dz) / 100.0f;

				fn::general_overlay::DrawNormalText(font_current, xorthis("Ammo Box [%.0f m]"), ImVec2(AmmoBoxPos.X, AmmoBoxPos.Y), 14.5f, ImColor({ 0.75f, 0.75f, 0.75f, 1.0f }), true);
			}
		}

		else if (settings.loot_esp && strstr(NameCurrentActor.c_str(), xorthis("Tiered_Chest")) && !((ReadBYTE(pawn, SDK::Classes::StaticOffsets::bAlreadySearched) >> 0xC61)))
		{
			auto ChestRoot = GetPawnRootLocations(LocalPawn);
			uintptr_t VehicleRootComponent = SDK::Utilities::read<uintptr_t>(CurrentActor + 0x130);
			auto itemPlayerLocation = reinterpret_cast<float*>(reinterpret_cast<PBYTE>(VehicleRootComponent) + SDK::Classes::StaticOffsets::RelativeLocation);
			if (ChestRoot) {
				auto ChestPos = *ChestRoot;
				char draw[64];
				float dx = itemPlayerLocation[0] - ChestPos.X;
				float dy = itemPlayerLocation[1] - ChestPos.Y;
				float dz = itemPlayerLocation[2] - ChestPos.Z;

				ImFont* font_current = ImGui::GetFont();

				float dist = SDK::Utilities::SpoofCall(sqrtf, dx * dx + dy * dy + dz * dz) / 100.0f;

				fn::general_overlay::DrawNormalText(font_current, xorthis("Chest [%.0f m]"), ImVec2(ChestPos.X, ChestPos.Y), 14.5f, ImColor({ 255,128,0,255 }), true);
			}
		}

		if (strstr(NameItemActor.c_str(), xorthis("AthenaSupplyDrop_Llama")))
		{
			if (settings.loot_esp)
			{
				float dist;
				uintptr_t VehicleRootComponent = SDK::Utilities::read<uintptr_t>(CurrentActor + 0x130);
				SDK::Structs::Vector3 VehicleRoot = SDK::Utilities::read<SDK::Structs::Vector3>(VehicleRootComponent + 0x11C);
				auto itemPlayerLocation = reinterpret_cast<float*>(reinterpret_cast<PBYTE>(VehicleRootComponent) + SDK::Classes::StaticOffsets::RelativeLocation);
				SDK::Structs::Vector3 VehicleRootW2S;
				SDK::Classes::AController::WorldToScreen(VehicleRoot, &VehicleRootW2S);
				auto itemRoot = GetPawnRootLocations(CurrentActor);
				if (itemRoot) {
					auto itemPos = *itemRoot;
					float x = itemPlayerLocation[0] - itemPos.X;
					float y = itemPlayerLocation[1] - itemPos.Y;
					float z = itemPlayerLocation[2] - itemPos.Z;

					dist = sqrtf((x * x) + (y * y) + (z * z)) / 100.0f;

					DrawOutlinedText("Llama", ImVec2(VehicleRootW2S.x, VehicleRootW2S.y), ImColor(ITEM_COLOR_SUPPLYDROP), false);
				}
			}
		}

		if (settings.vehicle and strstr(NameCurrentActor.c_str(), xorthis("Valet_Taxi"))) {
			if (SDK::Utilities::valid_pointer(LocalPawn))
			{
				uintptr_t VehicleRootComponent = SDK::Utilities::read<uintptr_t>(CurrentActor + 0x130);
				SDK::Structs::Vector3 VehicleRoot = SDK::Utilities::read<SDK::Structs::Vector3>(VehicleRootComponent + 0x11C);
				SDK::Structs::Vector3 VehicleRootW2S;
				SDK::Classes::AController::WorldToScreen(VehicleRoot, &VehicleRootW2S);

				DrawOutlinedText("Taxi", ImVec2(VehicleRootW2S.x, VehicleRootW2S.y), ImColor(3, 252, 227), false);
			}
		}
		else if (settings.vehicle and strstr(NameCurrentActor.c_str(), xorthis("Valet_BasicTr"))) {
			if (SDK::Utilities::valid_pointer(LocalPawn))
			{
				uintptr_t VehicleRootComponent = SDK::Utilities::read<uintptr_t>(CurrentActor + 0x130);
				SDK::Structs::Vector3 VehicleRoot = SDK::Utilities::read<SDK::Structs::Vector3>(VehicleRootComponent + 0x11C);
				SDK::Structs::Vector3  VehicleRootW2S;
				SDK::Classes::AController::WorldToScreen(VehicleRoot, &VehicleRootW2S);

				DrawOutlinedText("Truck", ImVec2(VehicleRootW2S.x, VehicleRootW2S.y), ImColor(3, 252, 227), false);
			}
		}
		else if (settings.vehicle and strstr(NameCurrentActor.c_str(), xorthis("Valet_BigRig"))) {
			if (SDK::Utilities::valid_pointer(LocalPawn)) {
				uintptr_t VehicleRootComponent = SDK::Utilities::read<uintptr_t>(CurrentActor + 0x130);
				SDK::Structs::Vector3 VehicleRoot = SDK::Utilities::read<SDK::Structs::Vector3>(VehicleRootComponent + 0x11C);
				SDK::Structs::Vector3  VehicleRootW2S;
				SDK::Classes::AController::WorldToScreen(VehicleRoot, &VehicleRootW2S);

				DrawOutlinedText("BigRig", ImVec2(VehicleRootW2S.x, VehicleRootW2S.y), ImColor(3, 252, 227), false);
			}
		}
		else if (settings.vehicle and strstr(NameCurrentActor.c_str(), xorthis("Valet_BasicC"))) {
			if (SDK::Utilities::valid_pointer(LocalPawn)) {
				uintptr_t VehicleRootComponent = SDK::Utilities::read<uintptr_t>(CurrentActor + 0x130);
				SDK::Structs::Vector3 VehicleRoot = SDK::Utilities::read<SDK::Structs::Vector3>(VehicleRootComponent + 0x11C);
				SDK::Structs::Vector3  VehicleRootW2S;
				SDK::Classes::AController::WorldToScreen(VehicleRoot, &VehicleRootW2S);

				DrawOutlinedText("Car", ImVec2(VehicleRootW2S.x, VehicleRootW2S.y), ImColor(3, 252, 227), false);
			}
		}
		else if (settings.vehicle and strstr(NameCurrentActor.c_str(), xorthis("Valet_SportsCarC"))) {
			if (SDK::Utilities::valid_pointer(LocalPawn)) {
				uintptr_t VehicleRootComponent = SDK::Utilities::read<uintptr_t>(CurrentActor + 0x130);
				SDK::Structs::Vector3 VehicleRoot = SDK::Utilities::read<SDK::Structs::Vector3>(VehicleRootComponent + 0x11C);
				SDK::Structs::Vector3  VehicleRootW2S;
				SDK::Classes::AController::WorldToScreen(VehicleRoot, &VehicleRootW2S);

				DrawOutlinedText("SportsCar", ImVec2(VehicleRootW2S.x, VehicleRootW2S.y), ImColor(3, 252, 227), false);
			}
		}
		else if (settings.vehicle and strstr(NameCurrentActor.c_str(), xorthis("MeatballVehicle_L"))) {
			if (SDK::Utilities::valid_pointer(LocalPawn)) {
				uintptr_t VehicleRootComponent = SDK::Utilities::read<uintptr_t>(CurrentActor + 0x130);
				SDK::Structs::Vector3 VehicleRoot = SDK::Utilities::read<SDK::Structs::Vector3>(VehicleRootComponent + 0x11C);
				SDK::Structs::Vector3  VehicleRootW2S;
				SDK::Classes::AController::WorldToScreen(VehicleRoot, &VehicleRootW2S);

				DrawOutlinedText("Boat", ImVec2(VehicleRootW2S.x, VehicleRootW2S.y), ImColor(3, 252, 227), false);
			}
		}

		if (strstr(NameCurrentActor.c_str(), xorthis("PlayerPawn_")) || strstr(NameCurrentActor.c_str(), xorthis("PlayerPawn_Athena_Phoebe_C")))
		{

			/*if (strstr(NameCurrentActor.c_str(), xorthis("PlayerController")) && strstr(NameCurrentActor.c_str(), xorthis("Kick"))) {
				return 0;
			}*/

			bool IsVisibleEnemy;
			SDK::Structs::Vector3 CameraView;

			IsVisibleEnemy = SDK::Classes::APlayerCameraManager::LineOfSightTo((PVOID)PlayerController, (PVOID)CurrentActor, &CameraView);
			if (IsVisibleEnemy) { std::string E = xorthis(""); DrawOutlinedText(E.c_str(), ImVec2(880, 120), ImColor(218, 42, 28), false); }

			auto col = ImColor(218, 42, 28);

			ImColor SkelColor = ImColor(3, 252, 227);

			ImColor distancecolor = { 255, 255, 255, 220 };
			ImColor namecolor = { 255, 255, 255, 220 };
			ImColor weaponColor = { 255, 255, 255, 220 };
			ImColor ammoColor = { 255, 255, 255, 220 };

			SDK::Structs::Vector3 headpos[3] = { 0 };
			SDK::Classes::USkeletalMeshComponent::GetBoneLocation(CurrentActor, 66, &headpos[3]);
			SDK::Classes::AController::WorldToScreen(SDK::Structs::Vector3(0), &headpos[3]);

			SDK::Structs::Vector3 Headbox, bottom, pelviss;

			SDK::Classes::USkeletalMeshComponent::GetBoneLocation(CurrentActor, 66, &Headbox);
			SDK::Classes::USkeletalMeshComponent::GetBoneLocation(CurrentActor, 2, &pelviss);
			SDK::Classes::USkeletalMeshComponent::GetBoneLocation(CurrentActor, 0, &bottom);

			SDK::Classes::AController::WorldToScreen(SDK::Structs::Vector3(Headbox.x, Headbox.y, Headbox.z + 20), &Headbox);
			SDK::Classes::AController::WorldToScreen(SDK::Structs::Vector3(pelviss.x, pelviss.y, pelviss.z + 70), &pelviss);
			SDK::Classes::AController::WorldToScreen(bottom, &bottom);

			if (Headbox.x == 0 && Headbox.y == 0) continue;
			if (bottom.x == 0 && bottom.y == 0) continue;

			float Height = Headbox.y - bottom.y;
			if (Height < 0)
				Height = Height * (-1.f);
			float Width = Height * 0.25;
			Headbox.x = Headbox.x - (Width / 2);

			float Height1 = Headbox.y - bottom.y;

			if (Height1 < 0)
				Height1 = Height1 * (-1.f);
			float Width1 = Height1 * 0.25;

			ImColor PlayerPointColor = { 255,128,0,255 }; //Radar Player Color

			SDK::Structs::Vector3 head2, neck, pelvis, chest, leftShoulder, rightShoulder, leftElbow, rightElbow, leftHand, rightHand, leftLeg, rightLeg, leftThigh, rightThigh, leftFoot, rightFoot, leftFeet, rightFeet, leftFeetFinger, rightFeetFinger;

			if (valid_pointer(LocalPawn))
			{
				uintptr_t MyState = SDK::Utilities::read<uintptr_t>(LocalPawn + SDK::Classes::StaticOffsets::PlayerState);
				if (!MyState) continue;

				LocalWeapon = SDK::Utilities::read<uint64_t>(LocalPawn + SDK::Classes::StaticOffsets::CurrentWeapon);
				if (!LocalWeapon) continue;

				MyTeamIndex = SDK::Utilities::read<uintptr_t>(MyState + SDK::Classes::StaticOffsets::TeamIndex);
				if (!MyTeamIndex) continue;

				uintptr_t EnemyState = SDK::Utilities::read<uintptr_t>(CurrentActor + SDK::Classes::StaticOffsets::PlayerState);
				if (!EnemyState) continue;

				EnemyTeamIndex = SDK::Utilities::read<uintptr_t>(EnemyState + SDK::Classes::StaticOffsets::TeamIndex);
				if (!EnemyTeamIndex) continue;

				if (CurrentActor == LocalPawn) continue;

				SDK::Structs::Vector3 viewPoint;
				bool IsVisible;

				bool wasClicked = false;

				
				if (settings.nospread)
				{
					if (GetAsyncKeyState(VK_RBUTTON))
					{
						(*(FLOAT*)(((PBYTE)LocalWeapon + SDK::Classes::StaticOffsets::PlayerController))) = 0.0f;
					}
				}
				

				if (settings.speedhack)
				{
					if (GetAsyncKeyState(VK_SHIFT))
					{
						setAllToSpeed(3.0);
					}
					else
					{
						setAllToSpeed(1.0);
					}
				}
				else
				{
					setAllToSpeed(1.0);
				}

				if (settings.name && SDK::Classes::Utils::CheckInScreen(CurrentActor, X, Y))
				{
					SDK::Structs::Vector3 headA;
					SDK::Classes::USkeletalMeshComponent::GetBoneLocation(CurrentActor, 66, &headA);
					SDK::Classes::AController::WorldToScreen(headA, &headA);
					SDK::Structs::FString gayName = SDK::Utilities::read<SDK::Structs::FString>(EnemyState + 0x300);

					if (gayName.c_str())
					{
						CHAR Eichhörnchen[0xFF] = { 0 };
						wcstombs(Eichhörnchen, gayName.c_str(), sizeof(gayName));
						fn::general_overlay::OutlinedRBGText(headA.x, headA.y + 25, ImColor(161, 5, 5), TxtFormat(xorthis("[ %s ]"), Eichhörnchen));
					}
				}

				if (settings.vischeck) {
					IsVisible = SDK::Classes::APlayerCameraManager::LineOfSightTo((PVOID)PlayerController, (PVOID)CurrentActor, &viewPoint);
					if (IsVisible) {
						col = { 255,0,0,255 }; //Visible
					}
					else {
						col = { 255,128,0,255 }; //Not visible
					}
					if (IsVisible)
					{
						SkelColor = { 3, 252, 227 };
					}
					else
					{
						SkelColor = { 3, 252, 227 };
					}
					if (IsVisible) {
						distancecolor = { 255,0,0,255 };
					}
					else {
						distancecolor = { 255,128,0,255 };
					}
					if (IsVisible) {
						namecolor = { 255,0,0,255 };
					}
					else {
						namecolor = { 255,128,0,255 };
					}
					if (IsVisible) {
						PlayerPointColor = { 255,0,0,255 };
					}
					else {
						PlayerPointColor = { 255,128,0,255 };
					}
				}
			}
				
			if (SDK::Utilities::DiscordHelper::IsAiming())
			{
				if (SDK::Utilities::CheckIfInFOV(CurrentActor, FOVmax)) {

					if (settings.vischeck and IsVisibleEnemy)
					{
						if (settings.memory)
						{
							SDK::Structs::Vector3 NewAngle = SDK::Utilities::GetRotation(CurrentActor);

							if (NewAngle.x == 0 && NewAngle.y == 0) continue;

							if (settings.smoothness > 0)
								NewAngle = SDK::Utilities::SmoothAngles(SDK::Utilities::CamRot, NewAngle);

							NewAngle.z = 0;

							SDK::Classes::AController::ValidateClientSetRotation(NewAngle, false);
							SDK::Classes::AController::ClientSetRotation(NewAngle, false);
						}
					}
					else if (!settings.vischeck)
					{
						SDK::Structs::Vector3 NewAngle = SDK::Utilities::GetRotation(CurrentActor);

						if (NewAngle.x == 0 && NewAngle.y == 0) continue;

						if (settings.smoothness > 0)
							NewAngle = SDK::Utilities::SmoothAngles(SDK::Utilities::CamRot, NewAngle);

						NewAngle.z = 0;

						SDK::Classes::AController::ValidateClientSetRotation(NewAngle, false);
						SDK::Classes::AController::ClientSetRotation(NewAngle, false);

					}
				}
			}

			if (settings.radar)
			{
				RadarLoop(CurrentActor, PlayerPointColor);
			}

			if (settings.corner && SDK::Classes::Utils::CheckInScreen(CurrentActor, X, Y))
			{
				SDK::Classes::USkeletalMeshComponent::GetBoneLocation(CurrentActor, 66, &head2);
				SDK::Classes::USkeletalMeshComponent::GetBoneLocation(CurrentActor, 65, &neck);
				SDK::Classes::USkeletalMeshComponent::GetBoneLocation(CurrentActor, 2, &pelvis);
				SDK::Classes::USkeletalMeshComponent::GetBoneLocation(CurrentActor, 36, &chest);
				SDK::Classes::USkeletalMeshComponent::GetBoneLocation(CurrentActor, 9, &leftShoulder);
				SDK::Classes::USkeletalMeshComponent::GetBoneLocation(CurrentActor, 37, &rightShoulder);
				SDK::Classes::USkeletalMeshComponent::GetBoneLocation(CurrentActor, 10, &leftElbow);
				SDK::Classes::USkeletalMeshComponent::GetBoneLocation(CurrentActor, 38, &rightElbow);
				SDK::Classes::USkeletalMeshComponent::GetBoneLocation(CurrentActor, 11, &leftHand);
				SDK::Classes::USkeletalMeshComponent::GetBoneLocation(CurrentActor, 39, &rightHand);
				SDK::Classes::USkeletalMeshComponent::GetBoneLocation(CurrentActor, 67, &leftLeg);
				SDK::Classes::USkeletalMeshComponent::GetBoneLocation(CurrentActor, 74, &rightLeg);
				SDK::Classes::USkeletalMeshComponent::GetBoneLocation(CurrentActor, 73, &leftThigh);
				SDK::Classes::USkeletalMeshComponent::GetBoneLocation(CurrentActor, 80, &rightThigh);
				SDK::Classes::USkeletalMeshComponent::GetBoneLocation(CurrentActor, 68, &leftFoot);
				SDK::Classes::USkeletalMeshComponent::GetBoneLocation(CurrentActor, 75, &rightFoot);
				SDK::Classes::USkeletalMeshComponent::GetBoneLocation(CurrentActor, 71, &leftFeet);
				SDK::Classes::USkeletalMeshComponent::GetBoneLocation(CurrentActor, 78, &rightFeet);
				SDK::Classes::USkeletalMeshComponent::GetBoneLocation(CurrentActor, 72, &leftFeetFinger);
				SDK::Classes::USkeletalMeshComponent::GetBoneLocation(CurrentActor, 79, &rightFeetFinger);
				SDK::Classes::AController::WorldToScreen(head2, &head2);
				SDK::Classes::AController::WorldToScreen(neck, &neck);
				SDK::Classes::AController::WorldToScreen(pelvis, &pelvis);
				SDK::Classes::AController::WorldToScreen(chest, &chest);
				SDK::Classes::AController::WorldToScreen(leftShoulder, &leftShoulder);
				SDK::Classes::AController::WorldToScreen(rightShoulder, &rightShoulder);
				SDK::Classes::AController::WorldToScreen(leftElbow, &leftElbow);
				SDK::Classes::AController::WorldToScreen(rightElbow, &rightElbow);
				SDK::Classes::AController::WorldToScreen(leftHand, &leftHand);
				SDK::Classes::AController::WorldToScreen(rightHand, &rightHand);
				SDK::Classes::AController::WorldToScreen(leftLeg, &leftLeg);
				SDK::Classes::AController::WorldToScreen(rightLeg, &rightLeg);
				SDK::Classes::AController::WorldToScreen(leftThigh, &leftThigh);
				SDK::Classes::AController::WorldToScreen(rightThigh, &rightThigh);
				SDK::Classes::AController::WorldToScreen(leftFoot, &leftFoot);
				SDK::Classes::AController::WorldToScreen(rightFoot, &rightFoot);
				SDK::Classes::AController::WorldToScreen(leftFeet, &leftFeet);
				SDK::Classes::AController::WorldToScreen(rightFeet, &rightFeet);
				SDK::Classes::AController::WorldToScreen(leftFeetFinger, &leftFeetFinger);
				SDK::Classes::AController::WorldToScreen(rightFeetFinger, &rightFeetFinger);

				int array[20] = { head2.x, neck.x, pelvis.x, chest.x, leftShoulder.x, rightShoulder.x, leftElbow.x, rightElbow.x, leftHand.x, rightHand.x, leftLeg.x, rightLeg.x, leftThigh.x, rightThigh.x, leftFoot.x, rightFoot.x, leftFeet.x, rightFeet.x, leftFeetFinger.x, rightFeetFinger.x };
				int mostright = array[0];
				int mostleft = array[0];

				for (int mostrighti = 0; mostrighti < 20; mostrighti++)
				{
					if (array[mostrighti] > mostright)
						mostright = array[mostrighti];
				}

				for (int mostlefti = 0; mostlefti < 20; mostlefti++)
				{
					if (array[mostlefti] < mostleft)
						mostleft = array[mostlefti];
				}

				fn::general_overlay::DrawCorneredBox(pelviss.x - ((mostright - mostleft) / 2), pelviss.y, (mostright - mostleft), Height1, ImColor(col), 2.3f);
				//fn::general_overlay::Rect(pelviss.x - ((mostright - mostleft) / 2), pelviss.y, (mostright - mostleft), Height1, ImColor(col), 2.3f);

				if (SDK::Utilities::valid_pointer(LocalPawn))
				{
					float dist;
					Headbox + 15;
					auto playerRoot = GetPawnRootLocations(LocalPawn);
					uintptr_t VehicleRootComponent = SDK::Utilities::read<uintptr_t>(CurrentActor + 0x130);
					auto localPlayerLocation = reinterpret_cast<float*>(reinterpret_cast<PBYTE>(VehicleRootComponent) + SDK::Classes::StaticOffsets::RelativeLocation);
					if (playerRoot) {
						auto playerPos = *playerRoot;
						float x = localPlayerLocation[0] - playerPos.X;
						float y = localPlayerLocation[1] - playerPos.Y;
						float z = localPlayerLocation[2] - playerPos.Z;
						ImVec2 BottomPos = ImVec2(bottom.x, bottom.y);
						char draw[64];
						dist = sqrtf((x * x) + (y * y) + (z * z)) / 100.0f;
						sprintf_s(draw, "[%.0fm]", dist);
						ImFont* font_current = ImGui::GetFont();
						fn::general_overlay::DrawNormalText(font_current, draw, BottomPos, 12.0f, ImColor(distancecolor), true);
					}
				}
				if (strstr(NameCurrentActor.c_str(), xorthis("PlayerPawn_Athena_Phoebe_C")))
				{
					SDK::Structs::Vector3 headpos = { 35 };
					SDK::Classes::USkeletalMeshComponent::GetBoneLocation(CurrentActor, 5, &headpos);
					SDK::Classes::AController::WorldToScreen(SDK::Structs::Vector3(headpos.x, headpos.y, headpos.z), &headpos);
					DrawOutlinedText("Bot", ImVec2(headpos.x, headpos.y), ImColor(namecolor), false);
				}
				/*
				if (SDK::Classes::Utils::CheckInScreen(CurrentActor, X, Y))
				{
					SDK::Structs::Vector3 headA;
					SDK::Classes::USkeletalMeshComponent::GetBoneLocation(CurrentActor, 66, &headA);
					SDK::Classes::AController::WorldToScreen(headA, &headA);
					SDK::Structs::FString gayName = SDK::Utilities::read<SDK::Structs::FString>(EnemyTeamIndex + 0x300);

					if (gayName.c_str())
					{
						CHAR Eichhörnchen[0xFF] = { 0 };
						wcstombs(Eichhörnchen, gayName.c_str(), sizeof(gayName));
						fn::general_overlay::OutlinedRBGText(headA.x, headA.y + 25, ImColor(161, 5, 5), TxtFormat(xorthis("%s"), Eichhörnchen));
					}
				}
				*/
			}

			//Exploits


			uintptr_t bDisableEquipAnimation = 0x2B3;
			uintptr_t ReviveFromDBNOTime = 0x33B8;

			if (settings.AimWhileJumping) 
			{
				(SDK::Classes::StaticOffsets::CurrentWeapon + bDisableEquipAnimation, true);
			}

			if (settings.InstantRevive and GetAsyncKeyState(0x45))
			{
				(LocalPawn + ReviveFromDBNOTime, 0.101);
			}

			if (settings.NoBloom)
			{
				(SDK::Classes::StaticOffsets::CurrentWeapon + 0x0D90, 0.01f);
			}

			if (settings.silentAim)
			{
				//if (!L::MainGay)
				{
					
				}
			}

			if (settings.debug_info)
			{
				for (int youareretardedmonkey = 0; youareretardedmonkey < 87; youareretardedmonkey++) 
				{
					SDK::Structs::Vector3 out;
					SDK::Structs::Vector3 outw2s;

					SDK::Classes::USkeletalMeshComponent::GetBoneLocation(CurrentActor, youareretardedmonkey, &out);
					SDK::Classes::AController::WorldToScreen(out, &outw2s);

					fn::general_overlay::OutlinedString(std::to_string(youareretardedmonkey), ImVec2(outw2s.x, outw2s.y), ImColor(3, 252, 227));
				}
			}

			if (settings.skeleton)
			{
				SDK::Structs::Vector3 head2, neck, pelvis, chest, leftShoulder, rightShoulder, leftElbow, rightElbow, leftHand, rightHand, leftLeg, rightLeg, leftThigh, rightThigh, leftFoot, rightFoot, leftFeet, rightFeet, leftFeetFinger, rightFeetFinger;

				SDK::Classes::USkeletalMeshComponent::GetBoneLocation(CurrentActor, 66, &head2);
				SDK::Classes::USkeletalMeshComponent::GetBoneLocation(CurrentActor, 65, &neck);
				SDK::Classes::USkeletalMeshComponent::GetBoneLocation(CurrentActor, 2, &pelvis);
				SDK::Classes::USkeletalMeshComponent::GetBoneLocation(CurrentActor, 36, &chest);
				SDK::Classes::USkeletalMeshComponent::GetBoneLocation(CurrentActor, 9, &leftShoulder);
				SDK::Classes::USkeletalMeshComponent::GetBoneLocation(CurrentActor, 62, &rightShoulder);
				SDK::Classes::USkeletalMeshComponent::GetBoneLocation(CurrentActor, 10, &leftElbow);
				SDK::Classes::USkeletalMeshComponent::GetBoneLocation(CurrentActor, 38, &rightElbow);
				SDK::Classes::USkeletalMeshComponent::GetBoneLocation(CurrentActor, 11, &leftHand);
				SDK::Classes::USkeletalMeshComponent::GetBoneLocation(CurrentActor, 39, &rightHand);
				SDK::Classes::USkeletalMeshComponent::GetBoneLocation(CurrentActor, 67, &leftLeg);
				SDK::Classes::USkeletalMeshComponent::GetBoneLocation(CurrentActor, 74, &rightLeg);
				SDK::Classes::USkeletalMeshComponent::GetBoneLocation(CurrentActor, 73, &leftThigh);
				SDK::Classes::USkeletalMeshComponent::GetBoneLocation(CurrentActor, 80, &rightThigh);
				SDK::Classes::USkeletalMeshComponent::GetBoneLocation(CurrentActor, 68, &leftFoot);
				SDK::Classes::USkeletalMeshComponent::GetBoneLocation(CurrentActor, 75, &rightFoot);
				SDK::Classes::USkeletalMeshComponent::GetBoneLocation(CurrentActor, 71, &leftFeet);
				SDK::Classes::USkeletalMeshComponent::GetBoneLocation(CurrentActor, 78, &rightFeet);
				SDK::Classes::USkeletalMeshComponent::GetBoneLocation(CurrentActor, 72, &leftFeetFinger);
				SDK::Classes::USkeletalMeshComponent::GetBoneLocation(CurrentActor, 79, &rightFeetFinger);
				SDK::Classes::AController::WorldToScreen(head2, &head2);
				SDK::Classes::AController::WorldToScreen(neck, &neck);
				SDK::Classes::AController::WorldToScreen(pelvis, &pelvis);
				SDK::Classes::AController::WorldToScreen(chest, &chest);
				SDK::Classes::AController::WorldToScreen(leftShoulder, &leftShoulder);
				SDK::Classes::AController::WorldToScreen(rightShoulder, &rightShoulder);
				SDK::Classes::AController::WorldToScreen(leftElbow, &leftElbow);
				SDK::Classes::AController::WorldToScreen(rightElbow, &rightElbow);
				SDK::Classes::AController::WorldToScreen(leftHand, &leftHand);
				SDK::Classes::AController::WorldToScreen(rightHand, &rightHand);
				SDK::Classes::AController::WorldToScreen(leftLeg, &leftLeg);
				SDK::Classes::AController::WorldToScreen(rightLeg, &rightLeg);
				SDK::Classes::AController::WorldToScreen(leftThigh, &leftThigh);
				SDK::Classes::AController::WorldToScreen(rightThigh, &rightThigh);
				SDK::Classes::AController::WorldToScreen(leftFoot, &leftFoot);
				SDK::Classes::AController::WorldToScreen(rightFoot, &rightFoot);
				SDK::Classes::AController::WorldToScreen(leftFeet, &leftFeet);
				SDK::Classes::AController::WorldToScreen(rightFeet, &rightFeet);
				SDK::Classes::AController::WorldToScreen(leftFeetFinger, &leftFeetFinger);
				SDK::Classes::AController::WorldToScreen(rightFeetFinger, &rightFeetFinger);
				fn::general_overlay::DrawLine(head2.x, head2.y, neck.x, neck.y, SkelColor, 1.5f);
				fn::general_overlay::DrawLine(neck.x, neck.y, pelvis.x, pelvis.y, SkelColor, 1.5f);
				fn::general_overlay::DrawLine(chest.x, chest.y, leftShoulder.x, leftShoulder.y, SkelColor, 1.5f);
				fn::general_overlay::DrawLine(chest.x, chest.y, rightShoulder.x, rightShoulder.y, SkelColor, 1.5f);
				fn::general_overlay::DrawLine(leftShoulder.x, leftShoulder.y, leftElbow.x, leftElbow.y, SkelColor, 1.5f);
				fn::general_overlay::DrawLine(rightShoulder.x, rightShoulder.y, rightElbow.x, rightElbow.y, SkelColor, 1.5f);
				fn::general_overlay::DrawLine(leftElbow.x, leftElbow.y, leftHand.x, leftHand.y, SkelColor, 1.5f);
				fn::general_overlay::DrawLine(rightElbow.x, rightElbow.y, rightHand.x, rightHand.y, SkelColor, 1.5f);
				fn::general_overlay::DrawLine(pelvis.x, pelvis.y, leftLeg.x, leftLeg.y, SkelColor, 1.5f);
				fn::general_overlay::DrawLine(pelvis.x, pelvis.y, rightLeg.x, rightLeg.y, SkelColor, 1.5f);
				fn::general_overlay::DrawLine(leftLeg.x, leftLeg.y, leftThigh.x, leftThigh.y, SkelColor, 1.5f);
				fn::general_overlay::DrawLine(rightLeg.x, rightLeg.y, rightThigh.x, rightThigh.y, SkelColor, 1.5f);
				fn::general_overlay::DrawLine(leftThigh.x, leftThigh.y, leftFoot.x, leftFoot.y, SkelColor, 1.5f);
				fn::general_overlay::DrawLine(rightThigh.x, rightThigh.y, rightFoot.x, rightFoot.y, SkelColor, 1.5f);
				fn::general_overlay::DrawLine(leftFoot.x, leftFoot.y, leftFeet.x, leftFeet.y, SkelColor, 1.5f);
				fn::general_overlay::DrawLine(rightFoot.x, rightFoot.y, rightFeet.x, rightFeet.y, SkelColor, 1.5f);
				fn::general_overlay::DrawLine(leftFeet.x, leftFeet.y, leftFeetFinger.x, leftFeetFinger.y, SkelColor, 1.5f);
				fn::general_overlay::DrawLine(rightFeet.x, rightFeet.y, rightFeetFinger.x, rightFeetFinger.y, SkelColor, 1.5f);

			}
		}
	}

	return true;
}

std::string random_string(std::size_t length)
{
	const std::string CHARACTERS = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

	std::random_device random_device;
	std::mt19937 generator(random_device());
	std::uniform_int_distribution<> distribution(0, CHARACTERS.size() - 1);

	std::string random_string;

	for (std::size_t i = 0; i < length; ++i)
	{
		random_string += CHARACTERS[distribution(generator)];
	}

	return random_string;
}

void fn::render(ImGuiWindow& window)
{
	
}

void fn::menu()
{
	fn::keys();

	float ScreenCenterX = X1;
	float ScreenCenterY = Y1;

	if (settings.crosshair)
	{
		ImGui::GetOverlayDrawList()->AddLine(ImVec2(ScreenCenterX - 8.f, ScreenCenterY), ImVec2((ScreenCenterX - 8.f) + (8.f * 2), ScreenCenterY), ImColor(96, 96, 96, 255));
		ImGui::GetOverlayDrawList()->AddLine(ImVec2(ScreenCenterX, ScreenCenterY - 8.f), ImVec2(ScreenCenterX, (ScreenCenterY - 8.f) + (8.f * 2)), ImColor(96, 96, 96, 255));
	}

	if (settings.aimbot_fov)
	{
		ImGui::GetOverlayDrawList()->AddCircle(ImVec2(X1, Y1), settings.radius, ImColor(96, 96, 96, 255), 100);
		ImGui::GetOverlayDrawList()->AddCircle(ImVec2(X1, Y1), settings.radius + 0.5f, ImColor(96, 96, 96, 255), 100);
	}

	if (settings.menu)
	{
		ImGuiStyle& style = ImGui::GetStyle();
		style.Colors[ImGuiCol_Text] = ImColor(255, 165, 0);

		style.Colors[ImGuiCol_WindowBg] = ImColor(0.00f, 0.22f, 0.56f, 1.00f);
		style.Colors[ImGuiCol_PopupBg] = ImColor(0.16f, 0.16f, 0.16f, 1.00f);

		style.Colors[ImGuiCol_Border] = ImColor(0.f, 0.f, 0.f, 0.00f);
		style.Colors[ImGuiCol_BorderShadow] = ImColor(0.00f, 0.00f, 0.00f, 1.00f);

		style.Colors[ImGuiCol_FrameBg] = ImColor(0.23f, 0.23f, 0.23f, 1.00f);
		style.Colors[ImGuiCol_FrameBgHovered] = ImColor(0.40f, 0.40f, 0.40f, 1.00f);
		style.Colors[ImGuiCol_FrameBgActive] = ImColor(0.52f, 0.52f, 0.52f, 1.00f);

		style.Colors[ImGuiCol_TitleBg] = ImColor(0.22f, 0.21f, 0.21f, 0.00f);
		style.Colors[ImGuiCol_TitleBgActive] = ImColor(0.0f, 0.22f, 0.55f, 1.00f);
		style.Colors[ImGuiCol_TitleBgCollapsed] = ImColor(0.22f, 0.21f, 0.21f, 0.00f);

		style.Colors[ImGuiCol_MenuBarBg] = ImColor(0.14f, 0.14f, 0.14f, 1.00f);

		style.Colors[ImGuiCol_SliderGrab] = ImColor(4, 34, 86);
		style.Colors[ImGuiCol_SliderGrabActive] = ImColor(4, 34, 86);

		style.Colors[ImGuiCol_Button] = ImColor(4, 34, 86);
		style.Colors[ImGuiCol_ButtonHovered] = ImColor(4, 34, 86);
		style.Colors[ImGuiCol_ButtonActive] = ImColor(4, 34, 86);

		style.Colors[ImGuiCol_Header] = ImColor(0.26f, 0.59f, 0.98f, 1.00f);
		style.Colors[ImGuiCol_HeaderHovered] = ImColor(0.26f, 0.59f, 0.98f, 1.00f);
		style.Colors[ImGuiCol_HeaderActive] = ImColor(0.26f, 0.59f, 0.98f, 1.00f);

		style.Colors[ImGuiCol_Separator] = ImColor(0.43f, 0.43f, 0.50f, 1.00f);
		style.Colors[ImGuiCol_SeparatorHovered] = ImColor(0.10f, 0.40f, 0.75f, 1.00f);
		style.Colors[ImGuiCol_SeparatorActive] = ImColor(0.10f, 0.40f, 0.75f, 1.00f);

		style.WindowPadding = ImVec2(8.00f, 8.00f);
		style.FramePadding = ImVec2(4.00f, 3.00f);
		style.ItemSpacing = ImVec2(6.00f, 10.00f);
		style.ItemInnerSpacing = ImVec2(4.00f, 4.00f);
		style.TouchExtraPadding = ImVec2(0.00f, 0.00f);
		style.IndentSpacing = 21.00f;
		style.ScrollbarSize = 14.00f;
		style.GrabMinSize = 10.00f;
		style.PopupBorderSize = 1.00f;

		style.WindowRounding = 10;
		style.ChildRounding = 10;
		style.FrameRounding = 10;

		style.DisplaySafeAreaPadding = ImVec2(3.00f, 3.00f);

		static int tabs = 1;

		ImGui::SetNextWindowSize({ 461.f,302.f });

		{
			ImGui::SetNextWindowSize({ 525, 633 });
			ImGui::Begin("cheatloverz.store", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollWithMouse);
			ImGui::SetCursorPos({ 10, 28 });
			if (ImGui::Button("Visuals", { 120, 30 }))
			{
				tabs = 1;
			}
			ImGui::SameLine(0, 8);
			if (ImGui::Button("Aimbot", { 120, 30 }))
			{
				tabs = 2;
			}
			ImGui::SameLine(0, 8);
			if (ImGui::Button("Exploits", { 120, 30 }))
			{
				tabs = 3;
			}
			ImGui::SameLine(0, 8);
			if (ImGui::Button("Misc", { 120, 30 }))
			{
				tabs = 4;
			}
			ImGui::SetCursorPos({ 13, 612 });
			ImGui::Text("Tips: Use middle mouse for sliders and F8 to toggle Menu");
			if (tabs == 1)
			{
				ImGui::SetCursorPos({ 10, 80 });
				ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0f, 0.16f, 0.42f, 1.0f));
				ImGui::BeginChild("child", { 502.f,530.f }, true);
				PS::CL_ToggleButton("Characters ESP", &settings.corner, 12, 20, 376);

				//ImGui::Text("Radar Posistion (X,Y)%%"); ImGui::SameLine(0, 59); ImGui::PushItemWidth(130); ImGui::SliderFloat("  ", &PosDx, 0, 100); ImGui::SameLine(0, 2); ImGui::PushItemWidth(130); ImGui::SliderFloat("    ", &PosDy, 0, 100);
				//ImGui::Text("Radar Size(px), Resolut on(m)"); ImGui::SameLine(0, 10); ImGui::PushItemWidth(130); ImGui::SliderFloat("   ", &settings.radar_xsize, 0, 350); ImGui::SameLine(0, 0); ImGui::PushItemWidth(130); ImGui::SliderFloat("      ", &settings.radar_ysize, 0, 300);

				PS::CL_ToggleButton("Skeleton ESP", &settings.skeleton, 12, 50, 376);

				//ImGui::Text("Skeleton Only Behind Walls"); ImGui::SameLine(0, 198); ImGui::Checkbox("                                                     ", &settings.skel_walls);
				//PS::CL_ToggleButton("Loot ESP", &settings.loot_esp, 12, 80, 376);

				//ImGui::Text("Pickup Distance Limit (m)"); ImGui::SameLine(0, 150); ImGui::PushItemWidth(130); ImGui::SliderFloat("         ", &settings.pickUp_dis, 0, 1000);
				//ImGui::Text("Supply/Chest/Ammo/Trap Distance Limit (m)"); ImGui::SameLine(0, 37); ImGui::PushItemWidth(130); ImGui::SliderFloat("                        ", &settings.supply_chest, 0, 1000);

				PS::CL_ToggleButton("Vehicle ESP", &settings.vehicle, 12, 80, 376);
				//ImGui::Text("Vechicle Distance Limit (m)"); ImGui::SameLine(0, 135); ImGui::PushItemWidth(130); ImGui::SliderFloat("           ", &settings.vech_dis, 0, 1000);

				//ImGui::Text("Trap/Projectiles ESP"); ImGui::SameLine(0, 241); ImGui::Checkbox("            ", &settings.trap_esp);
				//ImGui::Text("Visuals Toggle Button"); ImGui::SameLine(0, 85); ImGui::Checkbox("             ", &settings.visual_toggle);

				ImGui::PopStyleColor();
			}
			if (tabs == 2)
			{
				ImGui::SetCursorPos({ 10, 80 });
				ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0f, 0.16f, 0.42f, 1.0f));
				ImGui::BeginChild("child", { 502.f,530.f }, true);
				PS::CL_ToggleButton("Aimbot", &settings.memory, 12, 20, 376);
				//ImGui::Text("Aimbot Key");
				//ImGui::Text("Head Rate"); ImGui::SameLine(0, 265); ImGui::PushItemWidth(130); ImGui::SliderFloat(" ", &settings.head_rate, 0, 100);
				//ImGui::Text("Neck Rate"); ImGui::SameLine(0, 265); ImGui::PushItemWidth(130); ImGui::SliderFloat("  ", &settings.neck_rate, 0, 100);
				//ImGui::Text("Chest Rate"); ImGui::SameLine(0, 258); ImGui::PushItemWidth(130); ImGui::SliderFloat("   ", &settings.chest_rate, 0, 100);
				//ImGui::Text("Pelvis Rate"); ImGui::SameLine(0, 251); ImGui::PushItemWidth(130); ImGui::SliderFloat("    ", &settings.pelvis_rate, 0, 100);

				//ImGui::Text("Refresh Rate"); ImGui::SameLine(0, 244); ImGui::PushItemWidth(130); ImGui::SliderFloat("     ", &settings.refresh_rate, 0, 1);

				ImGui::Text("Aimbot Smooth"); ImGui::SameLine(0, 237); ImGui::PushItemWidth(130); ImGui::SliderFloat("      ", &settings.smoothness, 0, 25);
				ImGui::Text("Aimbot Fov"); ImGui::SameLine(0, 258); ImGui::PushItemWidth(130); ImGui::SliderFloat("       ", &settings.radius, 0, 1000);

				//ImGui::Text("Aimbot Distance Limit (m)"); ImGui::SameLine(0, 153); ImGui::PushItemWidth(130); ImGui::SliderFloat("        ", &settings.aimbot_distance, 0, 505);
				//ImGui::Text("Velocity Adjust"); ImGui::SameLine(0, 223); ImGui::PushItemWidth(130); ImGui::SliderFloat("         ", &settings.velocity_ajust, 0, 2);
				//ImGui::Text("Aim Shake (+- cm)"); ImGui::SameLine(0, 209); ImGui::PushItemWidth(130); ImGui::SliderFloat("          ", &settings.aim_shake, 0, 50);
				//ImGui::Text("Shake Speed (cm/s)"); ImGui::SameLine(0, 202); ImGui::PushItemWidth(130); ImGui::SliderFloat("           ", &settings.shake_speed, 0, 100);
				ImGui::PopStyleColor();
			}
			if (tabs == 3)
			{
				ImGui::SetCursorPos({ 10, 80 });
				ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0f, 0.16f, 0.42f, 1.0f));
				ImGui::BeginChild("child", { 502.f,530.f }, true);
				//ImGui::Text("Triggerbot"); ImGui::SameLine(0, 310); ImGui::Checkbox("", &settings.trigger_bot);
				//ImGui::Text("Delay (ms)"); ImGui::SameLine(0, 258); ImGui::PushItemWidth(130); ImGui::SliderFloat(" ", &settings.delay, 0, 300);
				//ImGui::Text("Triggerbot distance limit (m)"); ImGui::SameLine(0, 125); ImGui::PushItemWidth(130); ImGui::SliderFloat("  ", &settings.trigger_bot_distance_limit, 0, 300);
				//ImGui::Text("Triggerbot spray distance limit (m)"); ImGui::SameLine(0, 83); ImGui::PushItemWidth(130); ImGui::SliderFloat("   ", &settings.spray, 0, 300);
				//ImGui::Text("Triggerbot spread"); ImGui::SameLine(0, 209); ImGui::PushItemWidth(130); ImGui::SliderFloat("                     ", &settings.trigger_spread, 0, 10);
				//ImGui::Text("");
				//ImGui::Text("                 Exploits (DETECTION RISK!) Below");
				PS::CL_ToggleButton("No bloom (semi detected!)", &settings.NoBloom, 12, 20, 376);
				PS::CL_ToggleButton("No spread (very detected!)", &settings.nospread, 12, 50, 376);
				//PS::CL_ToggleButton("Silent Aim (undetected for now!)", &settings.silentAim, 12, 80, 376);
				//ImGui::Text("Projectile bullet teleport"); ImGui::SameLine(0, 198); ImGui::Checkbox("        ", &settings.bullet_tp);
				//ImGui::Text("SpeedHack"); ImGui::SameLine(0, 268); ImGui::PushItemWidth(130); ImGui::SliderFloat("                  ", &settings.speedhack, 0, 20.5);

				PS::CL_ToggleButton("SpeedHack Toggle/Hotkey", &settings.speedhack, 12, 80, 376);
				//ImGui::Text("Player Teleport (may freeze)"); ImGui::SameLine(0, 36); ImGui::Checkbox("          ", &settings.player_tp);
				//ImGui::Text("Limit Teleport Distance (No Damage Limit)"); ImGui::SameLine(0, 92); ImGui::Checkbox("            ", &settings.limit_tp_dis);
				//PS::CL_ToggleButton("instant Revive", &settings.InstantRevive, 12, 140, 376);
				ImGui::PopStyleColor();
			}
			if (tabs == 4)
			{
				ImGui::SetCursorPos({ 10, 80 });
				ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0f, 0.16f, 0.42f, 1.0f));
				ImGui::BeginChild("child", { 502.f,530.f }, true);
				PS::CL_ToggleButton("Debug Info", &settings.debug_info, 12, 20, 376);
				//ImGui::Text("Debug World Items Max Distance (m)"); ImGui::SameLine(0, 90); ImGui::PushItemWidth(130); ImGui::SliderFloat("          ", &settings.debug_world_items, 0, 300);

				PS::CL_ToggleButton("Visibility Check", &settings.vischeck, 12, 50, 376);
				//ImGui::Text("Aimbot Aim Line"); ImGui::SameLine(0, 275); ImGui::Checkbox("  ", &settings.aim_line);
				PS::CL_ToggleButton("Aimbot Fov Circle", &settings.aimbot_fov, 12, 80, 376);
				PS::CL_ToggleButton("Crosshair", &settings.crosshair, 12, 110, 376);
				//ImGui::Text("Lines from crosshair/muzzle"); ImGui::SameLine(0, 191); ImGui::Checkbox("     ", &settings.lines_from_muzzle);
				//ImGui::Text("Show menu on start"); ImGui::SameLine(0, 254); ImGui::Checkbox("         ", &settings.show_menu);
				//ImGui::Text("Scale Text"); ImGui::SameLine(0, 310); ImGui::Checkbox("      ", &settings.scale_text);
				PS::CL_ToggleButton("Aim while jumping", &settings.AimWhileJumping, 12, 140, 376);
				//ImGui::Text("Stream Proof"); ImGui::SameLine(0, 296); ImGui::Checkbox("        ", &settings.stream_proof);
				//ImGui::Text("ManhattenQC ESP"); ImGui::SameLine(0, 275); ImGui::Checkbox("           ", &settings.ManhattenQC);
				ImGui::Text("Discord server"); ImGui::SameLine(0, 275); if (ImGui::Button("Discord", { 90, 30 })) {
					system("start https://discord.gg/pdGHbFptpy");
				}
				ImGui::Text("Reset Dx"); ImGui::SameLine(0, 317); if (ImGui::Button("Reset", { 90, 30 })) {
					system("start https://discord.gg/pdGHbFptpy");
				}
				ImGui::PopStyleColor();
			}
			ImGui::End();
		}
	}
}
auto addrress = SDK::Utilities::Scanners::PatternScan(xorthis("E8 ? ? ? ? 48 8D 4B 28 E8 ? ? ? ? 48 8B C8")); //CalculateSpreadHook Signature

float* (*CalculateShot)(PVOID, PVOID, PVOID) = nullptr;
float* CalculateShotHook(PVOID arg0, PVOID arg1, PVOID arg2) {
	auto ret = CalculateShot(arg0, arg1, arg2);
	if (ret && settings.silentAim && LocalPawn)
	{

		SDK::Structs::Vector3 headvec3;
		SDK::Classes::USkeletalMeshComponent::GetBoneLocation((uintptr_t)LocalPawn, 66, &headvec3);
		SDK::Structs::FVector head = { headvec3.x, headvec3.y , headvec3.z };

		uintptr_t RootComp = SDK::Utilities::read<uintptr_t>(LocalPawn + SDK::Classes::StaticOffsets::RootComponent);
		SDK::Structs::Vector3 RootCompLocationvec3 = SDK::Utilities::read<SDK::Structs::Vector3>(RootComp + SDK::Classes::StaticOffsets::RelativeLocation);
		SDK::Structs::FVector RootCompLocation = { RootCompLocationvec3.x, RootCompLocationvec3.y , RootCompLocationvec3.z };
		SDK::Structs::FVector* RootCompLocation_check = &RootCompLocation;
		if (!RootCompLocation_check) return ret;
		auto root = RootCompLocation;

		auto dx = head.X - root.X;
		auto dy = head.Y - root.Y;
		auto dz = head.Z - root.Z;
		if (dx * dx + dy * dy + dz * dz < 125000.0f) {
			ret[4] = head.X;
			ret[5] = head.Y;
			ret[6] = head.Z;
		}
		else {
			head.Z -= 16.0f;
			root.Z += 45.0f;

			auto y = atan2f(head.Y - root.Y, head.X - root.X);

			root.X += cosf(y + 1.5708f) * 32.0f;
			root.Y += sinf(y + 1.5708f) * 32.0f;

			auto length = sqrtf(powf(head.X - root.X, 2) + powf(head.Y - root.Y, 2));
			auto x = -atan2f(head.Z - root.Z, length);
			y = atan2f(head.Y - root.Y, head.X - root.X);

			x /= 2.0f;
			y /= 2.0f;

			ret[0] = -(sinf(x) * sinf(y));
			ret[1] = sinf(x) * cosf(y);
			ret[2] = cosf(x) * sinf(y);
			ret[3] = cosf(x) * cosf(y);
		}
	}

	return ret;
}