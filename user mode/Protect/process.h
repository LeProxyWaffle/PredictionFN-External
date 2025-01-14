#pragma once
#include <Windows.h>
#include <TlHelp32.h>
#include <filesystem>
#include <vector>
#include "lazy.h"
#include "xorstr.h"
#include "skCrypt.h"

int check_processes()
{
    std::vector<const wchar_t*> processes = {
    TEXT("ollydbg.exe"),
    TEXT("ProcessHacker.exe"),
    TEXT("tcpview.exe"),
    TEXT("autoruns.exe"),
    TEXT("autorunsc.exe"),
    TEXT("filemon.exe"),
    TEXT("procmon.exe"),
    TEXT("regmon.exe"),
    TEXT("procexp.exe"),
    TEXT("ImmunityDebugger.exe"),
    TEXT("Wireshark.exe"),
    TEXT("dumpcap.exe"),
    TEXT("HookExplorer.exe"),
    TEXT("ImportREC.exe"),
    TEXT("PETools.exe"),
    TEXT("LordPE.exe"),
    TEXT("SysInspector.exe"),
    TEXT("proc_analyzer.exe"),
    TEXT("sysAnalyzer.exe"),
    TEXT("sniff_hit.exe"),
    TEXT("windbg.exe"),
    TEXT("joeboxcontrol.exe"),
    TEXT("joeboxserver.exe"),
    TEXT("ResourceHacker.exe"),
    TEXT("ida.exe"),
    TEXT("ida64.exe"),
    TEXT("ida32.exe"),
    TEXT("x32dbg.exe"),
    TEXT("x64dbg.exe"),
    TEXT("Fiddler.exe"),
    TEXT("httpdebugger.exe"),
    TEXT("HTTP Debugger Windows Service (32 bit).exe"),
    TEXT("HTTPDebuggerUI.exe"),
    TEXT("HTTPDebuggerSvc.exe"),
    TEXT("cheatengine-x86_64.exe"),
    TEXT("cheatengine-x86_64-SSE4-AVX2.exe"),
    TEXT("Scylla.exe"),
    TEXT("KsDumper11.exe"),
    TEXT("KsDumper7.exe"),
    };

    for (auto process : processes)
    {
        if (auto pid = request->get_process_pid(process); pid)
        {
            std::wstring wprocess(process);
            std::string process_name(wprocess.begin(), wprocess.end());

            system("cls");
            std::string error_bad_process = xorstr(" unallowed process or processes found, pid: ").c_str() + 
                std::to_string(pid) + 
                xorstr(", name: ").c_str() + 
                process_name;
            std::cout << error_bad_process.c_str() << std::endl;
            Sleep(5000);
            exit(1);
            *(uintptr_t*)nullptr = 0;
        }
    }

    return false;
}