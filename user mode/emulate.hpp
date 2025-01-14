#pragma once
#include <cstdint>
#include <WinUser.h>
#include "variables.hpp"
#include "stdafx.hpp"
#include "spoof.hpp"
#include "sdk.hpp"
#include "execute.hpp"
#ifndef AIMBOT_H
#define AIMBOT_H

// globals
//static bool already_started = false;
//static bool reset_angles = false;


struct emulate_aimbot_input
{
	// FortWeapRanged_FireSingle 0x1160
	//void silent_aim(uintptr_t current_weapon, FVector rotation)
	//{
	//	if (!already_started)
	//	{
	//		uintptr_t writecopy_access = request->read<uintptr_t>(current_weapon + 0x1D65);
	//		{
	//			uintptr_t pointer = request->read<uintptr_t>(writecopy_access + 0x1BD0);
	//			uintptr_t pointer2 = request->read<uintptr_t>(pointer + 0x9AC);
	//			uintptr_t pointer3 = request->read<uintptr_t>(pointer2 + 0x6F);

	//			for (int32_t i = 0; i < sizeof(pointer3); i++)
	//			{
	//				request->write<BYTE>(pointer3 + i, 0xCC); // ret
	//				// fix this crash 
	//				// maybe try using Sleep(5); in between each write.
	//			}
	//		}
	//		already_started = true;
	//	}

	//	//backup angles for reset
	//	double aim_pitch = request->read<double>(current_weapon + 0x4452);
	//	double aim_yaw = request->read<double>(current_weapon + 0x445A);

	//	request->write<double>(current_weapon + 0x4452, rotation.x);
	//	request->write<double>(current_weapon + 0x445A, rotation.y);

	//	reset_angles = true;

	//	//resetting aim angles 
	//	if (reset_angles)
	//	{
	//		// reset angles
	//		request->write<double>(current_weapon + 0x4452, aim_pitch);
	//		request->write<double>(current_weapon + 0x445A, aim_yaw);

	//		// pause so it doesn't save the player's last pos
	//		std::this_thread::sleep_for(std::chrono::milliseconds(900));

	//		// save new pitch and yaw aim angles to write back too
	//		aim_pitch = request->read<double>(current_weapon + 0x4452);
	//		aim_yaw = request->read<double>(current_weapon + 0x445A);

	//		reset_angles = false;
	//	}
	//}

	void AddInput(APlayerController* PlayerController, FRotator TargetRotation, camera_ Camera)
	{
		if (PlayerController)
		{
			TargetRotation.Pitch = (TargetRotation.Pitch - Camera.Rotation.x) / double(PlayerController->InputPitchScale());
			TargetRotation.Yaw = (TargetRotation.Yaw - Camera.Rotation.y) / double(PlayerController->InputYawScale());

			request->write<float>(PlayerController->InputPitchScale(), TargetRotation.Pitch);
			request->write<float>(PlayerController->InputYawScale(), TargetRotation.Yaw);
		}
	}
}; std::unique_ptr<emulate_aimbot_input> game_emulate = std::make_unique<emulate_aimbot_input>();

