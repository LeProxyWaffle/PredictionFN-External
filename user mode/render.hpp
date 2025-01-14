#pragma once
#include <dwmapi.h>
#include <stdio.h>
#include <string>
#include "hexray.hpp"
#include "sdk.hpp"
#include "math.h"
#include "emulate.hpp"
#include "overlay.hpp"
#include "stdafx.hpp"
#include "spoof.hpp"
#include "menu.hpp"
#ifndef RENDER_HPP
#define RENDER_HPP
using namespace std;

void Range(double* x, double* y, float range)
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

FVector2D RotatePoint(FVector2D radar_pos, FVector2D radar_size, FVector LocalLocation, FVector TargetLocation) {
	auto dx = TargetLocation.x - LocalLocation.x;
	auto dy = TargetLocation.y - LocalLocation.y;

	auto x = dy * -1;
	x *= -1;
	auto y = dx * -1;

	double calcualted_range = 34 * 1000;

	Range(&x, &y, calcualted_range);

	int rad_x = (int)radar_pos.x;
	int rad_y = (int)radar_pos.y;

	double r_siz_x = radar_size.x;
	double r_siz_y = radar_size.y;

	int x_max = (int)r_siz_x + rad_x - 5;
	int y_max = (int)r_siz_y + rad_y - 5;

	auto return_value = FVector2D();

	return_value.x = rad_x + ((int)r_siz_x / 2 + int(x / calcualted_range * r_siz_x));
	return_value.y = rad_y + ((int)r_siz_y / 2 + int(y / calcualted_range * r_siz_y));

	if (return_value.x > x_max)
		return_value.x = x_max;

	if (return_value.x < rad_x)
		return_value.x = rad_x;

	if (return_value.y > y_max)
		return_value.y = y_max;

	if (return_value.y < rad_y)
		return_value.y = rad_y;

	return return_value;
}

void AddToRadar(FVector WorldLocation, FVector LocalLocation, ImColor Color, FVector2D RadarPos, FVector2D RadarSize)
{
	FVector2D Projected = RotatePoint(RadarPos, RadarSize, LocalLocation, WorldLocation);
	ImGui::GetBackgroundDrawList()->AddCircleFilled(ImVec2(Projected.x, Projected.y), 3, Color, 32);
}

