#define _CRT_SECURE_NO_WARNINGS 124
#include "Utilities.h"
#include "CheatSettings.h"

#pragma once
#define M_PI	3.14159265358979323846264338327950288419716939937510
#define RELATIVE_ADDR(addr, size) ((uintptr_t)((UINT_PTR)(addr) + *(PINT)((UINT_PTR)(addr) + ((size) - sizeof(INT))) + (size)))
#define BIT_CHECK(a,b) (!!((a) & (1ULL<<(b))))
#define ReadBYTE(base, offset) (*(((PBYTE)base + offset)))

#define WIN32_LINUX(win32, linux) win32

#include "xorstring.h"
#include <vector>
#include <algorithm>

class cDetails {
public:
	uintptr_t UWORLD;
	uintptr_t GETOBJECTNAMEBYINDEX;
	uintptr_t PROCESSEVENT;
	uintptr_t CALCULATESHOT;
	uintptr_t GETWEAPONSTATS;
};
cDetails Details;
float FOVAngle;
uintptr_t PlayerController;
uintptr_t PlayerCameraManager;
uintptr_t LocalPawn;
uintptr_t LocalWeapon;
uintptr_t LineOfS;
uintptr_t GetNameByIndex;

inline float sqrtf_(float x)
{
	union { float f; uint32_t i; } z = { x };
	z.i = 0x5f3759df - (z.i >> 1);
	z.f *= (1.5f - (x * 0.5f * z.f * z.f));
	z.i = 0x7EEEEEEE - z.i;
	return z.f;
}

double powf_(double x, int y)
{
	double temp;
	if (y == 0)
		return 1;
	temp = powf_(x, y / 2);
	if ((y % 2) == 0) {
		return temp * temp;
	}
	else {
		if (y > 0)
			return x * temp * temp;
		else
			return (temp * temp) / x;
	}
}

namespace SDK
{
	namespace Utilities {
		uintptr_t GetImageBase() {
			return (uintptr_t)GetModuleHandleA(0);
		}

		template <typename Ret, typename... Args>
		static inline auto SpoofCall(Ret(*fn)(Args...), Args... args) -> Ret
		{
			static const void* jmprbx = nullptr;
			if (!jmprbx) {
				const auto ntdll = reinterpret_cast<const unsigned char*>(::GetModuleHandleW(NULL));
				const auto dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(ntdll);
				const auto nt = reinterpret_cast<const IMAGE_NT_HEADERS*>(ntdll + dos->e_lfanew);
				const auto sections = IMAGE_FIRST_SECTION(nt);
				const auto num_sections = nt->FileHeader.NumberOfSections;

				constexpr char section_name[5]{ '.', 't', 'e', 'x', 't' };
				const auto     section = std::find_if(sections, sections + num_sections, [&](const auto& s) {
					return std::equal(s.Name, s.Name + 5, section_name);
					});

				constexpr unsigned char instr_bytes[2]{ 0xFF, 0x26 };
				const auto              va = ntdll + section->VirtualAddress;
				jmprbx = std::search(va, va + section->Misc.VirtualSize, instr_bytes, instr_bytes + 2);
			}

			struct shell_params
			{
				const void* trampoline;
				void* function;
				void* rdx;
			};

			shell_params p
			{
				jmprbx,
				reinterpret_cast<void*>(fn)
			};

			using mapper = detail::argument_remapper<sizeof...(Args), void>;
			return mapper::template do_call<Ret, Args...>((const void*)&detail::_spoofer_stub, &p, args...);
		}

		BOOL valid_pointer(DWORD64 address)
		{
			if (!SpoofCall(IsBadWritePtr, (LPVOID)address, (UINT_PTR)8)) return TRUE;
			else return FALSE;
		}

		template<typename T>
		T read(DWORD_PTR address, const T& def = T())
		{
			if (valid_pointer(address))
				return *(T*)address;
			else
				return T();
		}

		template<typename T>
		T write(DWORD_PTR address, DWORD_PTR value, const T& def = T())
		{
			if (valid_pointer(address))
				return *(T*)address = value;
			else
				return T();
		}

		namespace Scanners
		{
			uintptr_t PatternScan(uintptr_t pModuleBaseAddress, const char* sSignature, size_t nSelectResultIndex = 0);
			uintptr_t PatternScan(const char* sSignature, size_t nSelectResultIndex = 0);