struct mouse_
{
	void mouse_aim(double x, double y, int smooth)
	{
		float ScreenCenterX = center_x;
		float ScreenCenterY = center_y;
		int AimSpeed = smooth;
		float TargetX = 0;
		float TargetY = 0;

		if (x != 0)
		{
			if (x > ScreenCenterX)
			{
				TargetX = -(ScreenCenterX - x);
				TargetX /= AimSpeed;
				if (TargetX + ScreenCenterX > ScreenCenterX * 2) TargetX = 0;
			}

			if (x < ScreenCenterX)
			{
				TargetX = x - ScreenCenterX;
				TargetX /= AimSpeed;
				if (TargetX + ScreenCenterX < 0) TargetX = 0;
			}
		}

		if (y != 0)
		{
			if (y > ScreenCenterY)
			{
				TargetY = -(ScreenCenterY - y);
				TargetY /= AimSpeed;
				if (TargetY + ScreenCenterY > ScreenCenterY * 2) TargetY = 0;
			}

			if (y < ScreenCenterY)
			{
				TargetY = y - ScreenCenterY;
				TargetY /= AimSpeed;
				if (TargetY + ScreenCenterY < 0) TargetY = 0;
			}
		}

		if (aimbot->humanization)
		{
			float targetx_min = TargetX - 1;
			float targetx_max = TargetX + 1;

			float targety_min = TargetY - 1;
			float targety_max = TargetY + 1;

			float offset_x = RandomFloat(targetx_min, targetx_max);
			float offset_y = RandomFloat(targety_min, targety_max);

			free_mouse_event(MOUSEEVENTF_MOVE, static_cast<int>((float)offset_x), static_cast<int>((float)offset_y / 1), NULL, NULL);
		}
		else
		{
			free_mouse_event(MOUSEEVENTF_MOVE, TargetX, TargetY, NULL, NULL);
		}

		return;
	}
}; std::unique_ptr<mouse_> mouse = std::make_unique<mouse_>();

struct aimbot_utilites
{
	FVector target_prediction(FVector TargetPosition, FVector ComponentVelocity, float player_distance, float ProjectileSpeed)
	{
		float gravity = abs(-336); // Gravity (Probably other ways to get this but I set it as a constant value)
		float time = player_distance / abs(ProjectileSpeed);
		float bulletDrop = (gravity / 250) * time * time;
		return FVector(TargetPosition.x += time * (ComponentVelocity.x), TargetPosition.y += time * (ComponentVelocity.y), TargetPosition.z += time * (ComponentVelocity.z));
	}
}; std::unique_ptr<aimbot_utilites> aimbot_helper = std::make_unique<aimbot_utilites>();