void playerpawn_loop()
{
	do {
		std::unique_lock<std::mutex> lock(global::data_mutex);
		if (aimbot->showfov)
			ImGui::GetBackgroundDrawList()->AddCircle(ImVec2(center_x, center_y), (aimbot->fov * center_x / camera.FieldOfView), ImColor(0, 0, 0, 255), 2000, 0.9f);

		for (unsigned long i = 0; i < global::Player.size(); ++i)
		{
			AFortPawn* entity = global::Player[i];
			USkeletalMeshComponent* mesh = entity->Mesh();
			int MyTeamId = global::LocalPawn->PlayerState()->TeamIndex();
			int ActorTeamId = entity->PlayerState()->TeamIndex();
			bool IsVisible = mesh->WasRecentlyRendered();

			if (misc->bIgnoreTeam) {
				AFortAthenaVehicle* Vehicle = request->read<AFortAthenaVehicle*>((FortPTR)entity + offsets::CurrentVehicle);
				if (AFortAthenaVehicle* CurrentVehicle = global::LocalPawn->CurrentVehicle(); hook::is_valid((FortPTR)Vehicle) && hook::is_valid((FortPTR)CurrentVehicle))
					if (CurrentVehicle->CurrentTeam(Vehicle))
						continue;
				if (MyTeamId == ActorTeamId)
					continue;
			}

			if (misc->bIgnoreBots) {
				bool IsBot = request->read<BYTE>((uintptr_t)entity + 0x292);
				if (!IsBot)
					continue;
			}

			if (misc->bIgnoreDead) {
				bool bIsDying = (request->read<BYTE>((uintptr_t)entity + 0x750) & 0x10); // 0x710
				bool bisDBNO = (request->read<BYTE>((uintptr_t)entity + 0x872) & 0x10); // 0x832
				if (bisDBNO || bIsDying)
					continue;
			}

			if (misc->bIgnoreAFK) {
				bool bIsDisconnectedPawn = (request->read<BYTE>((uintptr_t)entity + 0x11A8));
				if (!bIsDisconnectedPawn)
					continue;
			}

			if (visual->draw_radar)
			{
				float HUDScale = 0.f;

				if (UFortClientSettingsRecord* ClientSettingsRecord = global::LocalPlayer->ClientSettingsRecord())
				{
					HUDScale = ClientSettingsRecord->HUDScale();
				}

				double RadarSize = 15.625 * center_x * double(HUDScale) / 100.0;
				RadarSize *= 2;

				double RadarPositionOffset = RadarSize / 30.0;
				auto RadarPosition = FVector2D(center_y - RadarSize - RadarPositionOffset, RadarPositionOffset);

				if (HUDScale != 0.f) {
					AddToRadar(mesh->GetBone(0),
						camera.Location,
						ImColor(255, 255, 255),
						RadarPosition,
						FVector2D(RadarSize, RadarSize)
					);
				}
			}

			if (misc->bIgnoreHidden) {
				__int64 CurrentBuild = 0;
				bool bTargetedBuild = true;
				__int64 TargetedBuild = global::PlayerController->TargetedBuild();
				if (bTargetedBuild) {
					CurrentBuild = TargetedBuild;
					bTargetedBuild = false;
				}
				else {
					CurrentBuild = 0;
					bTargetedBuild = true;
				}

				if (CurrentBuild != 0 && CurrentBuild != TargetedBuild) {
					bTargetedBuild = false;
					continue;
				}
			}

			FVector head3d, root3d; double player_distance;
			FVector2D head2d = global::LocalPlayer->W2S(head3d = mesh->GetBone(106));
			FVector2D root2d = global::LocalPlayer->W2S(root3d = mesh->GetBone(88));
			if (player_distance = entity->player_distance(root3d) <= visual->MaxDistance && IsInScreen(root2d)) continue;
			FVector2D chest2d, pelvis2d, rshoulder2d, relbow2d, rhand2d, rknee2d, rfoot2d, lshoulder2d, lelbow2d, lhand2d, lknee2d, lfoot2d;
			chest2d = global::LocalPlayer->W2S(mesh->GetBone(7)); pelvis2d = global::LocalPlayer->W2S(mesh->GetBone(2));
			rshoulder2d = global::LocalPlayer->W2S(mesh->GetBone(35));
			relbow2d = global::LocalPlayer->W2S(mesh->GetBone(10)); rhand2d = global::LocalPlayer->W2S(mesh->GetBone(29));
			rknee2d = global::LocalPlayer->W2S(mesh->GetBone(72)); rfoot2d = global::LocalPlayer->W2S(mesh->GetBone(76));
			lshoulder2d = global::LocalPlayer->W2S(mesh->GetBone(64));
			lelbow2d = global::LocalPlayer->W2S(mesh->GetBone(65)); lhand2d = global::LocalPlayer->W2S(mesh->GetBone(62));
			lknee2d = global::LocalPlayer->W2S(mesh->GetBone(79)); lfoot2d = global::LocalPlayer->W2S(mesh->GetBone(83));
			FVector2D bones_to_check[] = { head2d, chest2d, pelvis2d, rhand2d, relbow2d, rfoot2d, lhand2d, lelbow2d, lfoot2d, root2d };

			auto most_left = FLT_MAX;
			auto most_right = FLT_MIN;
			auto most_top = FLT_MAX;
			auto most_bottom = FLT_MIN;
			for (int i = 0; i < sizeof(bones_to_check); i++) {
				auto bone = bones_to_check[i];

				if (bone.x < most_left)
					most_left = bone.x;

				if (bone.x > most_right)
					most_right = bone.x;

				if (bone.y < most_top)
					most_top = bone.y;

				if (bone.y > most_bottom)
					most_bottom = bone.y;
			}

			auto actor_height = most_bottom - most_top;
			auto actor_width = most_right - most_left;

			auto calculated_distance = 225 - player_distance;
			auto offset = calculated_distance * 0.025;
			auto corner_wide = actor_width / 3;
			auto corner_heit = actor_height / 3;

			auto text_offset = double(0);
			FVector2D top_middle = FVector2D(most_left + actor_width / 2, head2d.y);
			FVector2D bottom_middle = FVector2D(most_left + actor_width / 2, root2d.y);

			auto ColorBox = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
			auto ColorSkeleton = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
			auto ColorSnapline = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

			if (MyTeamId != ActorTeamId)
				ColorBox = IsVisible ? ImVec4(visual->color->PlayerboxVisible[0], visual->color->PlayerboxVisible[1], visual->color->PlayerboxVisible[2], 1.0f) : ImVec4(visual->color->PlayerboxNotVisible[0], visual->color->PlayerboxNotVisible[1], visual->color->PlayerboxNotVisible[2], 1.0f);
			else
				ColorBox = IsVisible ? ImVec4(visual->color->TeamboxVisible[0], visual->color->TeamboxVisible[1], visual->color->TeamboxVisible[2], 1.0f) : ImVec4(visual->color->TeamboxNotVisible[0], visual->color->TeamboxNotVisible[1], visual->color->TeamboxNotVisible[2], 1.0f);

			ColorSkeleton = IsVisible ? ImVec4(visual->color->SkeletonVisible[0], visual->color->SkeletonVisible[1], visual->color->SkeletonVisible[2], 1.0f) : ImVec4(visual->color->SkeletonNotVisible[0], visual->color->SkeletonNotVisible[1], visual->color->SkeletonNotVisible[2], 1.0f);
			ColorSnapline = IsVisible ? ImVec4(visual->color->SnaplinesVisible[0], visual->color->SnaplinesVisible[1], visual->color->SnaplinesVisible[2], 1.0f) : ImVec4(visual->color->SnaplinesNotVisible[0], visual->color->SnaplinesNotVisible[1], visual->color->SnaplinesNotVisible[2], 1.0f);

			if (visual->box)
			{
				box_esp(offset, most_top, most_bottom, 
					most_right, most_left, 
					ImGui::GetColorU32(ColorBox), 1.f, 
					visual->fill, visual->outline);
			}
			else if (visual->cornered_box)
			{
				corner_esp(offset, most_top, most_bottom,
					most_right, most_left,
					corner_wide, corner_heit,
					ImGui::GetColorU32(ColorBox), 1.f,
					visual->outline);
			}
			else if (visual->threed)
			{
				threed_box(30.0f, 
					root2d.x, root2d.y,
					head2d.x, head2d.y,
					ImGui::GetColorU32(ColorBox), 
					0.1f);
			}


			if (visual->distance)
			{
				std::string text_distance = "(" + std::to_string(player_distance) + "m)";
				ImVec2 text_size = ImGui::CalcTextSize(text_distance.c_str());
				_DrawText(ESPFont, ImVec2(top_middle.x - (text_size.x / 2.0), top_middle.y + offset - 20.0), ImColor(255, 255, 255), text_distance.c_str(), visual->size, false);
			}

			if (visual->line)
				ImGui::GetBackgroundDrawList()->AddLine(ImVec2(center_x, Height), ImVec2(root2d.x, root2d.y), ImGui::GetColorU32(ColorSnapline), 0.5f);

			if (visual->WeaponESP)
			{
				std::string WeaponName = entity->CurrentWeapon()->GetWeaponName();
				std::string text_weapon = entity->CurrentWeapon()->IsReloadingWeapon() ? ("reloading") : (WeaponName + " (" + std::to_string(entity->CurrentWeapon()->GetAmmoCount()) + ")");
				ImVec2 Textsize = ImGui::CalcTextSize(text_weapon.c_str());
				_DrawText(ESPFont, ImVec2(bottom_middle.x - (Textsize.x / 2.0), top_middle.y + offset + 10.0), ImColor(255, 255, 255), text_weapon.c_str(), visual->size, false);
			}

			if (visual->name)
			{
				std::string name_buff = entity->PlayerState()->GetPlayerName();
				if (!name_buff.empty() && name_buff.length() < FLT_MAX) {
					ImVec2 text_size = ImGui::CalcTextSize(name_buff.c_str());
					_DrawText(ESPFont, ImVec2(bottom_middle.x - (text_size.x / 2.0), top_middle.y + offset + 10.0), ImColor(255, 255, 255), name_buff.c_str(), visual->size, false);
				}
			}

			if (visual->console)
			{
				std::string str_platform = entity->PlayerState()->GetPlatformName();
				ImVec2 TextSize = ImGui::CalcTextSize(str_platform.c_str());

				if (!str_platform.empty() && strstr(str_platform.c_str(), ("WIN")))
					_DrawText(ESPFont, ImVec2((bottom_middle.x) - (TextSize.x / 2.0), (top_middle.y + offset - 20.0)), ImColor(255, 255, 255), "Windows");
				else if (!str_platform.empty() && strstr(str_platform.c_str(), ("XBL")) || strstr(str_platform.c_str(), ("XSX")))
					_DrawText(ESPFont, ImVec2((bottom_middle.x) - (TextSize.x / 2.0), (top_middle.y + offset - 20.0)), ImColor(255, 255, 255), "Xbox");
				else if (!str_platform.empty() && strstr(str_platform.c_str(), ("PSN")) || strstr(str_platform.c_str(), ("PS5")))
					_DrawText(ESPFont, ImVec2((bottom_middle.x) - (TextSize.x / 2.0), (top_middle.y + offset - 20.0)), ImColor(255, 255, 255), "PlayStation");
				else if (!str_platform.empty() && strstr(str_platform.c_str(), ("SWT")))
					_DrawText(ESPFont, ImVec2((bottom_middle.x) - (TextSize.x / 2.0), (top_middle.y + offset - 20.0)), ImColor(255, 255, 255), "Nintendo");
				else if (!str_platform.empty() && strstr(str_platform.c_str(), ("MAC")))
					_DrawText(ESPFont, ImVec2((bottom_middle.x) - (TextSize.x / 2.0), (top_middle.y + offset - 20.0)), ImColor(255, 255, 255), "MacOS");
				else if (!str_platform.empty() && strstr(str_platform.c_str(), ("LNX")))
					_DrawText(ESPFont, ImVec2((bottom_middle.x) - (TextSize.x / 2.0), (top_middle.y + offset - 20.0)), ImColor(255, 255, 255), "Linux");
				else if (!str_platform.empty() && strstr(str_platform.c_str(), ("IOS")))
					_DrawText(ESPFont, ImVec2((bottom_middle.x) - (TextSize.x / 2.0), (top_middle.y + offset - 20.0)), ImColor(255, 255, 255), "IOS");
				else if (!str_platform.empty() && strstr(str_platform.c_str(), ("AND")))
					_DrawText(ESPFont, ImVec2((bottom_middle.x) - (TextSize.x / 2.0), (top_middle.y + offset - 20.0)), ImColor(255, 255, 255), "Android"); 
			}

			if (visual->b2Dhead)
				ImGui::GetBackgroundDrawList()->AddCircle(ImVec2(head2d.x, head2d.y), (float)(entity->player_distance(head3d) - 10.0), ImColor(255, 255, 255), 0.5f);

			if (visual->bViewAngles)
			{
				FRotator player_rotation; FVector vplayer_rotation;
				FVector player_angle = entity->RootComponent()->RelativeRotation();
				player_rotation = GetRotation(player_angle);
				vplayer_rotation.x = player_rotation.Pitch *= 360;
				vplayer_rotation.y = player_rotation.Yaw *= 360;
				vplayer_rotation.z = player_rotation.Roll *= 360;
				FVector2D head_rotation = global::LocalPlayer->W2S(head3d + vplayer_rotation);
				ImGui::GetBackgroundDrawList()->AddLine(ImVec2(head2d.x, head2d.y), ImVec2(head_rotation.x, head_rotation.y), ImColor(255, 0, 255), 1.f);
			}

			if (visual->gayskeleton)
			{
				FVector2D neck, leftwrist, rightwrist, pelvis, leftthigh, lefthip, leftjoint, righthip, rightthigh, rightjoint;
				neck = global::LocalPlayer->W2S(mesh->GetBone(67));
				leftwrist = global::LocalPlayer->W2S(mesh->GetBone(62));
				rightwrist = global::LocalPlayer->W2S(mesh->GetBone(33));
				pelvis = global::LocalPlayer->W2S(mesh->GetBone(2));
				lefthip = global::LocalPlayer->W2S(mesh->GetBone(78));
				leftthigh = global::LocalPlayer->W2S(mesh->GetBone(84));
				leftjoint = global::LocalPlayer->W2S(mesh->GetBone(80));
				righthip = global::LocalPlayer->W2S(mesh->GetBone(71));
				rightthigh = global::LocalPlayer->W2S(mesh->GetBone(77));
				rightjoint = global::LocalPlayer->W2S(mesh->GetBone(73));

				ImGui::GetBackgroundDrawList()->AddLine(ImVec2(head2d.x, head2d.y), ImVec2(neck.x, neck.y), ImGui::GetColorU32(ColorSkeleton), 2.0f);
				ImGui::GetBackgroundDrawList()->AddLine(ImVec2(neck.x, neck.y), ImVec2(chest2d.x, chest2d.y), ImGui::GetColorU32(ColorSkeleton), 2.0f);
				ImGui::GetBackgroundDrawList()->AddLine(ImVec2(chest2d.x, chest2d.y), ImVec2(lshoulder2d.x, lshoulder2d.y), ImGui::GetColorU32(ColorSkeleton), 2.0f);
				ImGui::GetBackgroundDrawList()->AddLine(ImVec2(lshoulder2d.x, lshoulder2d.y), ImVec2(lelbow2d.x, lelbow2d.y), ImGui::GetColorU32(ColorSkeleton), 2.0f);
				ImGui::GetBackgroundDrawList()->AddLine(ImVec2(lelbow2d.x, lelbow2d.y), ImVec2(leftwrist.x, leftwrist.y), ImGui::GetColorU32(ColorSkeleton), 2.0f);
				ImGui::GetBackgroundDrawList()->AddLine(ImVec2(leftwrist.x, leftwrist.y), ImVec2(lhand2d.x, lhand2d.y), ImGui::GetColorU32(ColorSkeleton), 2.0f);
				ImGui::GetBackgroundDrawList()->AddLine(ImVec2(chest2d.x, chest2d.y), ImVec2(rshoulder2d.x, rshoulder2d.y), ImGui::GetColorU32(ColorSkeleton), 2.0f);
				ImGui::GetBackgroundDrawList()->AddLine(ImVec2(rshoulder2d.x, rshoulder2d.y), ImVec2(relbow2d.x, relbow2d.y), ImGui::GetColorU32(ColorSkeleton), 2.0f);
				ImGui::GetBackgroundDrawList()->AddLine(ImVec2(relbow2d.x, relbow2d.y), ImVec2(rightwrist.x, rightwrist.y), ImGui::GetColorU32(ColorSkeleton), 2.0f);
				ImGui::GetBackgroundDrawList()->AddLine(ImVec2(rightwrist.x, rightwrist.y), ImVec2(rhand2d.x, rhand2d.y), ImGui::GetColorU32(ColorSkeleton), 2.0f);
				ImGui::GetBackgroundDrawList()->AddLine(ImVec2(chest2d.x, chest2d.y), ImVec2(pelvis.x, pelvis.y), ImGui::GetColorU32(ColorSkeleton), 2.0f);
				ImGui::GetBackgroundDrawList()->AddLine(ImVec2(pelvis.x, pelvis.y), ImVec2(lefthip.x, lefthip.y), ImGui::GetColorU32(ColorSkeleton), 2.0f);
				ImGui::GetBackgroundDrawList()->AddLine(ImVec2(lefthip.x, lefthip.y), ImVec2(leftthigh.x, leftthigh.y), ImGui::GetColorU32(ColorSkeleton), 2.0f);
				ImGui::GetBackgroundDrawList()->AddLine(ImVec2(leftthigh.x, leftthigh.y), ImVec2(lknee2d.x, lknee2d.y), ImGui::GetColorU32(ColorSkeleton), 2.0f);
				ImGui::GetBackgroundDrawList()->AddLine(ImVec2(lknee2d.x, lknee2d.y), ImVec2(leftjoint.x, leftjoint.y), ImGui::GetColorU32(ColorSkeleton), 2.0f);
				ImGui::GetBackgroundDrawList()->AddLine(ImVec2(leftjoint.x, leftjoint.y), ImVec2(lfoot2d.x, lfoot2d.y), ImGui::GetColorU32(ColorSkeleton), 2.0f);
				ImGui::GetBackgroundDrawList()->AddLine(ImVec2(pelvis.x, pelvis.y), ImVec2(righthip.x, righthip.y), ImGui::GetColorU32(ColorSkeleton), 2.0f);
				ImGui::GetBackgroundDrawList()->AddLine(ImVec2(righthip.x, righthip.y), ImVec2(rightthigh.x, rightthigh.y), ImGui::GetColorU32(ColorSkeleton), 2.0f);
				ImGui::GetBackgroundDrawList()->AddLine(ImVec2(rightthigh.x, rightthigh.y), ImVec2(rknee2d.x, rknee2d.y), ImGui::GetColorU32(ColorSkeleton), 2.0f);
				ImGui::GetBackgroundDrawList()->AddLine(ImVec2(rknee2d.x, rknee2d.y), ImVec2(rightjoint.x, rightjoint.y), ImGui::GetColorU32(ColorSkeleton), 2.0f);
				ImGui::GetBackgroundDrawList()->AddLine(ImVec2(rightjoint.x, rightjoint.y), ImVec2(rfoot2d.x, rfoot2d.y), ImGui::GetColorU32(ColorSkeleton), 2.0f);
			}

			if (visual->PlayerDebug)
			{
				if (!PlayerData->IsValid)
				{
					entity->CurrentWeapon()->RetriveWeaponData();
					entity->PlayerState()->RetrivePlayerData();
					entity->RetrivePlayerData();
				}

				bool bIsBot = request->read<BYTE>((uintptr_t)entity + 0x292);
				if (bIsBot || !IsVisible || MyTeamId != ActorTeamId)
					PlayerData->IsValid = false;

				if (PlayerData->IsValid)
				{
					static int pushback = 10;
					std::string text_distance = "PlayerID (" + std::to_string(PlayerData->PlayerId) + ")";
					ImVec2 text_size = ImGui::CalcTextSize(text_distance.c_str());
					_DrawText(ESPFont, ImVec2(bottom_middle.x - (text_size.x / 2), most_bottom + offset + 10), ImColor(255, 255, 255), text_distance.c_str(), visual->size - 5, false);

					if (PlayerData->bIsEquippingWeapon) {
						pushback += 10;
						std::string text_distance = "Equiqqing Weapon";
						ImVec2 text_size = ImGui::CalcTextSize(text_distance.c_str());
						_DrawText(ESPFont, ImVec2(bottom_middle.x - (text_size.x / 2), most_bottom + offset + pushback), ImColor(255, 255, 255), text_distance.c_str(), visual->size - 5, false);
					}
					if (entity->CurrentWeapon()->IsReloadingWeapon()) {
						pushback += 10;
						std::string text_distance = "Reloading Weapon";
						ImVec2 text_size = ImGui::CalcTextSize(text_distance.c_str());
						_DrawText(ESPFont, ImVec2(bottom_middle.x - (text_size.x / 2), most_bottom + offset + pushback), ImColor(255, 255, 255), text_distance.c_str(), visual->size - 5, false);
					}
					if (PlayerData->bIsChargingWeapon) {
						pushback += 10;
						std::string text_distance = "Charging Weapon";
						ImVec2 text_size = ImGui::CalcTextSize(text_distance.c_str());
						_DrawText(ESPFont, ImVec2(bottom_middle.x - (text_size.x / 2), most_bottom + offset + pushback), ImColor(255, 255, 255), text_distance.c_str(), visual->size - 5, false);
					}
					if (PlayerData->bIsTargeting) {
						pushback += 10;
						std::string text_distance = "Targeting";
						ImVec2 text_size = ImGui::CalcTextSize(text_distance.c_str());
						_DrawText(ESPFont, ImVec2(bottom_middle.x - (text_size.x / 2), most_bottom + offset + pushback), ImColor(255, 255, 255), text_distance.c_str(), visual->size - 5, false);
					}
					if (PlayerData->bIsSliding) {
						pushback += 10;
						std::string text_distance = "Slidding";
						ImVec2 text_size = ImGui::CalcTextSize(text_distance.c_str());
						_DrawText(ESPFont, ImVec2(bottom_middle.x - (text_size.x / 2), most_bottom + offset + pushback), ImColor(255, 255, 255), text_distance.c_str(), visual->size - 5, false);
					}
					if (PlayerData->bWeaponHolstered) {
						pushback += 10;
						std::string text_distance = "Weapon Holstered";
						ImVec2 text_size = ImGui::CalcTextSize(text_distance.c_str());
						_DrawText(ESPFont, ImVec2(bottom_middle.x - (text_size.x / 2), most_bottom + offset + pushback), ImColor(255, 255, 255), text_distance.c_str(), visual->size - 5, false);
					}
					if (PlayerData->bMovingEmote) {
						pushback += 10;
						std::string text_distance = "Emoting";
						ImVec2 text_size = ImGui::CalcTextSize(text_distance.c_str());
						_DrawText(ESPFont, ImVec2(bottom_middle.x - (text_size.x / 2), most_bottom + offset + pushback), ImColor(255, 255, 255), text_distance.c_str(), visual->size - 5, false);
					}
					if (PlayerData->PhantomReserveAmmo != 0) {
						pushback += 10;
						std::string text_distance = "ReserveAmmo (" + std::to_string(PlayerData->PhantomReserveAmmo) + ")";
						ImVec2 text_size = ImGui::CalcTextSize(text_distance.c_str());
						_DrawText(ESPFont, ImVec2(bottom_middle.x - (text_size.x / 2), most_bottom + offset + pushback), ImColor(255, 255, 255), text_distance.c_str(), visual->size - 5, false);
					}
					if (entity->CurrentWeapon()->GetAmmoCount() != 0) {
						pushback += 10;
						std::string text_distance = "AmmoCount (" + std::to_string(entity->CurrentWeapon()->GetAmmoCount()) + ")";
						ImVec2 text_size = ImGui::CalcTextSize(text_distance.c_str());
						_DrawText(ESPFont, ImVec2(bottom_middle.x - (text_size.x / 2), most_bottom + offset + pushback), ImColor(255, 255, 255), text_distance.c_str(), visual->size - 5, false);
					}
					if (PlayerData->BurstFireCounter != 0) {
						pushback += 10;
						std::string text_distance = "BurstFireCounter (" + std::to_string(PlayerData->BurstFireCounter) + ")";
						ImVec2 text_size = ImGui::CalcTextSize(text_distance.c_str());
						_DrawText(ESPFont, ImVec2(bottom_middle.x - (text_size.x / 2), most_bottom + offset + pushback), ImColor(255, 255, 255), text_distance.c_str(), visual->size - 5, false);
					}
					if (PlayerData->Score != 0.f) {
						pushback += 10;
						std::string text_distance = "Kills (" + std::to_string(PlayerData->Score) + ")";
						ImVec2 text_size = ImGui::CalcTextSize(text_distance.c_str());
						_DrawText(ESPFont, ImVec2(bottom_middle.x - (text_size.x / 2), most_bottom + offset + pushback), ImColor(255, 255, 255), text_distance.c_str(), visual->size - 5, false);
					}
					if (PlayerData->ChargeTime != 0.f) {
						pushback += 10;
						std::string text_distance = "ChargeTime (" + std::to_string(PlayerData->ChargeTime) + ")";
						ImVec2 text_size = ImGui::CalcTextSize(text_distance.c_str());
						_DrawText(ESPFont, ImVec2(bottom_middle.x - (text_size.x / 2), most_bottom + offset + pushback), ImColor(255, 255, 255), text_distance.c_str(), visual->size - 5, false);
					}
					if (PlayerData->TimeToEquip != 0.f) {
						pushback += 10;
						std::string text_distance = "TimeToEquip (" + std::to_string(PlayerData->TimeToEquip) + ")";
						ImVec2 text_size = ImGui::CalcTextSize(text_distance.c_str());
						_DrawText(ESPFont, ImVec2(bottom_middle.x - (text_size.x / 2), most_bottom + offset + pushback), ImColor(255, 255, 255), text_distance.c_str(), visual->size - 5, false);
					}
					if (PlayerData->LastChargeTime != 0.f) {
						pushback += 10;
						std::string text_distance = "LastChargeTime (" + std::to_string(PlayerData->LastChargeTime) + ")";
						ImVec2 text_size = ImGui::CalcTextSize(text_distance.c_str());
						_DrawText(ESPFont, ImVec2(bottom_middle.x - (text_size.x / 2), most_bottom + offset + pushback), ImColor(255, 255, 255), text_distance.c_str(), visual->size - 5, false);
					}
					if (PlayerData->LastSuccessfulReloadTime != 0.f) {
						pushback += 10;
						std::string text_distance = "LastSuccessfulReloadTime (" + std::to_string(PlayerData->LastSuccessfulReloadTime) + ")";
						ImVec2 text_size = ImGui::CalcTextSize(text_distance.c_str());
						_DrawText(ESPFont, ImVec2(bottom_middle.x - (text_size.x / 2), most_bottom + offset + pushback), ImColor(255, 255, 255), text_distance.c_str(), visual->size - 5, false);
					}
					if (PlayerData->LastReloadTime != 0.f) {
						pushback += 10;
						std::string text_distance = "LastReloadTime (" + std::to_string(PlayerData->LastReloadTime) + ")";
						ImVec2 text_size = ImGui::CalcTextSize(text_distance.c_str());
						_DrawText(ESPFont, ImVec2(bottom_middle.x - (text_size.x / 2), most_bottom + offset + pushback), ImColor(255, 255, 255), text_distance.c_str(), visual->size - 5, false);
					}
					if (PlayerData->CurrentReloadDuration != 0.f) {
						pushback += 10;
						std::string text_distance = "CurrentReloadTime (" + std::to_string(PlayerData->CurrentReloadDuration) + ")";
						ImVec2 text_size = ImGui::CalcTextSize(text_distance.c_str());
						_DrawText(ESPFont, ImVec2(bottom_middle.x - (text_size.x / 2), most_bottom + offset + pushback), ImColor(255, 255, 255), text_distance.c_str(), visual->size - 5, false);
					}
					if (PlayerData->TriggerType != EFortWeaponTriggerType::EFortWeaponTriggerType_MAX) {
						pushback += 10;
						std::string text_distance = "TriggerType (" + std::to_string(PlayerData->TriggerType) + ")";
						ImVec2 text_size = ImGui::CalcTextSize(text_distance.c_str());
						_DrawText(ESPFont, ImVec2(bottom_middle.x - (text_size.x / 2), most_bottom + offset + pushback), ImColor(255, 255, 255), text_distance.c_str(), visual->size - 5, false);
					}
					if (PlayerData->MovementMode != EMovementMode::MOVE_MAX) {
						pushback += 10;
						std::string text_distance = "MovementType (" + std::to_string(PlayerData->MovementMode) + ")";
						ImVec2 text_size = ImGui::CalcTextSize(text_distance.c_str());
						_DrawText(ESPFont, ImVec2(bottom_middle.x - (text_size.x / 2), most_bottom + offset + pushback), ImColor(255, 255, 255), text_distance.c_str(), visual->size - 5, false);
					}

				}
			}
		} 
		lock.unlock();
	} while (FALSE);
}

