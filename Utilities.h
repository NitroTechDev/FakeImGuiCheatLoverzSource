#include <conio.h>
#include <Windows.h>
#include <string>
#include <Psapi.h>
#include <d3d11.h>
#pragma comment (lib, "d3d11.lib")

#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_internal.h"

#include "hook/MinHook.h"
#pragma comment (lib, "minhook.lib")

float X, Y;
#define _CRT_SECURE_NO_WARNINGS
namespace Utils
{
	ImGuiWindow& createscene();
	VOID destroyscene(ImGuiWindow& window);
}

namespace detail
{
	extern "C" void* _spoofer_stub();

	template <typename Ret, typename... Args>
	static inline auto shellcode_stub_helper(
		const void* shell,
		Args... args
	) -> Ret
	{
		auto fn = (Ret(*)(Args...))(shell);
		return fn(args...);
	}

	template <std::size_t Argc, typename>
	struct argument_remapper
	{
		template<
			typename Ret,
			typename First,
			typename Second,
			typename Third,
			typename Fourth,
			typename... Pack
		>
			static auto do_call(const void* shell, void* shell_param, First first, Second second,
				Third third, Fourth fourth, Pack... pack) -> Ret
		{
			return shellcode_stub_helper< Ret, First, Second, Third, Fourth, void*, void*, Pack... >(shell, first, second, third, fourth, shell_param, nullptr, pack...);
		}
	};

	template <std::size_t Argc>
	struct argument_remapper<Argc, std::enable_if_t<Argc <= 4>>
	{
		template<
			typename Ret,
			typename First = void*,
			typename Second = void*,
			typename Third = void*,
			typename Fourth = void*
		>
			static auto do_call(
				const void* shell,
				void* shell_param,
				First first = First{},
				Second second = Second{},
				Third third = Third{},
				Fourth fourth = Fourth{}
			) -> Ret
		{
			return shellcode_stub_helper<
				Ret,
				First,
				Second,
				Third,
				Fourth,
				void*,
				void*
			>(
				shell,
				first,
				second,
				third,
				fourth,
				shell_param,
				nullptr
				);
		}
	};
}
