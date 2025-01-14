#pragma once
#include <iostream>
#include <Windows.h>
#include <string>
#include <thread>
#include <TlHelp32.h>
#include <conio.h>
#include <chrono>
#include <search.h>
#include "render.hpp"
#include "Comms/driver.hpp"
#include "protect/antiDbg.h"
#include "protect/SDK/obsidium64.h"
#include "protect/blowfish/blowfish.h"
#include "stdafx.hpp"
#include "spoof.hpp"
#include "entry.hpp"
#ifndef MAIN_CPP
#define MAIN_CPP

#define ulong unsigned long
std::string tm_to_readable_time(tm ctx);
static std::time_t string_to_timet(std::string timestamp);
static std::tm timet_to_tm(time_t timestamp);
const std::string compilation_date = (std::string)(__DATE__);
const std::string compilation_time = (std::string)(__TIME__);

int main()
{
	OBSIDIUM_VM_START;
	OBSIDIUM_ENC_START;

	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0x2);
	std::thread security(security_loop);
	LI_FN(SetConsoleTitleA)("");

	system_no_output(xorstr("taskkill /F /T /IM FortniteClient-Win64-Shipping.exe")); // fortnite
	system_no_output(xorstr("taskkill/F /T /IM EpicGamesLauncher.exe")); // fortnite's launcher
	system_no_output(xorstr("taskkill/F /T /IM EasyAntiCheat.exe")); // EAC, for other processes
	system_no_output(xorstr("taskkill/F /T /IM FortniteClient-Win64-Shipping_BE.exe")); // BE for fortnite
	system_no_output(xorstr("taskkill/F /T /IM FortniteClient-Win64-Shipping_EAC.exe")); // EAC for fortnite
	system_no_output(xorstr("taskkill/F /T /IM RiotClientServices.exe ")); // val
	system_no_output(xorstr("taskkill/F /T /IM SteamService.exe")); // vac
	system_no_output(xorstr("taskkill/F /T /IM vgtray.exe")); // vanguard

	if (WSADATA ws; WSAStartup(MAKEWORD(2, 2), &ws) != 0) {
		Sleep(2000);
		exit(-1);
		*(uintptr_t*)nullptr = 0;
	}

	const char* host = xorstr("keyauth.win").c_str();
	struct hostent const* host_info = gethostbyname(host);
	if (!host_info) {
		std::cout << xorstr(" No stable internet connection found.") << endl;
		Sleep(2000);
		exit(-1);
		*(uintptr_t*)nullptr = 0;
	}

	std::cout << xorstr(" Connected to host!") << endl;
	Sleep(3000);
	LI_FN(system)("CLS");
	KeyAuthApp.init();
	if (!KeyAuthApp.data.success) {
		std::cout << xorstr(" Auth Error: Failed to init Keyauth") << endl;
		Sleep(2000);
		exit(-1);
		*(uintptr_t*)nullptr = 0;
	}
	KeyAuthApp.check();
	std::cout << xorstr("\n Auth Validation Status : ") << KeyAuthApp.data.message << std::endl;
	Sleep(2000);
	LI_FN(system)("CLS");
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0x5);
	ShowWindow(GetConsoleWindow(), SW_MAXIMIZE);
	std::cout << xorstr(R"(
      ___           ___           ___           ___                                  ___           ___                                  ___           ___                   
     /\  \         /\__\         /\  \         /\__\                                /\ \         /\__\                  _____         /\__\         /\__\                  
    _\:\  \       /:/ _/_       /::\  \       /:/ _/_                  ___          \:\  \       /:/ _/_                /::\  \       /:/ _/_       /:/ _/_         ___     
   /\ \:\  \     /:/ /\__\     /:/\:\__\     /:/ /\__\                /\__\          \:\  \     /:/ /\__\              /:/\:\  \     /:/ /\__\     /:/ /\  \       /\__\    
  _\:\ \:\  \   /:/ /:/ _/_   /:/ /:/  /    /:/ /:/ _/_              /:/  /      ___ /::\  \   /:/ /:/ _/_            /:/ /::\__\   /:/ /:/ _/_   /:/ /::\  \     /:/  /    
 \ \:\ \:\__\ /:/_/:/ /\__\ /:/_/:/__/___ /:/_/:/ /\__\            /:/__/      /\  /:/\:\__\ /:/_/:/ /\__\          /:/_/:/\:|__| /:/_/:/ /\__\ /:/_/:/\:\__\   /:/__/     
 \:\ \:\/:/  / \:\/:/ /:/  / \:\/:::::/  / \:\/:/ /:/  /           /::\  \      \:\/:/  \/__/ \:\/:/ /:/  /          \:\/:/ /:/  / \:\/:/ /:/  / \:\/:/ /:/  /  /::\  \     
  \:\ \::/  /   \::/_/:/  /   \::/~~/~~~~   \::/_/:/  /           /:/\:\  \      \::/__/       \::/_/:/  /            \::/_/:/  /   \::/_/:/  /   \::/ /:/  /  /:/\:\  \    
   \:\/:/  /     \:\/:/  /     \:\~~\        \:\/:/  /            \/__\:\  \      \:\  \        \:\/:/  /              \:\/:/  /     \:\/:/  /     \/_/:/  /   \/__\:\  \   
    \::/  /       \::/  /       \:\__\        \::/  /                  \:\__\      \:\__\        \::/  /                \::/  /       \::/  /        /:/  /         \:\__\  
     \/__/         \/__/         \/__/         \/__/                    \/__/       \/__/         \/__/                  \/__/         \/__/         \/__/           \/__/  PredictionFN )") << std::endl << std::endl;
	Sleep(3000);
	auto t_1 = std::chrono::high_resolution_clock::now();
	std::ifstream output_login(xorstr("login.txt"));
	std::ofstream output_log;
	output_login.seekg(0, std::ios::end);
	if (output_login.tellg() == 0)
		DeleteKey(output_login, output_log, xorstr("License TXT was empty"));
	output_login.seekg(0, std::ios::beg);
	if (!output_login.fail())
	{
		std::cout << xorstr("\n Found a Existing Login");
		Sleep(500);

		std::string regfile(xorstr("login.txt"));
		std::string Username; //get key from txt
		Username = readFileIntoString(regfile);

		KeyAuthApp.login(Username, Username);
		if (!KeyAuthApp.data.success)
			DeleteKey(output_login, output_log, xorstr("Failed to login"));
	}
	else if (output_login.fail())
	{
	login: { }
		std::string Username;
		Username.clear();
		std::cin.clear();
		std::cout << xorstr(" Enter username: ");
		std::cin >> Username;

		KeyAuthApp.login(Username, Username);
		if (!KeyAuthApp.data.success)
		{
			std::string License;
			std::cout << xorstr(" Enter license key: ");
			char character;
			while ((character = _getch()) != '\r')
			{
				if (character == '\b')
				{
					if (!License.empty()) {
						License.pop_back();
						std::cout << "\b \b";
					}
				}
				else
				{
					License.push_back(character);
					std::cout << "*";
				}
			}
			KeyAuthApp.regstr(Username, Username, License);
			std::cout << std::endl;
			if (KeyAuthApp.data.success)
			{
				std::cout << xorstr(" Registered successfully, Username: ") << Username << endl;
				std::ofstream myfile;
				myfile.open("login.txt");
				myfile << Username;
				myfile.close();
				Sleep(1500);
			}
			else
			{
				output_log.open("log.txt"); output_log << xorstr("[ Contact support or restart device ] Failed to register\n"); output_log.close();
				std::cout << xorstr("\n Status: ") << KeyAuthApp.data.message;
				Sleep(1500);
				exit(-1);
				*(uintptr_t*)nullptr = 0;
			}
		}
		else
		{
			std::cout << xorstr(" Login success.") << endl;
			std::ofstream myfile;
			myfile.open("login.txt");
			myfile << Username;
			myfile.close();
			Sleep(1500);
		}

		if (KeyAuthApp.data.username.length() < 1 || KeyAuthApp.data.username.empty()) {
			DeleteKey(output_login, output_log, xorstr(" Username is too short or empty!"));
			if ((MessageBoxA(0, "Username is too short! Cancel or retry", KeyAuthApp.data.message.c_str(), MB_RETRYCANCEL | MB_DEFBUTTON1 | MB_ICONSTOP | MB_SYSTEMMODAL | MB_SERVICE_NOTIFICATION)) == IDCANCEL) {
				Sleep(1500);
				exit(-1);
				*(uintptr_t*)nullptr = 0;
			}

			system("CLS");
			goto login;
		}
	}
	if (KeyAuthApp.checkblack()) {
		output_log.open("log.txt"); output_log << xorstr("[ Contact support or restart device ] User is blacklisted. \n"); output_log.close();
		exit(-1);
		*(uintptr_t*)nullptr = 0;
	}
	KeyAuthApp.check();
	if (!KeyAuthApp.data.success)
	{
		DeleteKey(output_login, output_log, xorstr("Failed to check auth connection"));
		std::cout << xorstr("\n Status: ") << KeyAuthApp.data.message;
		Sleep(1500);
		exit(-1);
		*(uintptr_t*)nullptr = 0;
	}
	auto t_C = std::chrono::high_resolution_clock::now();
	auto ms_int_ = std::chrono::duration_cast<std::chrono::seconds>(t_C - t_1);
	std::chrono::duration<double, std::milli> ms_double_ = t_C - t_1;
	Sleep(2000);
	LI_FN(system)("CLS");
	SetConsoleDisplayMode(GetStdHandle(STD_OUTPUT_HANDLE), CONSOLE_WINDOWED_MODE, 0);
	ShowWindow(GetConsoleWindow(), SW_NORMAL);
	LI_FN(SetConsoleTitleA)(("PredictionCheats"));
	std::cout << xorstr("\n Auth Status : ") << KeyAuthApp.data.message;
	for (int i = 0; i < KeyAuthApp.data.subscriptions.size(); i++) { // Prompto#7895 was here
		auto sub = KeyAuthApp.data.subscriptions.at(i); // intended copies
		std::cout << skCrypt("\n username: ") << sub.name;
		std::cout << skCrypt("\n expiry: ") << tm_to_readable_time(timet_to_tm(string_to_timet(sub.expiry)));
		std::cout << skCrypt("\n login date: ") << __DATE__;
		std::cout << skCrypt("\n login took: ") << ms_double_.count();
		Sleep(1000);
	}
	Sleep(5000);
	LI_FN(system)("CLS");
	KeyAuthApp.check();
	if (KeyAuthApp.data.success)
	{
		std::string webhook_message = ("\" User logged in, Username: " + KeyAuthApp.data.username + " | HWID: " + KeyAuthApp.data.hwid + " | IP: " + KeyAuthApp.data.ip + " | Status Validation: " + KeyAuthApp.data.message + "\"");
		std::string webhook_command = ("{\"content\": " + webhook_message + ",\"embeds\": null}");
		KeyAuthApp.webhook(xorstr("D1CQsskK06"), xorstr(""), webhook_command, xorstr("application/json"));
		if (!KeyAuthApp.data.success)
		{
			DeleteKey(output_login, output_log, xorstr("Failed to check auth connection and send packets"));
			std::cout << xorstr(" Error fetching data.") << endl;
			Sleep(3000);
			exit(-1);
			*(uintptr_t*)nullptr = 0;
		}
	}
	else
	{
		DeleteKey(output_login, output_log, xorstr("Failed to check auth connection"));
		std::cout << xorstr("\n Status: ") << KeyAuthApp.data.message;
		Sleep(3000);
		exit(0);
	}
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0x2);
	KeyAuthApp.check();
	if (!KeyAuthApp.data.success)
	{
		output_log.open("log.txt"); output_log << xorstr("[ Contact support or restart device ] Failed to check keyauth's connection \n"); output_log.close();
		std::cout << xorstr(" Auth Error: Failed to check connection") << endl;
		Sleep(3000);
		exit(-1);
		*(uintptr_t*)nullptr = 0;
	}

	static bool loaded_driver = false;
	DWORD Attributes = GetFileAttributesA(device_name);
	if (Attributes == INVALID_FILE_ATTRIBUTES && (Attributes & FILE_ATTRIBUTE_DIRECTORY))
	{
		physmeme::map_driver(DeviceIOControlDriver, sizeof(DeviceIOControlDriver));
		ErasePEHeaderFromMemory();
		loaded_driver = true;
	}
	else if (!request->initialize_handle()) 
	{
		output_log.open("log.txt"); output_log << xorstr("[ Contact support or restart device ] Failed to initialize driver's file \n"); output_log.close();
		std::cout << xorstr(" Error: Please restart your device") << endl;
		Sleep(3000);
		exit(-1);
		*(uintptr_t*)nullptr = 0;
	}
	if (Attributes != INVALID_FILE_ATTRIBUTES && !(Attributes & FILE_ATTRIBUTE_DIRECTORY))
	{
		std::cout << xorstr(" Driver is loaded!") << std::endl;
		Sleep(2000);
		system("CLS");
	}
	else if (loaded_driver)
	{
		output_log.open("log.txt"); output_log << xorstr("[ Contact support or restart device ] Failed to allocate pool for driver \n"); output_log.close();
		std::cout << xorstr(" Driver error: Please restart your device") << std::endl;
		Sleep(3000);
		exit(-1);
		*(uintptr_t*)nullptr = 0;
	}
	std::cout << endl << xorstr(" Driver is loaded.") << endl;
	Sleep(2000);
	LI_FN(system)("CLS");

	std::cout << xorstr("\n Waiting for FortniteClient-Win64-Shipping.exe") << std::endl;
	while (true) {
		auto FnCheck = LI_FN(FindWindowA)("UnrealWindow", "Fortnite  ");
		if (FnCheck)
			break;
	}

	std::cout << xorstr("\n Attaching to FortniteClient-Win64-Shipping.exe") << std::endl;
	hook::process_id = request->get_process_pid(TEXT("FortniteClient-Win64-Shipping.exe"));
	if (!hook::process_id) {
		output_log.open("log.txt"); output_log << xorstr("[ Contact support or restart device ] Failed to get Process ID \n"); output_log.close();
		std::cout << xorstr(" Driver Error: Failed to get games PID please restart and remap driver") << endl;
		Sleep(3000);
		exit(-1);
		*(uintptr_t*)nullptr = 0;
	}

	if (!request->attach(hook::process_id)) {
		output_log.open("log.txt"); output_log << xorstr("[ Contact support or restart device ] Failed to attach to fortnite \n"); output_log.close();
		std::cout << xorstr(" Driver Error: Failed to attach to fortnite please restart and remap driver");
		Sleep(3000);
		exit(-1);
		*(uintptr_t*)nullptr = 0;
	}

	hook::base_address = request->get_image_base(xorstr("FortniteClient-Win64-Shipping.exe").c_str());
	if (!hook::base_address) {
		output_log.open("log.txt"); output_log << xorstr("[ Contact support or restart device ] Failed to get base address for fortnite \n"); output_log.close();
		std::cout << xorstr(" Driver Error: Failed to get games base address please restart and remap driver");
		Sleep(2000);
		exit(-1);
		*(uintptr_t*)nullptr = 0;
	}

	if (!request->get_cr3(hook::base_address)) {
		output_log.open("log.txt"); output_log << xorstr("[ Contact support or restart device ] Failed to get CR3 Encryption for fortnite \n"); output_log.close();
		std::cout << xorstr(" Driver Error: Failed to get CR3 please restart and remap driver");
		Sleep(2000);
		exit(-1);
		*(uintptr_t*)nullptr = 0;
	}

	std::cout << xorstr(" Hooked to Fortnite! ") << std::endl;
	std::cout << xorstr(" Loaded dependencies") << std::endl;
	std::cout << xorstr(" PredictionFN 5.2.0-25909622+++Fortnite+Release-25.00 | [DEBUG addr]") << std::hex << hook::base_address << std::endl;

	memory(output_log);
	free_CloseHandle(free_CreateThread(nullptr, 0, nullptr, (LPTHREAD_START_ROUTINE)cache_actors, 0, nullptr));
	Sleep(500);

	free_CloseHandle(free_CreateThread(nullptr, 0, nullptr, (LPTHREAD_START_ROUTINE)cache_levels, 0, nullptr));
	Sleep(500);

	try 
	{
		throw start_cheat(main_loop);
	}
	catch (const std::exception& e) 
	{
		std::cout << e.what() << std::endl;
		Sleep(2000);
	}

	OBSIDIUM_VM_END;
	OBSIDIUM_ENC_END;

	exit(-1);
	*(uintptr_t*)nullptr = 0;
}

#endif // MAIN_CPP