void itempawn_loop()
{
	do {
		std::unique_lock<std::mutex> lock(global::data_mutex);
		for (unsigned long i = 0; i < global::Item.size(); ++i)
		{
			AFortPawn* entity = global::Item[i];

			if (visual->loot->loot)
			{
				USceneComponent* RootComponent = entity->RootComponent();
				FVector RelativeLocation = RootComponent->RelativeLocation();

				//if (request->read<bool>((FortPTR)entity + 0x2A2))
				//	std::cout << " AutoPickup is on!" << std::endl;

				if (misc->bIgnoreWaterLoot) {
					float SimulatingTooLongLengthInWaterMoving = request->read<float>((FortPTR)entity + 0x2AC);
					float SimulatingTooLongLengthInWaterBobbing = request->read<float>((FortPTR)entity + 0x2B0);
					if (SimulatingTooLongLengthInWaterMoving <= 0.f && SimulatingTooLongLengthInWaterBobbing <= 0.f)
						continue;
				}

				if (misc->bIgnoreDespawned) {
					float DespawnTime = request->read<float>((FortPTR)entity + 0x728);
					float StormDespawnTime = request->read<float>((FortPTR)entity + 0x72C);
					if (DespawnTime >= 2 && StormDespawnTime >= 2)
						continue;
				}

				if (misc->bIgnorePickedup) {
					bool bPickedUp = request->read<bool>((FortPTR)entity + 0x5C9);
					if (!bPickedUp)
						continue;
				}

				double item_distance = 0.0;
				if (item_distance = camera.Location.Distance(RelativeLocation) / 100.0 <= visual->loot->MaxItemDistance)
					continue;

				if (visual->loot->pickups && entity->GetActorFName().contains("FortPickupAthena") ||
					entity->GetActorFName().contains("Fort_Pickup_Creative_C"))
				{
					FVector2D Position = global::LocalPlayer->W2S(RelativeLocation);
					if (IsInScreen(Position)) {
						uintptr_t PrimaryPickupItemEntry = request->read<uintptr_t>((FortPTR)entity + offsets::PrimaryPickupItemEntry);
						uintptr_t ItemDefinition = request->read<uintptr_t>((FortPTR)entity + offsets::ItemDefinition);
						std::string DisplayName = request->read<std::string>((FortPTR)entity + 0x90);
						EFortRarity HighestRarity = (EFortRarity)request->read<int>((FortPTR)entity + 0xD90);
						FColor ItemColor;
						ItemColor.A = 255;
						if (HighestRarity == EFortRarity::Common) { //Rarity: Uncommon
							ItemColor.R = 61; ItemColor.G = 61; ItemColor.B = 76;
						}
						if (HighestRarity == EFortRarity::Uncommon) { //Rarity: Green
							ItemColor.R = 52; ItemColor.G = 71; ItemColor.B = 52;
						}
						else if (HighestRarity == EFortRarity::Rare) { //Rarity: Blue
							ItemColor.R = 87; ItemColor.G = 134; ItemColor.B = 208;
						}
						else if (HighestRarity == EFortRarity::Epic) { //Rarity: Purple
							ItemColor.R = 126; ItemColor.G = 0; ItemColor.B = 220;
						}
						else if (HighestRarity == EFortRarity::Legendary) { //Rarity: Gold
							ItemColor.R = 195; ItemColor.G = 116; ItemColor.B = 43;
						}
						else if (HighestRarity == EFortRarity::Mythic) { //Rarity: Gold+
							ItemColor.R = 169; ItemColor.G = 139; ItemColor.B = 42;
						}
						std::string text_position = DisplayName + " [" + std::to_string(item_distance) + "m]";
						ImVec2 TextSize = ImGui::CalcTextSize(text_position.c_str());
						_DrawText(ESPFont, ImVec2((Position.x) - (TextSize.x / 2.0), (Position.y - 20.0)), ImColor(255, 255, 255), text_position.c_str());
					}
				}
				if (visual->loot->chests && entity->GetActorFName().contains("Tiered_Chest"))
				{
					FVector2D Position = global::LocalPlayer->W2S(RelativeLocation);
					if (IsInScreen(Position)) {
						std::string text_position = "Chest [" + std::to_string(item_distance) + "m]";
						ImVec2 TextSize = ImGui::CalcTextSize(text_position.c_str());
						_DrawText(ESPFont, ImVec2((Position.x) - (TextSize.x / 2.0), (Position.y - 20.0)), ImColor(255, 255, 255), text_position.c_str());
					}
				}
				if (visual->loot->ammo && entity->GetActorFName().contains("Tiered_Ammo"))
				{
					FVector2D Position = global::LocalPlayer->W2S(RelativeLocation);
					if (IsInScreen(Position)) {
						std::string text_position = "Ammo [" + std::to_string(item_distance) + "m]";
						ImVec2 TextSize = ImGui::CalcTextSize(text_position.c_str());
						_DrawText(ESPFont, ImVec2((Position.x) - (TextSize.x / 2.0), (Position.y - 20.0)), ImColor(255, 255, 255), text_position.c_str());
					}
				}
				if (visual->loot->supply && entity->GetActorFName().contains("AthenaSupplyDrop_C"))
				{
					FVector2D Position = global::LocalPlayer->W2S(RelativeLocation);
					if (IsInScreen(Position)) {
						std::string text_position = "Supply Drop [" + std::to_string(item_distance) + "m]";
						ImVec2 TextSize = ImGui::CalcTextSize(text_position.c_str());
						_DrawText(ESPFont, ImVec2((Position.x) - (TextSize.x / 2.0), (Position.y - 20.0)), ImColor(255, 255, 255), text_position.c_str());
					}
				}
				if (visual->loot->vehicle && entity->GetActorFName().contains("Vehicle") ||
					entity->GetActorFName().contains("MeatballVehicle_L") ||
					entity->GetActorFName().contains("Valet_Taxi") ||
					entity->GetActorFName().contains("Valet_BigRig") ||
					entity->GetActorFName().contains("Valet_BasicTr") ||
					entity->GetActorFName().contains("Valet_SportsC") ||
					entity->GetActorFName().contains("Valet_BasicC")) {
					FVector2D Position = global::LocalPlayer->W2S(RelativeLocation);
					if (IsInScreen(Position)) {
						std::string text_position = "Vehicle [" + std::to_string(item_distance) + "m]";
						ImVec2 TextSize = ImGui::CalcTextSize(text_position.c_str());
						_DrawText(ESPFont, ImVec2((Position.x) - (TextSize.x / 2.0), (Position.y - 20.0)), ImColor(255, 255, 255), text_position.c_str());
					}
				}
				if (visual->loot->animal && entity->GetActorFName().contains("NPC_Pawn_Irwin_Predator_Robert_C") ||
					entity->GetActorFName().contains("NPC_Pawn_Irwin_Prey_Burt_C") ||
					entity->GetActorFName().contains("NPC_Pawn_Irwin_Simple_Smackie_C") ||
					entity->GetActorFName().contains("NPC_Pawn_Irwin_Prey_Nug_C") ||
					entity->GetActorFName().contains("NPC_Pawn_Irwin_Predator_Grandma_C")) {
					FVector2D Position = global::LocalPlayer->W2S(RelativeLocation);
					if (IsInScreen(Position)) {
						std::string text_position = "Animal [" + std::to_string(item_distance) + "m]";
						ImVec2 TextSize = ImGui::CalcTextSize(text_position.c_str());
						_DrawText(ESPFont, ImVec2((Position.x) - (TextSize.x / 2.0), (Position.y - 20.0)), ImColor(255, 255, 255), text_position.c_str());
					}
				}
				if (visual->loot->generades && entity->GetActorFName().contains("Grenade")) {
					FVector2D Position = global::LocalPlayer->W2S(RelativeLocation);
					if (IsInScreen(Position)) {
						std::string text_position = "Grenade [" + std::to_string(item_distance) + "m]";
						ImVec2 TextSize = ImGui::CalcTextSize(text_position.c_str());
						_DrawText(ESPFont, ImVec2((Position.x) - (TextSize.x / 2.0), (Position.y - 20.0)), ImColor(255, 255, 255), text_position.c_str());
					}
				}
				if (visual->loot->trap && entity->GetActorFName().contains("BuildingTrap")) {
					if (/*auto ObjectID = entity->GetClassId((FortPTR)entity);*/ entity->IsA((FortPTR)entity, 3757)) {
						FVector2D Position = global::LocalPlayer->W2S(RelativeLocation);
						if (IsInScreen(Position)) {
							std::string text_position = "trap [" + std::to_string(item_distance) + "m]";
							ImVec2 TextSize = ImGui::CalcTextSize(text_position.c_str());
							_DrawText(ESPFont, ImVec2((Position.x) - (TextSize.x / 2.0), (Position.y - 20.0)), ImColor(255, 255, 255), text_position.c_str());
						}
					}
				}
				if (visual->loot->rift && entity->GetActorFName().contains("FortAthenaRiftPortal")) {
					if (auto ObjectID = entity->GetClassId((FortPTR)entity); entity->IsA((FortPTR)entity, ObjectID)) {
						std::cout << " ObjectID " << ObjectID << std::endl; // decimal responce
						FVector2D Position = global::LocalPlayer->W2S(RelativeLocation);
						if (IsInScreen(Position)) {
							std::string text_position = "rift [" + std::to_string(item_distance) + "m]";
							ImVec2 TextSize = ImGui::CalcTextSize(text_position.c_str());
							_DrawText(ESPFont, ImVec2((Position.x) - (TextSize.x / 2.0), (Position.y - 20.0)), ImColor(255, 255, 255), text_position.c_str());
						}
					}
				}

			}
		}
		lock.unlock();
	} while (FALSE);
}