			uintptr_t PatternScan(uintptr_t pModuleBaseAddress, const char* sSignature, size_t nSelectResultIndex)
			{
				static auto patternToByte = [](const char* pattern)
				{
					auto       bytes = std::vector<int>{};
					const auto start = const_cast<char*>(pattern);
					const auto end = const_cast<char*>(pattern) + strlen(pattern);

					for (auto current = start; current < end; ++current)
					{
						if (*current == '?')
						{
							++current;
							if (*current == '?')
								++current;
							bytes.push_back(-1);
						}
						else
							bytes.push_back(strtoul(current, &current, 16));
					}
					return bytes;
				};

				const auto dosHeader = (PIMAGE_DOS_HEADER)pModuleBaseAddress;
				const auto ntHeaders = (PIMAGE_NT_HEADERS)((std::uint8_t*)pModuleBaseAddress + dosHeader->e_lfanew);

				const auto sizeOfImage = ntHeaders->OptionalHeader.SizeOfImage;
				auto       patternBytes = patternToByte(sSignature);
				const auto scanBytes = reinterpret_cast<std::uint8_t*>(pModuleBaseAddress);

				const auto s = patternBytes.size();
				const auto d = patternBytes.data();

				size_t nFoundResults = 0;

				for (auto i = 0ul; i < sizeOfImage - s; ++i)
				{
					bool found = true;

					for (auto j = 0ul; j < s; ++j)
					{
						if (scanBytes[i + j] != d[j] && d[j] != -1)
						{
							found = false;
							break;
						}
					}

					if (found)
					{
						if (nSelectResultIndex != 0)
						{
							if (nFoundResults < nSelectResultIndex)
							{
								nFoundResults++;
								found = false;
							}
							else
								return reinterpret_cast<uintptr_t>(&scanBytes[i]);
						}
						else
							return reinterpret_cast<uintptr_t>(&scanBytes[i]);
					}
				}

				return NULL;
			}

			uintptr_t PatternScan(const char* sSignature, size_t nSelectResultIndex)
			{
				static bool bIsSetted = false;

				static MODULEINFO info = { 0 };

				if (!bIsSetted)
				{
					GetModuleInformation(GetCurrentProcess(), GetModuleHandle(0), &info, sizeof(info));
					bIsSetted = true;
				}

				return PatternScan((uintptr_t)info.lpBaseOfDll, sSignature, nSelectResultIndex);
			}
		}
	}

	namespace Structs {
		class UClass {
		public:
			BYTE _padding_0[0x40];
			UClass* SuperClass;
		};

		class UObject {
		public:
			PVOID VTableObject;
			DWORD ObjectFlags;
			DWORD InternalIndex;
			UClass* Class;
			BYTE _padding_0[0x8];
			UObject* Outer;

			inline BOOLEAN IsA(PVOID parentClass) {
				for (auto super = this->Class; super; super = super->SuperClass) {
					if (super == parentClass) {
						return TRUE;
					}
				}

				return FALSE;
			}
		};

		class FLinearColor {
		public:

		};

		template<class T>
		struct TArray
		{
			friend struct FString;

		public:
			inline TArray()
			{
				Data = nullptr;
				Count = Max = 0;
			};

			inline int Num() const
			{
				return Count;
			};

			inline T& operator[](int i)
			{
				return Data[i];
			};

			inline const T& operator[](int i) const
			{
				return Data[i];
			};

			inline bool IsValidIndex(int i) const
			{
				return i < Num();
			}

		private:
			T* Data;
			int32_t Count;
			int32_t Max;
		};

		class FText {
		private:
			char _padding_[0x28];
			PWCHAR Name;
			DWORD Length;

		public:
			inline PWCHAR c_str() {
				return Name;
			}
		};

		struct FString : private TArray<wchar_t>
		{
			inline FString()
			{
			};

			FString(const wchar_t* other)
			{
				Max = Count = *other ? std::wcslen(other) + 1 : 0;

				if (Count)
				{
					Data = const_cast<wchar_t*>(other);
				}
			};

			inline bool IsValid() const
			{
				return Data != nullptr;
			}

			inline const wchar_t* c_str() const
			{
				return Data;
			}

			std::string ToString() const
			{
				auto length = std::wcslen(Data);

				std::string str(length, '\0');

				std::use_facet<std::ctype<wchar_t>>(std::locale()).narrow(Data, Data + length, '?', &str[0]);

				return str;
			}
		};

		struct FVector2D {
			float x;
			float y;

			inline BOOL IsValid() {
				return X != NULL && Y != NULL;
			}

			inline float distance() {
				return sqrtf(this->x * this->x + this->y * this->y);
			}
		};

		class Vector4
		{
		public:
			// Constructors
			Vector4() : x(0.f), y(0.f), z(0.f), w(0.f)
			{

			}

			Vector4(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w)
			{

			}

			Vector4(float _v) : x(_v), y(_v), z(_v), w(_v)
			{

			}

			~Vector4()
			{

			}

			// Variables
			float x;
			float y;
			float z;
			float w;

			// Functions
			inline Vector4 ToColor(float fTransparant)
			{
				return Vector4(x, y, z, fTransparant);
			}

			inline ImVec4 ToImVec()
			{
				return ImVec4(x, y, z, w);
			}

			inline bool IsZero()
			{
				if (x == 0 && y == 0 && z == 0 && w == 0)
					return true;

				return false;
			}

			// Operators
			Vector4 operator+(Vector4 v)
			{
				return Vector4(x + v.x, y + v.y, z + v.z, y + v.w);
			}

			Vector4 operator-(Vector4 v)
			{
				return Vector4(x - v.x, y - v.y, z - v.z, y - v.w);
			}

			Vector4 operator*(float number) const {
				return Vector4(x * number, y * number, z * number, y * number);
			}

