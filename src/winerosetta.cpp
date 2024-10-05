#include <windows.h>
#include <cstdint>
#include <cstdio>

LONG WINAPI VectoredHandler1(struct _EXCEPTION_POINTERS* ExceptionInfo) {
    if (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_ILLEGAL_INSTRUCTION) {
        auto context = ExceptionInfo->ContextRecord;

        if (*reinterpret_cast<uint16_t*>(context->Eip) == 0xD063) {
            auto dest = reinterpret_cast<uint16_t*>(&context->Eax);
            auto src = reinterpret_cast<uint16_t*>(&context->Edx);
            if ((*dest & 3) < (*src & 3)) {
                context->EFlags |= 0x40;
                *dest = (*dest & ~3) | (*src & 3);
            } else {
                context->EFlags &= ~0x40;
            }
            context->Eip += 2;
            return EXCEPTION_CONTINUE_EXECUTION;
        }

        if (*reinterpret_cast<uint16_t*>(context->Eip) == 0xD8DC) {
            *reinterpret_cast<uint16_t*>(context->Eip) = 0xD8D8;
            return EXCEPTION_CONTINUE_EXECUTION;
        }
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

BOOL WINAPI DllMain(HMODULE mod, DWORD reason, LPVOID reserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        AddVectoredExceptionHandler(1, VectoredHandler1);
    }
    return TRUE;
}