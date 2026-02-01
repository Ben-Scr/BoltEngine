#include <Bolt.hpp>
#include "GameSystem.hpp"
#include "Systems/ParticleUpdateSystem.hpp"
#include "Core/Memory.hpp"

#include <iostream>
#include <string>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstdint>
#include <cstdio>
#include <vector>
#include <algorithm>

static uint64_t GetTotalAllocatedProcessHeapBytes()
{
    // 1) Get heap handles
    DWORD heapCount = GetProcessHeaps(0, nullptr);
    if (heapCount == 0) return 0;

    std::vector<HANDLE> heaps(heapCount);
    DWORD got = GetProcessHeaps(heapCount, heaps.data());
    if (got == 0) return 0;

    uint64_t totalBusyBytes = 0;

    // 2) Walk each heap and sum busy blocks
    for (DWORD i = 0; i < got; ++i)
    {
        HANDLE hHeap = heaps[i];
        if (!hHeap) continue;

        // Lock heap to stabilize enumeration
        if (!HeapLock(hHeap))
            continue;

        PROCESS_HEAP_ENTRY entry;
        entry.lpData = nullptr;

        while (HeapWalk(hHeap, &entry))
        {
            // Ignore regions/uncommitted ranges; sum only "busy" (allocated) blocks
            if ((entry.wFlags & PROCESS_HEAP_ENTRY_BUSY) != 0)
            {
                totalBusyBytes += static_cast<uint64_t>(entry.cbData);
            }

            // Optional: entry.wFlags & PROCESS_HEAP_ENTRY_MOVEABLE / DDESHARE etc.
        }

        // HeapWalk returns FALSE at end or error; check if real error:
        DWORD err = GetLastError();
        // ERROR_NO_MORE_ITEMS is expected when finished walking the heap
        (void)err;

        HeapUnlock(hHeap);
    }

    return totalBusyBytes;
}


int main() {
	Bolt::SceneDefinition& def = Bolt::SceneManager::RegisterScene("Game");
	def.AddSystem<GameSystem>();
	def.AddSystem<Bolt::ParticleUpdateSystem>();

	Bolt::Application::SetRunInBackground(true);
	Bolt::Application::SetForceSingleInstance(false);
	Bolt::Application app;

	runapp:
	app.Run();

    auto heapBytes = GetTotalAllocatedProcessHeapBytes();
    std::cout << (heapBytes / (1024 * 1024)) << "MB \n";
	std::cout << "Do you want to quit? (Y/N)\n";

	std::string input = "";
	size_t loopCount = 0;
	while (input != "y" && input != "n") {
		if (loopCount == 5) {
			std::cout << "Input entered wrong for the 5th time, last chance before terminating" << '\n';
		}
		if (loopCount > 5)
			break;

		std::cin >> input;
		std::transform(input.begin(), input.end(), input.begin(), [](unsigned char c) { return std::tolower(c); });
		++loopCount;
	}

	if (input == "n") {
		goto runapp;
	}
	std::cout << "Terminating...\n";
	return 0;
}