			Vector4 operator/(float number) const {
				return Vector4(x / number, y / number, z / number, y / number);
			}
		};

		class Vector3
		{
		public:
			// Constructors
			Vector3() : x(0.f), y(0.f), z(0.f)
			{

			}

			Vector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z)
			{

			}

			Vector3(float _v) : x(_v), y(_v), z(_v)
			{

			}

			~Vector3()
			{

			}

			// Variables
			float x;
			float y;
			float z;

			//Functions
			inline bool Zero() const
			{
				return (x > -0.1f && x < 0.1f && y > -0.1f && y < 0.1f && z > -0.1f && z < 0.1f);
			}

			inline float Dot(Vector3 v)
			{
				return x * v.x + y * v.y + z * v.z;
			}

			// Functions
			inline float Distance(Vector3 v)
			{
				return Utilities::SpoofCall(sqrtf, (Utilities::SpoofCall(powf, (v.x - x), 2.0f) + Utilities::SpoofCall(powf, (v.y - y), 2.0f) + Utilities::SpoofCall(powf, (v.z - z), 2.0f))) / 100.0f;
			}

			inline double Length() {
				return sqrt(x * x + y * y + z * z);
			}

			// Operators
			Vector3 operator+(Vector3 v)
			{
				return Vector3(x + v.x, y + v.y, z + v.z);
			}

			Vector3 operator-(Vector3 v)
			{
				return Vector3(x - v.x, y - v.y, z - v.z);
			}

			Vector3 operator*(float number) const {
				return Vector3(x * number, y * number, z * number);
			}

			Vector3 operator/(float number) const {
				return Vector3(x / number, y / number, z / number);
			}
		};

		class FRotator
		{
		public:
			FRotator() : Pitch(0.f), Yaw(0.f), Roll(0.f)
			{

			}

			FRotator(float _Pitch, float _Yaw, float _Roll) : Pitch(_Pitch), Yaw(_Yaw), Roll(_Roll)
			{

			}
			~FRotator()
			{

			}

			float Pitch;
			float Yaw;
			float Roll;


			double Length() {
				return sqrt(Pitch * Pitch + Yaw * Yaw + Roll * Roll);
			}

			FRotator operator+(FRotator angB) { return FRotator(Pitch + angB.Pitch, Yaw + angB.Yaw, Roll + angB.Roll); }
			FRotator operator-(FRotator angB) { return FRotator(Pitch - angB.Pitch, Yaw - angB.Yaw, Roll - angB.Roll); }
			FRotator operator/(float flNum) { return FRotator(Pitch / flNum, Yaw / flNum, Roll / flNum); }
			FRotator operator*(float flNum) { return FRotator(Pitch * flNum, Yaw * flNum, Roll * flNum); }
			bool operator==(FRotator angB) { return Pitch == angB.Pitch && Yaw == angB.Yaw && Yaw == angB.Yaw; }
			bool operator!=(FRotator angB) { return Pitch != angB.Pitch || Yaw != angB.Yaw || Yaw != angB.Yaw; }

		};

		typedef struct {
			float X, Y, Z;
		} FVector;

		class PlayerInfo {
		public:
			uintptr_t Mesh;
			uintptr_t Raw;
			uintptr_t State;
			uintptr_t TeamIndex;
		};

		class FMinimalViewInfo
		{
		public:
			Vector3 Loc;
			Vector3 Rot;
			float FOV;
		};