void text_patch_exploits()
{
	do {
		static bool no_bullets = true;
		static bool no_spread = false;
		static bool unlock_fire_rate = false;
		static bool no_reload = false;
		static bool instant_reload = false;
		static bool aim_jump = false;

		while (true)
		{

			for (unsigned long i = 0; i < global::Player.size(); ++i)
			{
				AFortPawn* entity = global::Player[i];
				if (exploits->cheat_flying) {
					static uintptr_t CharacterMovement = request->read<uintptr_t>((FortPTR)entity + 0x320); //
					if (hook::is_valid(CharacterMovement) && hook::is_valid((FortPTR)global::LocalPawn)) {
						if (EMovementMode MovementMode = (EMovementMode)request->read<uintptr_t>(CharacterMovement + 0x201);
							request->write<EMovementMode>(MovementMode, EMovementMode::MOVE_Flying)) {
							//FVector Acceleration = request->read<FVector>(CharacterMovement + 0x2F8);
							request->write<bool>((request->read<bool>(CharacterMovement) + 0x505 & 0x10), false); // bCheatFlying
							request->write<float>(request->read<uintptr_t>((FortPTR)entity->RootComponent() + 0xA8 + 0x2D0), 0.f); // FluidFriction
						}
					}
				}

				if (exploits->fire_rate != exploits->fire_rate_old) {
					uintptr_t GetFireRate = request->read<uintptr_t>(hook::base_address + 0x9BAE2B0);
					BYTE original_bytes[] = { 0xF3, 0x0F, 0x11, 0x03 };

					if (exploits->fire_rate) {
						for (int i = 0; i < sizeof(original_bytes); i++) {
							request->write<BYTE>(GetFireRate + i, hook::nop);
						}
					}
					else {
						for (int i = 0; i < sizeof(original_bytes); i++) {
							request->write<BYTE>(GetFireRate + i, original_bytes[i]);
						}
					}

					exploits->fire_rate_old = exploits->fire_rate;
				}

				if (exploits->no_spread != exploits->no_spread_old) {
					uintptr_t GetCalculatedReticleSpread = request->read<uintptr_t>(hook::base_address + 0x9B995BF);
					BYTE original_bytes[] = { 0xF3, 0x0F, 0x11, 0x03 };

					if (exploits->no_spread) {
						for (int i = 0; i < sizeof(original_bytes); i++) {
							request->write<BYTE>(GetCalculatedReticleSpread + i, hook::nop);
						}
					}
					else {
						for (int i = 0; i < sizeof(original_bytes); i++) {
							request->write<BYTE>(GetCalculatedReticleSpread + i, original_bytes[i]);
						}
					}

					exploits->no_spread = exploits->no_spread_old;
				}

				if (exploits->instant_reload != exploits->instant_reload_old) {
					uintptr_t GetReloadTime = request->read<uintptr_t>(hook::base_address + 0x9B84F2F); // Weapon 0x980
					BYTE original_bytes[] = { 0xF3, 0x0F, 0x11, 0x03 };

					if (exploits->instant_reload) {
						for (int i = 0; i < sizeof(original_bytes); i++) {
							request->write<BYTE>(GetReloadTime + i, hook::nop);
						}
					}
					else {
						for (int i = 0; i < sizeof(original_bytes); i++) {
							request->write<BYTE>(GetReloadTime + i, original_bytes[i]);
						}
					}
					 
					exploits->instant_reload = exploits->instant_reload_old;
				}

				if (exploits->no_clip) {
					if (!exploits->no_clip_init) {
						uintptr_t Exploit = request->read<uintptr_t>(hook::base_address + 0x9AC7AFC + 0x3A);
						uintptr_t Exploit2 = request->read<uintptr_t>(hook::base_address + 0x9AC7B4E); //NOTE: fixed crash when shoot
						for (int32_t i = 0; i++;) {
							uintptr_t Address = request->read<uintptr_t>(Exploit + i);
							if (Address == Exploit2 || !hook::is_valid(Exploit)) break;

							request->write<BYTE>(Address, hook::ret);
						} exploits->no_clip_init = true;
					}
				}

				if (exploits->no_over_heat != exploits->no_over_heat_old) {
					uintptr_t GetOverheatingMaxValue = request->read<uintptr_t>(hook::base_address + 0x862223D);
					BYTE original_bytes[] = { 0x8B, 0x81, 0xD0, 0x00, 0x00, 0x00 };

					if (exploits->no_over_heat) {
						for (int i = 0; i < sizeof(original_bytes); i++) {
							request->write<BYTE>(GetOverheatingMaxValue + i, hook::nop);
						}
					}
					else {
						for (int i = 0; i < sizeof(original_bytes); i++) {
							request->write<BYTE>(GetOverheatingMaxValue + i, original_bytes[i]);
						}
					}

					exploits->no_over_heat = exploits->no_over_heat_old;
				}

				if (GetAsyncKeyState(VK_MBUTTON) && entity->GetActorFName().contains("AthenaPlayerMarker_WithCustomization"))
				{
					static uintptr_t CharacterMovement = request->read<uintptr_t>((FortPTR)entity + 0x320);
					FVector marker_location = entity->RootComponent()->RelativeLocation();
					request->write<bool>((BYTE)(CharacterMovement + 0x505 & 0x10), FALSE);
					request->write<FVector>((FortPTR)entity->RootComponent() + offsets::RelativeLocation, marker_location);
				}

				if (exploits->no_recoil)
				{
					static INT32 BulletCountForPerBulletRecoil_old = request->read<INT32>((FortPTR)entity->CurrentWeapon() + 0x17A4);
					uintptr_t BulletCountForPerBulletRecoil = request->read<uintptr_t>((FortPTR)entity->CurrentWeapon() + 0x17A4);
					if (!hook::is_valid((FortPTR)entity->CurrentWeapon()) || entity->CurrentWeapon()->GetAmmoCount() < 0 || !hook::is_valid(BulletCountForPerBulletRecoil)) // avoid crashes
						request->write<INT32>(BulletCountForPerBulletRecoil, BulletCountForPerBulletRecoil_old);
					else request->write<INT32>(BulletCountForPerBulletRecoil, 0);
				}

				if (exploits->magic_bullet)
				{
					uintptr_t player_capsule = request->read<uintptr_t>((FortPTR)entity + 0x328);
					request->write<FVector>(player_capsule, FVector(20, 20, 20));
				}

			}

		}
	} while (FALSE);
}