void aimbot_thread()
{
	while (true)
	{
		for (unsigned long i = 0; i < global::Player.size(); ++i)
		{
			AFortPawn* target_player = global::Player[i];
			AFortPawn* frozen_pawn = nullptr;
			USkeletalMeshComponent* closest_mesh = target_player->Mesh();
			float aim_raduis = (aimbot->fov * center_x / camera.FieldOfView);
			static bool setup_silent_aimbot = false;
			static bool set_pawn_feeze = false;
			FVector TargetPosition3D = { };
			FVector2D TargetPosition2D = { };
			static int BindedKey = 0;

			if (aimbot->bWeapon)
				if (global::LocalPawn->CurrentWeapon()->GetAmmoCount() <= 0)
					continue;

			if (aimbot->bOnlyMale)
				if (!(BOOL)request->read<BYTE>((FortPTR)target_player + 0x5B14))
					continue;

			if (aimbot->Head)
				TargetPosition3D = closest_mesh->GetBone(106);
			else if (aimbot->Neck)
				TargetPosition3D = closest_mesh->GetBone(66);
			else if (aimbot->Chest)
				TargetPosition3D = closest_mesh->GetBone(7);
			else if (aimbot->Belly)
				TargetPosition3D = closest_mesh->GetBone(3);
			if (aimbot->prediction) {
				static auto speed = global::LocalPawn->CurrentWeapon()->GetProjectileSpeed(); // ActiveWeapon->GetChargePercent()
				static auto distance = global::GWorld->GetCameraLocation().Distance(TargetPosition3D);
				aimbot_helper->target_prediction(TargetPosition3D, global::LocalPawn->RootComponent()->ComponentVelocity(), distance, speed);
			}

			if (aimbot->waypoint && target_player->GetActorFName().contains("BuildingWeakSpot"))
				if (auto ObjectID = target_player->GetClassId((FortPTR)target_player); target_player->IsA((FortPTR)target_player, ObjectID))
					TargetPosition3D = target_player->RootComponent()->RelativeLocation();
			TargetPosition2D = global::LocalPlayer->W2S(TargetPosition3D);

			if (aimbot->RMouse)
				BindedKey = VK_RBUTTON;
			else if (aimbot->LMouse)
				BindedKey = VK_LBUTTON;
			else if (aimbot->LShift)
				BindedKey = VK_LSHIFT;
			else if (aimbot->LCtrl)
				BindedKey = VK_LCONTROL;

			double dx = TargetPosition2D.x - center_x;
			double dy = TargetPosition2D.y - center_y;
			float closest_head_distance = sqrtf(dx * dx + dy * dy);
			if (aimbot->Aimline && bIsInRectangle(center_x, center_y, aim_raduis, TargetPosition2D.x, TargetPosition2D.y))
				ImGui::GetBackgroundDrawList()->AddLine(ImVec2(center_x, center_y), ImVec2(TargetPosition2D.x, TargetPosition2D.y), global::LocalPawn->CurrentWeapon()->CurrentReticleColor(), 0.5f);

			//std::this_thread::sleep_for(std::chrono::seconds(AimbotSlow));

			if (free_GetAsyncKeyState(BindedKey)) {
				if (TargetPosition2D.valid_location()) {
					if (bIsInRectangle(center_x, center_y, aim_raduis, TargetPosition2D.x, TargetPosition2D.y)) {
						if (closest_head_distance <= aimbot->fov * 2 && target_player->player_distance(TargetPosition3D) <= aimbot->AimDistance) {
							if (GetDistance(TargetPosition2D.x, TargetPosition2D.y, center_x, center_y) <= aimbot->fov) {
								if (aimbot->bAimbot)
									mouse->mouse_aim(TargetPosition2D.x, TargetPosition2D.y, aimbot->smoothness);
								else if (aimbot->bMemoryAim) {
									if (FRotator player_rot = GetRotation(TargetPosition3D); player_rot.Yaw != 0 && player_rot.Pitch != 0)
										game_emulate->AddInput(global::PlayerController, player_rot, camera);
								}
								else if (aimbot->bSilentAimbot) {
									if (!setup_silent_aimbot) setup_silent_aimbot = hook::setup_silent((FortPTR)global::LocalPlayer, (FortPTR)global::PlayerController);
									if (setup_silent_aimbot)
										hook::update_silent((FortPTR)global::LocalPlayer, (FortPTR)global::PlayerController);
								}

								if (aimbot->Backtrack) {
									request->write<float>((FortPTR)target_player + 0x64, 0.f);
									frozen_pawn = target_player;
									set_pawn_feeze = true;
								}
							}
						}
					}
				}
			}

			if (aimbot->triggerbot)
			{
				if (closest_mesh->WasRecentlyRendered()) {
					if (TargetPosition2D.valid_location()) {
						if (bIsInRectangle(center_x, center_y, aim_raduis, TargetPosition2D.x, TargetPosition2D.y)) {
							if (closest_head_distance <= aimbot->fov * 2 && target_player->player_distance(TargetPosition3D) <= aimbot->AimDistance) {
								if (GetDistance(TargetPosition2D.x, TargetPosition2D.y, center_x, center_y) <= aimbot->fov) {
									free_mouse_event(MOUSEEVENTF_LEFTDOWN, DWORD(NULL), DWORD(NULL), DWORD(0x0002), ULONG_PTR(NULL));
									free_mouse_event(MOUSEEVENTF_LEFTUP, DWORD(NULL), DWORD(NULL), DWORD(0x0004), ULONG_PTR(NULL));
									int x = 0;
									while (x < aimbot->triggerSpeed * 10) x++;
									x = 0;
								}
							}
						}
					}
				}
			}

			if (set_pawn_feeze)
			{
				request->write<float>((uintptr_t)frozen_pawn + 0x64, 0.f);
				frozen_pawn = NULL;
				set_pawn_feeze = false;
			}
		}
	}
}
#endif // AIMBOT_H