		struct FMatrix
		{
			float M[4][4];
		};
		static FMatrix* myMatrix = new FMatrix();
	}

	namespace Classes
	{
		namespace StaticOffsets {
			uintptr_t OwningGameInstance = 0x180;
			uintptr_t LocalPlayers = 0x38;
			uintptr_t PlayerController = 0x30;
			uintptr_t PlayerCameraManager = 0x2B8;
			uintptr_t AcknowledgedPawn = 0x2A0;

			uintptr_t Levels = 0x138;
			uintptr_t PersistentLevel = 0x30;
			uintptr_t AActors = 0x98;
			uintptr_t CostumTimeDilation = 0x98;
			uintptr_t ActorCount = 0xA0;

			uintptr_t RootComponent = 0x130;
			uintptr_t FireStartLoc = 0x878;
			uintptr_t RelativeLocation = 0x11c;
			uintptr_t RelativeRotation = 0x128;
			uintptr_t CurrentWeapon = 0x600;
			//uintptr_t PreviousWeapon = 0x5F8;
			uintptr_t PlayerState = 0x240;
			uintptr_t Mesh = 0x280;
			uintptr_t TeamIndex = 0xED0;
			uintptr_t SquadID = 0x1054;

			uintptr_t ProjectileGravityScale = 0x10c;

			uintptr_t bIsDying = 0x540;
			uintptr_t bIsDBNO = 0x57E;

			uintptr_t WeaponData = 0x378;
			uintptr_t DisplayName = 0x88;
			uintptr_t PrimaryPickupItemEntry = 0x2A8;
			uintptr_t ItemDefinition = 0x18;
			uintptr_t Tier = 0x6C;

			uintptr_t LastFireTime = 0x9BC;
			uintptr_t LastFireTimeVerified = 0x9C0;//0x900;

			uintptr_t ReloadTime = 0xFC; // 0x10c(0x04) //FFortBaseWeaponStats
			//uintptr_t ReloadScale = 0x110; // 0x110(0x04) //FFortBaseWeaponStats
			//uintptr_t ChargeDownTime = 0x13C; // 0x13C(0x04) //FFortBaseWeaponStats

			uintptr_t bAlreadySearched = 0xD49;
			//uintptr_t IsReloading = 0x2B1;
		}

		/*namespace StaticOffsets {
			uintptr_t OwningGameInstance = 0x180;
			uintptr_t LocalPlayers = 0x38;
			uintptr_t PlayerController = 0x30;
			uintptr_t PlayerCameraManager = 0x2B8;
			uintptr_t AcknowledgedPawn = 0x2A0;

			uintptr_t Levels = 0x138;
			uintptr_t PersistentLevel = 0x30;
			uintptr_t AActors = 0x98;
			uintptr_t CostumTimeDilation = 0x98;
			uintptr_t ActorCount = 0xA0;

			uintptr_t RootComponent = 0x130;
			uintptr_t FireStartLoc = 0x878;
			uintptr_t RelativeLocation = 0x11c;
			uintptr_t RelativeRotation = 0x128;
			uintptr_t CurrentWeapon = 0x5F0;
			uintptr_t PreviousWeapon = 0x5F8;
			uintptr_t PlayerState = 0x240;
			uintptr_t Mesh = 0x280;
			uintptr_t TeamIndex = 0xED0;
			uintptr_t SquadID = 0x103C;

			uintptr_t Spread = 0x16C;
			uintptr_t SpreadDownsights = 0x170;

			uintptr_t bIsDying = 0x538;
			uintptr_t bIsDBNO = 0x552;

			uintptr_t WeaponData = 0x378;
			uintptr_t DisplayName = 0x80;
			uintptr_t PrimaryPickupItemEntry = 0x2A8;
			uintptr_t ItemDefinition = 0x18;
			uintptr_t Tier = 0x64;

			uintptr_t LastFireTime = 0x9BC;
			uintptr_t LastFireTimeVerified = 0x9C0;//0x900;

			uintptr_t ReloadTime = 0x10c; // 0x10c(0x04) n
			uintptr_t ReloadScale = 0x110; // 0x110(0x04) //FFortBaseWeaponStats

			uintptr_t bAlreadySearched = 0xC61;
			uintptr_t IsReloading = 0x2B1;
		}*/

		class USkeletalMeshComponent
		{
		private:
		public:
			static Structs::Vector3 GetBoneLocation(uintptr_t CurrentActor, int id, Structs::Vector3* out)
			{
				static uintptr_t addr = 0;
				if (!addr) {
					addr = SDK::Utilities::Scanners::PatternScan(xorthis("E8 ? ? ? ? 48 8B 47 30 F3 0F 10 45"));
					addr = (addr + *(DWORD*)(addr + 0x1) + 0x5 - 0x100000000);
					if (!addr) {
						SDK::Utilities::SpoofCall(MessageBoxA, (HWND)0, (LPCSTR)xorthis("Something Failed!!"), (LPCSTR)0, (UINT)0);
						exit(0);
					}
				}

				uintptr_t mesh = SDK::Utilities::read<uintptr_t>(CurrentActor + StaticOffsets::Mesh);
				if (!mesh) return { 0,0,0 };

				auto fGetBoneMatrix = ((Structs::FMatrix * (__fastcall*)(uintptr_t, Structs::FMatrix*, int))(addr));
				Utilities::SpoofCall(fGetBoneMatrix, mesh, Structs::myMatrix, id);

				out->x = Structs::myMatrix->M[3][0];
				out->y = Structs::myMatrix->M[3][1];
				out->z = Structs::myMatrix->M[3][2];

				return Structs::Vector3(0, 0, 0);
			}
		};

		class AK2Vehicle
		{
		public:
			uintptr_t IsInVehicle = 0x1c7d29c;
		};

		class APlayerCameraManager
		{
		public:
			static float GetFOVAngle(uintptr_t PlayerCameraManager)
			{
				auto GetFOVAngle = reinterpret_cast<float(*)(UINT64, char*)>(*(ULONG_PTR*)(*(ULONG_PTR*)PlayerCameraManager + 0x6D0));
				return Utilities::SpoofCall(GetFOVAngle, (ULONG_PTR)PlayerCameraManager, (char*)0);
			}

			static Structs::Vector3 GetCameraLocation(uintptr_t PlayerCameraManager)
			{
				auto GetCameraLocation = reinterpret_cast<Structs::Vector3(*)(UINT64, char*)>(*(ULONG_PTR*)(*(ULONG_PTR*)PlayerCameraManager + 0x718));
				return Utilities::SpoofCall(GetCameraLocation, (ULONG_PTR)PlayerCameraManager, (char*)0);
			}

			static Structs::Vector3 GetCameraRotation(uintptr_t PlayerCameraManager)
			{
				auto GetCameraRotation = reinterpret_cast<Structs::Vector3(*)(UINT64, char*)>(*(ULONG_PTR*)(*(ULONG_PTR*)PlayerCameraManager + 0x710));
				return Utilities::SpoofCall(GetCameraRotation, (ULONG_PTR)PlayerCameraManager, (char*)0);
			}

			static VOID GetCameraInfo(uintptr_t Controller, Structs::Vector3* pLocation, Structs::Vector3* pRotation) {
				auto GetPlayerViewPoint = reinterpret_cast<char(*)(ULONG_PTR, Structs::Vector3*, Structs::Vector3*)>(*(ULONG_PTR*)(*(ULONG_PTR*)Controller + 0x708));
				Utilities::SpoofCall(GetPlayerViewPoint, (ULONG_PTR)Controller, pLocation, pRotation);
			}
			static BOOLEAN LineOfSightTo(PVOID PlayerController, PVOID Actor, SDK::Structs::Vector3* ViewPoint) {

				auto LOSTo = reinterpret_cast<bool(__fastcall*)(PVOID PlayerController, PVOID Actor, SDK::Structs::Vector3 * ViewPoint)>(LineOfS);

				return Utilities::SpoofCall(LOSTo, PlayerController, Actor, ViewPoint);
			}
			uintptr_t GetCurrentState(uintptr_t base)
			{
				static uintptr_t returnval = 0;
				returnval = SDK::Utilities::read<uintptr_t>(base + SDK::Classes::StaticOffsets::PlayerState);
				return returnval;
			}
		};

		class AController
		{
		public:
			static uintptr_t GetControlRotation(uintptr_t a1, uintptr_t a2, uintptr_t a3)
			{
				uintptr_t v3; // rbx
				uintptr_t v4; // rax
				uintptr_t result; // rax
				char v6; // [rsp+20h] [rbp-18h]

				v3 = a3;
				*(uintptr_t*)(a2 + 32) += *(uintptr_t*)(a2 + 32) != 0x0;
				v4 = (*(__int64(__fastcall**)(__int64, char*))(*(uintptr_t*)a1 + 0x688))(a1, &v6);
				*(uintptr_t*)v3 = *(uintptr_t*)v4;
				result = *(unsigned int*)(v4 + sizeof(uintptr_t));
				*(uintptr_t*)(v3 + 8) = result;
				return result;
			}

			static bool LineOfSightTo(uintptr_t Actor)
			{
				auto LineOfSightTo_ = (*(bool(__fastcall**)(uintptr_t Controller, uintptr_t Other, SDK::Structs::Vector3 ViewPoint, bool bAlternativeChecks))(*(uintptr_t*)PlayerController + 0x698));
				return SDK::Utilities::SpoofCall(LineOfSightTo_, PlayerController, Actor, SDK::Structs::Vector3(0, 0, 0), true);
			}

			static void ClientSetRotation(SDK::Structs::Vector3 NewRotation, bool bResetCamera = false)
			{
				auto ClientSetRotation_ = (*(void(__fastcall**)(uintptr_t Controller, SDK::Structs::Vector3 NewRotation, bool bResetCamera))(*(uintptr_t*)PlayerController + 0x640));
				SDK::Utilities::SpoofCall(ClientSetRotation_, PlayerController, NewRotation, bResetCamera);
			}

			static void ValidateClientSetRotation(SDK::Structs::Vector3 NewRotation, bool bResetCamera = false)
			{
				auto ValidateClientSetRotation_ = (*(void(__fastcall**)(uintptr_t Controller, SDK::Structs::Vector3 NewRotation, bool bResetCamera))(*(uintptr_t*)PlayerController + 0x638));
				SDK::Utilities::SpoofCall(ValidateClientSetRotation_, PlayerController, NewRotation, bResetCamera);
			}

			static void SetControlRotation(SDK::Structs::Vector3 NewRotation, bool bResetCamera = false)
			{
				auto SetControlRotation_ = (*(void(__fastcall**)(uintptr_t Controller, SDK::Structs::Vector3 NewRotation, bool bResetCamera))(*(uintptr_t*)PlayerController + 0x688));
				SDK::Utilities::SpoofCall(SetControlRotation_, PlayerController, NewRotation, bResetCamera);
			}

			static void SetIgnoreLookInput(char bNewLookInput)
			{
				auto SetIgnoreLookInput_ = (*(void(__fastcall**)(uintptr_t Controller, char bNewLookInput))(*(uintptr_t*)PlayerController + 0x748));
				SDK::Utilities::SpoofCall(SetIgnoreLookInput_, (uintptr_t)PlayerController, bNewLookInput);
			}

			static void ClientSetCameraMode(int h)
			{
				auto ClientSetCameraMode_ = (*(void(__fastcall**)(uintptr_t Controller, int h))(*(uintptr_t*)PlayerController + 0x920));
				SDK::Utilities::SpoofCall(ClientSetCameraMode_, (uintptr_t)PlayerController, h);
			}

			static Structs::Vector3 WorldToScreen(SDK::Structs::Vector3 vWorldPos, SDK::Structs::Vector3* vScreenPos)
			{
				static uintptr_t addr = 0;

				if (!addr) {
					addr = SDK::Utilities::Scanners::PatternScan(xorthis("E8 ? ? ? ? 41 88 07 48 83 C4 30"));;
					addr = RELATIVE_ADDR(addr, 5);
					if (!addr) {
						SDK::Utilities::SpoofCall(MessageBoxA, (HWND)0, (LPCSTR)xorthis("Something Failed"), (LPCSTR)0, (UINT)0);
						exit(0);
					}
				}

				auto WorldToScreen = reinterpret_cast<bool(__fastcall*)(uintptr_t pPlayerController, SDK::Structs::Vector3 vWorldPos, SDK::Structs::Vector3 * vScreenPosOut, char)>(addr);

				return SDK::Utilities::SpoofCall(WorldToScreen, (uintptr_t)PlayerController, vWorldPos, vScreenPos, (char)0);
				//auto WorldToScreen_ = (*(void(__fastcall**)(uintptr_t Controller, SDK::Structs::Vector3 vWorldPos, SDK::Structs::Vector3 * vScreenPosOut, char camerareset))(*(uintptr_t*)PlayerController + 0xAD8));
				//SDK::Utilities::SpoofCall(WorldToScreen_, PlayerController, vWorldPos, vScreenPos, (char)0);
			}
		};

		namespace Utils
		{
			uintptr_t State(uintptr_t base)
			{
				static uintptr_t returnval = 0;
				returnval = SDK::Utilities::read<uintptr_t>(base + SDK::Classes::StaticOffsets::PlayerState);
				return returnval;
			}

			bool CheckInScreen(uintptr_t CurrentActor, int Width, int Height) {
				SDK::Structs::Vector3 Pos;
				Classes::USkeletalMeshComponent::GetBoneLocation(CurrentActor, 66, &Pos);
				Classes::AController::WorldToScreen(Pos, &Pos);
				if (CurrentActor)
				{
					if (Pos.x > 0 and Pos.y > 0) {
						return true;
					}
					else {
						return false;
					}
				}
			}
		}
		class UCanvas
		{
		public:
			unsigned char Padding_jYrEa[0x28];
			float OrgX; // 0x28(0x4)
			// No Padding Required Here
			float OrgY; // 0x2c(0x4)
			// No Padding Required Here
			float ClipX; // 0x30(0x4)
			// No Padding Required Here
			float ClipY; // 0x34(0x4)
			// No Padding Required Here
			//struct SDK::Structs::FColor DrawColor; // 0x38(0x4)
			// No Padding Required Here
			char bCenterX : 1; // 0x3c(0x1)
			// No Padding Required Here
			char bCenterY : 1; // 0x3c(0x1)
			// No Padding Required Here
			char bNoSmooth : 1; // 0x3c(0x1)
			// No Padding Required Here
			unsigned char UnknownBuffer_GsD24 : 5; // 0x3c(0x1)
			// No Padding Required Here
			unsigned char UnknownBuffer_Pu72n[0x3]; // 0x3d(0x3)
			// No Padding Required Here
			int32_t SizeX; // 0x40(0x4)
			// No Padding Required Here
			int32_t SizeY; // 0x44(0x4)
			// No Padding Required Here
			unsigned char UnknownBuffer_67hdW[0x8]; // 0x48(0x8)
			// No Padding Required Here
			unsigned char ColorModulate[0x10]; // 0x50(0x10)
			// No Padding Required Here
			PVOID DefaultTexture; // 0x60(0x8)
			// No Padding Required Here
			PVOID GradientTexture0; // 0x68(0x8)
			// No Padding Required Here
			PVOID ReporterGraph; // 0x70(0x8)
		};
	}

	namespace Utilities {
		Structs::Vector3 CamLoc;
		Structs::Vector3 CamRot;

		double GetCrossDistance(double x1, double y1, double x2, double y2)
		{
			return sqrtf_(powf_((float)(x1 - x2), (float)2) + powf_((float)(y1 - y2), (float)2));
		}
		bool CheckInScreen(uintptr_t CurrentActor, int Width, int Height) {
			SDK::Structs::Vector3 Pos;
			Classes::USkeletalMeshComponent::GetBoneLocation(CurrentActor, 66, &Pos);
			Classes::AController::WorldToScreen(Pos, &Pos);
			if (CurrentActor)
			{
				if (Pos.x > 0 and Pos.y > 0) {
					return true;
				}
				else {
					return false;
				}
			}
		}
		bool CheckIfInFOV(uintptr_t CurrentActor, float& max)
		{
			SDK::Structs::Vector3 Pos;
			Classes::USkeletalMeshComponent::GetBoneLocation(CurrentActor, 66, &Pos);
			Classes::AController::WorldToScreen(Pos, &Pos);
			if (CurrentActor)
			{
				float Dist = GetCrossDistance(Pos.x, Pos.y, (X / 2), (Y / 2));

				if (Dist < max)
				{
					float Radius = (settings.radius);

					if (Pos.x <= ((X / 2) + Radius) &&
						Pos.x >= ((X / 2) - Radius) &&
						Pos.y <= ((Y / 2) + Radius) &&
						Pos.y >= ((Y / 2) - Radius))
					{
						max = Dist;
						return true;
					}

					return false;
				}
			}

			return false;
		}

		Structs::Vector3 GetRotation(uintptr_t a)
		{
			Structs::Vector3 RetVector = { 0,0,0 };

			Structs::Vector3 rootHead;
			SDK::Classes::USkeletalMeshComponent::GetBoneLocation(a, 66, &rootHead);

			if (rootHead.x == 0 && rootHead.y == 0) return Structs::Vector3(0, 0, 0);

			Structs::Vector3 VectorPos = rootHead - CamLoc;

			float distance = VectorPos.Length();
			RetVector.x = -(((float)acos(VectorPos.z / distance) * (float)(180.0f / M_PI)) - 90.f);
			RetVector.y = (float)atan2(VectorPos.y, VectorPos.x) * (float)(180.0f / M_PI);

			return RetVector;
		}

		Structs::Vector3 GetSilentRotation(uintptr_t a)
		{
			Structs::Vector3 RetVector = { 0,0,0 };

			Structs::Vector3 rootHead;
			SDK::Classes::USkeletalMeshComponent::GetBoneLocation(a, 66, &rootHead);

			if (rootHead.x == 0 && rootHead.y == 0) return Structs::Vector3(0, 0, 0);

			Structs::Vector3 VectorPos = rootHead - CamLoc;

			float distance = VectorPos.Length();
			RetVector.x = -(((float)acos(VectorPos.z / distance) * (float)(180.0f / M_PI)) - 90.f);
			RetVector.y = (float)atan2(VectorPos.y, VectorPos.x) * (float)(180.0f / M_PI);

			return RetVector;
		}

		Structs::Vector3 SmoothAngles(Structs::Vector3 rot1, Structs::Vector3 rot2)
		{
			Structs::Vector3 ret;
			auto currentRotation = rot1;

			ret.x = (rot2.x - rot1.x) / settings.smoothness + rot1.x;
			ret.y = (rot2.y - rot1.y) / settings.smoothness + rot1.y;

			return ret;
		}

		Structs::Vector3 GetPawnRootLocation(uintptr_t CurrentActor)
		{
			uintptr_t RootComponent = Utilities::read<uintptr_t>(CurrentActor + Classes::StaticOffsets::RootComponent);
			Structs::Vector3 RelativeLocation = *(Structs::Vector3*)(RootComponent + Classes::StaticOffsets::RelativeLocation);
			return RelativeLocation;
		}

		Structs::Vector3 GetPawnRootRotation(uintptr_t CurrentActor)
		{
			uintptr_t RootComponent = Utilities::read<uintptr_t>(CurrentActor + Classes::StaticOffsets::RootComponent);
			Structs::Vector3 RelativeRotation = *(Structs::Vector3*)(RootComponent + Classes::StaticOffsets::RelativeRotation);
			return RelativeRotation;
		}

		namespace DiscordHelper
		{
			static uintptr_t GetDiscordModule()
			{
				std::string module = xorthis("DiscordHook64.dll");
				return (uintptr_t)SDK::Utilities::SpoofCall(GetModuleHandleA, module.c_str());
			}
			std::vector<uintptr_t> pCreatedHooksArray;
			bool CreateHook(uintptr_t pOriginal, uintptr_t pHookedFunction, uintptr_t pOriginalCall)
			{
				static uintptr_t addrCreateHook = NULL;

				if (!addrCreateHook)
					addrCreateHook = Scanners::PatternScan(GetDiscordModule(), xorthis("41 57 41 56 56 57 55 53 48 83 EC 68 4D 89 C6 49 89 D7"));

				if (!addrCreateHook)
					return false;

				using CreateHook_t = uint64_t(__fastcall*)(LPVOID, LPVOID, LPVOID*);
				auto fnCreateHook = (CreateHook_t)addrCreateHook;

				return SpoofCall(fnCreateHook, (void*)pOriginal, (void*)pHookedFunction, (void**)pOriginalCall) == 0 ? true : false;
			}

			bool EnableHookQue()
			{
				static uintptr_t addrEnableHookQueu = NULL;

				if (!addrEnableHookQueu)
					addrEnableHookQueu = Scanners::PatternScan(GetDiscordModule(), xorthis("41 57 41 56 41 55 41 54 56 57 55 53 48 83 EC 38 48 ? ? ? ? ? ? 48 31 E0 48 89 44 24 30 BE 01 00 00 00 31 C0 F0 ? ? ? ? ? ? ? 74 2B"));

				if (!addrEnableHookQueu)
					return false;

				using EnableHookQueu_t = uint64_t(__stdcall*)(VOID);
				auto fnEnableHookQueu = (EnableHookQueu_t)addrEnableHookQueu;

				return SpoofCall(fnEnableHookQueu) == 0 ? true : false;
			}

			short GetAsyncKeyState(int key)
			{
				static uintptr_t GetAsyncKeyState_addr;
				if (!GetAsyncKeyState_addr)
					GetAsyncKeyState_addr = Scanners::PatternScan(GetDiscordModule(), xorthis("48 FF ? ? ? ? ? CC CC CC CC CC CC CC CC CC 48 FF ? ? ? ? ? CC CC CC CC CC CC CC CC CC 48 83 EC 28 48 ? ? ? ? ? ? 48 85 C9"));

				if (!GetAsyncKeyState_addr)
					return false;

				auto queuehook = ((short(__fastcall*)(int))(GetAsyncKeyState_addr));
				return SpoofCall(queuehook, key);
			}

			//this
			bool EnableHook(uintptr_t pTarget, bool bIsEnabled)
			{
				static uintptr_t addrEnableHook = NULL;

				if (!addrEnableHook)
					addrEnableHook = Scanners::PatternScan(GetDiscordModule(), xorthis("41 56 56 57 53 48 83 EC 28 49 89 CE BF 01 00 00 00 31 C0 F0 ? ? ? ? ? ? ? 74"));

				if (!addrEnableHook)
					return false;

				using EnableHook_t = uint64_t(__fastcall*)(LPVOID, bool);
				auto fnEnableHook = (EnableHook_t)addrEnableHook;

				return SpoofCall(fnEnableHook, (void*)pTarget, bIsEnabled) == 0 ? true : false;
			}

			bool InsertHook(uintptr_t pOriginal, uintptr_t pHookedFunction, uintptr_t pOriginalCall)
			{
				bool bAlreadyCreated = false;
				for (auto _Hook : pCreatedHooksArray)
				{
					if (_Hook == pOriginal)
					{
						bAlreadyCreated = true;
						break;
					}
				}

				if (!bAlreadyCreated)
					bAlreadyCreated = CreateHook(pOriginal, pHookedFunction, pOriginalCall);

				if (bAlreadyCreated)
					if (EnableHook(pOriginal, true))
						if (EnableHookQue())
							return true;

				return false;
			}
			bool IsAiming()
			{
				return GetAsyncKeyState(VK_RBUTTON);
			}
		}
	}
}