BOOL main_loop()
{
	free_CloseHandle(free_CreateThread(nullptr, 0, nullptr, (LPTHREAD_START_ROUTINE)aimbot_thread, 0, nullptr));
	while (D3D11->Message.message != WM_QUIT)
	{
		if (free_PeekMessageA(&D3D11->Message, NULL, 0, 0, PM_REMOVE))
		{
			free_TranslateMessage(&D3D11->Message);
			free_DispatchMessageA(&D3D11->Message);
		}

		if (misc->StreamProof)
		{
			SetWindowDisplayAffinity(D3D11->Window, WDA_EXCLUDEFROMCAPTURE);
		}
		else
			SetWindowDisplayAffinity(D3D11->Window, WDA_NONE);

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		//playerpawn_loop();
		//itempawn_loop();
		draw_menu();

		ImGui::EndFrame();
		ImGui::Render();
		const ImVec4 clear_color = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		float render_color_rgba_array[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
		D3D11->D3D11DeviceContext->OMSetRenderTargets(1, &D3D11->MainRenderTargetView, nullptr);
		D3D11->D3D11DeviceContext->ClearRenderTargetView(D3D11->MainRenderTargetView, render_color_rgba_array);
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		D3D11->pSwapChain->Present(misc->VSync, 0);
	}

	return D3D11->Message.message != WM_QUIT;
}
#endif // RENDER_HPP