bool GetPlayerViewPoint(uintptr_t PlayerController, SDK::Structs::Vector3* vCameraPos, SDK::Structs::Vector3* vCameraRot)
{
	if (!PlayerController) return false;

	static uintptr_t pGetPlayerViewPoint = 0;
	if (!pGetPlayerViewPoint)
	{
		uintptr_t VTable = *(uintptr_t*)PlayerController;
		if (!VTable)  return false;

		pGetPlayerViewPoint = *(uintptr_t*)(VTable + 0x708);
		if (!pGetPlayerViewPoint)  return false;
	}

	auto GetPlayerViewPoint = reinterpret_cast<void(__fastcall*)(uintptr_t, SDK::Structs::Vector3*, SDK::Structs::Vector3*)>(pGetPlayerViewPoint);

	SDK::Utilities::SpoofCall(GetPlayerViewPoint, (uintptr_t)PlayerController, vCameraPos, vCameraRot);

	return true;
}

void FreeObjName(__int64 address)
{
	static uintptr_t addr = 0;

	if (!addr) {
		addr = SDK::Utilities::Scanners::PatternScan(xorthis("48 85 C9 0F 84 ? ? ? ? 53 48 83 EC 20 48 89 7C 24 30 48 8B D9 48 8B 3D ? ? ? ? 48 85 FF 0F 84 ? ? ? ? 48 8B 07 4C 8B 40 30 48 8D 05 ? ? ? ? 4C 3B C0"));;
		if (!addr) {
			SDK::Utilities::SpoofCall(MessageBoxA, (HWND)0, (LPCSTR)xorthis("Something Failed"), (LPCSTR)0, (UINT)0);
			exit(0);
		}
	}

	auto func = reinterpret_cast<__int64(__fastcall*)(__int64 a1)>(addr);

	func(address);
}

std::string GetObjectName(uintptr_t Object) {

	static uintptr_t addr = 0;

	if (!addr) {
		addr = SDK::Utilities::Scanners::PatternScan(("48 89 5C 24 ? 48 89 74 24 ? 55 57 41 56 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 45 33 F6 48 8B F2 44 39 71 04 0F 85 ? ? ? ? 8B 19 0F B7 FB E8 ? ? ? ? 8B CB 48 8D 54 24"));
	}

	if (Object == NULL)
		return ("");

	auto fGetObjName = reinterpret_cast<SDK::Structs::FString * (__fastcall*)(int* index, SDK::Structs::FString * res)>(addr);

	int index = *(int*)(Object + 0x18);

	SDK::Structs::FString result;
	fGetObjName(&index, &result);

	if (result.c_str() == NULL)
		return ("");

	auto result_str = result.ToString();

	if (result.c_str() != NULL)
		FreeObjName((__int64)result.c_str());

	return result_str;
}
