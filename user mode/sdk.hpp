#pragma once
#include <d3d9.h>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <functional>
#include <chrono>
#include <mutex>
#include "Comms/driver.hpp"
#include "structs.hpp"
#include "hexray.hpp"
#include "variables.hpp"
#ifndef SDK_HPP
#define SDK_HPP

inline void debug_pointer(uintptr_t p, const char* s) {
	printf(("%s %I64u \n"), s, p);
}

namespace offsets
{
	FortStorage Gworld = 0xEE2F8E8;
	FortStorage ViewPoint = 0xED57310;
	FortStorage OwningGameInstance = 0x1B8;
	FortStorage PlayerController = 0x30;
	FortStorage AcknowledgedPawn = 0x338;
	FortStorage PlayerCameraManager = 0x348;
	FortStorage LastFrameCameraCachePrivate = 0x2A80;
	FortStorage DefaultFOV = 0x2A4;
	FortStorage ControlRotation = 0x300;
	FortStorage MyHUD = 0x340;
	FortStorage Canvas = 0x2E0;
	FortStorage SizeX = 0x40;
	//FortStorage SizeY = 0x44;
	FortStorage WorldSettings = 0x2A0;
	FortStorage PlayerState = 0x2B0;
	FortStorage PlayerArray = 0x2A0;
	FortStorage RootComponent = 0x198;
	FortStorage CurrentVehicle = 0x2590;
	FortStorage RelativeLocation = 0x128;
	FortStorage CurrentWeapon = 0x948;
	FortStorage WeaponData = 0x450;
	FortStorage Mesh = 0x318;
	FortStorage TeamID = 0x10D0;
	FortStorage GameStates = 0x158;
	FortStorage RelativeRotation = 0x140;
	FortStorage InputYawScale = 0x538;
	FortStorage InputPitchScale = 0x53C;
	FortStorage ComponentVelocity = 0x170;
	FortStorage RelativeScale3D = 0x158;
	FortStorage TargetedBuild = 0x1840;
	FortStorage DisplayName = 0x98;
	FortStorage Length = 0x40;
	FortStorage levels = 0x170;
	FortStorage PrimaryPickupItemEntry = 0x330;
	FortStorage ItemDefinition = 0x18;
	FortStorage HUDScale = 0x348;
}

class camera_ {
public:
	FVector Location;
	FVector Rotation;
	float FieldOfView;
	char Useless[0x18];
}; camera_ camera;

camera_ get_camera();
std::string ActorFName(int key);

class USceneComponent
{
public:
	FVector ComponentVelocity()
	{
		if (!(FortPTR)this) return { };
		return request->read<FVector>((FortPTR)this + offsets::ComponentVelocity);
	}

	FVector RelativeLocation()
	{
		if(!(FortPTR)this) return { };
		return request->read<FVector>((FortPTR)this + offsets::RelativeLocation);
	}

	FVector RelativeRotation()
	{
		if (!(FortPTR)this) return { };
		return request->read<FVector>((FortPTR)this + offsets::RelativeRotation);
	}

	FVector RelativeScale3D()
	{
		if (!(FortPTR)this) return { };
		return request->read<FVector>((FortPTR)this + offsets::RelativeScale3D);
	}

};

class UCanvas {
public:
	int SizeX() {
		if (!(FortPTR)this) return {};
		return request->read<int>((FortPTR)this + offsets::SizeX);
	}

	int SizeY() {
		if (!(FortPTR)this) return {};
		return request->read<int>((FortPTR)this + offsets::SizeX + sizeof(int));
	}

};

class UFortClientSettingsRecord {
public:
	float HUDScale()
	{
		if (!(FortPTR)this) return {};
		return request->read<int>((FortPTR)this + offsets::SizeX + sizeof(int));
	}

};

class AHUD {
public:
	UCanvas* Canvas() {
		if (!(FortPTR)this) return {};
		return (UCanvas*)request->read<uintptr_t>((FortPTR)this + offsets::Canvas);
	}

};

class APlayerState
{
public:
	int TeamIndex()
	{
		if (!(FortPTR)this) return {};
		return request->read<int>((FortPTR)this + offsets::TeamID);
	}

	std::string GetPlatformName()
	{
		if (!(FortPTR)this)
			return "";

		uintptr_t pNameStructure = request->read<uintptr_t>((FortPTR)this + 0x430);
		auto pNameLength = request->read<int>(pNameStructure + 0x10);
		if (pNameLength <= 0) return "";

		wchar_t* pNameBuffer = new wchar_t[pNameLength];
		request->read_array(pNameStructure, &pNameBuffer, pNameLength * sizeof(wchar_t));
		std::wstring temp_wstring(pNameBuffer);
		return std::string(temp_wstring.begin(), temp_wstring.end());
	}

	std::string GetPlayerName()
	{
		if (!(FortPTR)this)
			return {};

		int pNameLength; // rsi
		char v21; // al
		int v22; // r8d
		int i; // ecx
		int v25; // eax
		_WORD* v23;

		__int64 pNameStructure = request->read<__int64>((FortPTR)this + 0xAD8); // 0xAC8
		pNameLength = request->read<int>(pNameStructure + 16);
		__int64 v6 = pNameLength;
		if (!v6) return std::string("bot");

		wchar_t* pNameBuffer = new wchar_t[pNameLength];
		uintptr_t pNameEncryptedBuffer = request->read<__int64>(pNameStructure + 8);
		request->read_array(pNameEncryptedBuffer, &pNameBuffer, pNameLength * sizeof(wchar_t));

		v21 = v6 - 1;
		if (!(_DWORD)v6)
			v21 = 0;
		v22 = 0;
		v23 = (_WORD*)pNameBuffer;

		for (i = (v21) & 3; ; *v23++ += i & 7)
		{
			v25 = v6 - 1;
			if (!(_DWORD)v6)
				v25 = 0;

			if (v22 >= v25)
				break;

			i += 3;
			++v22;
		}

		std::wstring Temp{ pNameBuffer };
		return std::string(Temp.begin(), Temp.end());
	}

	void RetrivePlayerData() {
		if (!(FortPTR)this) return;
		PlayerData->CompressedPing = request->read<uint8>((FortPTR)this + 0x290);
		PlayerData->PlayerId = request->read<uint8>((FortPTR)this + 0x28C);
		PlayerData->Score = request->read<uint8>((FortPTR)this + 0x288);
		PlayerData->IsValid = true;
	}

};

class APlayerCameraManager 
{
public:
	camera_ LastFrameCameraCachePrivate()
	{
		if (!(FortPTR)this) return {};
		return request->read<camera_>((FortPTR)this + offsets::LastFrameCameraCachePrivate + 0x10);
	}

	float DefaultFOV() const 
	{
		if (!(FortPTR)this) return 0.f;
		return request->read<float>((FortPTR)this + offsets::DefaultFOV);
	}

};

class UFortWeaponItemDefinition {
public:

};

class AFortWeapon
{
public:
	ImColor CurrentReticleColor() {
		if (!(FortPTR)this) return {};
		return request->read<bool>((FortPTR)this + 0x62C);
	}

	UFortWeaponItemDefinition* WeaponData() {
		if (!(FortPTR)this) return {};
		return request->read<UFortWeaponItemDefinition*>((FortPTR)this + offsets::WeaponData);
	}

	float GetProjectileSpeed() const {
		if (!(FortPTR)this) return {};
		return request->read<float>((FortPTR)this + 0x174); // 0x9C0
	}

	int GetAmmoCount() {
		if (!(FortPTR)this) return 0;
		return request->read<int32>((FortPTR)this + 0xCFC);
	}

	bool IsReloadingWeapon() {
		if (!(FortPTR)this) return false;
		return request->read<bool>((FortPTR)this + 0x358);
	}

	std::string GetWeaponName() const {
		if (!(FortPTR)this) return {};

		auto itemdef = request->read<uint64_t>((FortPTR)this + offsets::WeaponData);	
		if (!itemdef) return "";
		auto DisplayName = request->read<uint64_t>(itemdef + offsets::DisplayName);
		if (!DisplayName) return "";
		auto WeaponLength = request->read<uint32_t>(DisplayName + offsets::Length);
		wchar_t* WeaponName = new wchar_t[uint64_t(WeaponLength) + 1];

		request->read_array((ULONG64)request->read<PVOID>(DisplayName + 0x38), WeaponName, WeaponLength * sizeof(wchar_t));

		std::wstring wWeaponName(WeaponName);
		return std::string(wWeaponName.begin(), wWeaponName.end());
	}

	void RetriveWeaponData() {
		if (!(FortPTR)this) return;
		PlayerData->bIsEquippingWeapon = request->read<bool>((FortPTR)this + 0x338);
		PlayerData->bIsChargingWeapon = request->read<bool>((FortPTR)this + 0x351);
		PlayerData->bIsTargeting = request->read<bool>((FortPTR)this + 0xB5D);
		PlayerData->TimeToEquip = request->read<float>((FortPTR)this + 0x328);
		PlayerData->LastReloadTime = request->read<float>((FortPTR)this + 0xBC8);
		PlayerData->LastSuccessfulReloadTime = request->read<float>((FortPTR)this + 0xBCC);
		PlayerData->CurrentReloadDuration = request->read<float>((FortPTR)this + 0xBD0);
		PlayerData->WeaponLevel = request->read<int32>((FortPTR)this + 0xBF8);
		PlayerData->PhantomReserveAmmo = request->read<float>((FortPTR)this + 0xC00);
		PlayerData->BurstFireCounter = request->read<INT32>((FortPTR)this + 0xC0C);
		PlayerData->ChargeTime = request->read<float>((FortPTR)this + 0xC10);
		PlayerData->AccumulatedChargeTime = request->read<float>((FortPTR)this + 0xC14);
		PlayerData->LastChargeTime = request->read<float>((FortPTR)this + 0xC18);
		PlayerData->TriggerType = (EFortWeaponTriggerType)request->read<int>((FortPTR)this->WeaponData() + 0xbfc);
		PlayerData->IsValid = true;
	}
};

class AFortAthenaVehicle {
public:
	bool CurrentTeam(AFortAthenaVehicle* B) {
		if (!(FortPTR)this) return {};
		uint8 EnemyTeamId = request->read<uint8>((FortPTR)B + 0x6D1);
		if (uint8 TeamId = request->read<uint8>((FortPTR)this + 0x6D1); EnemyTeamId == TeamId)
			return true;
		return false;
	}

	EVehicleSurface GetCurrentSurface() {
		if (!(FortPTR)this) return {};
		bool bOnRoad = request->read<bool>((FortPTR)this + 0x1968);
		bool bOnLandscape = request->read<bool>((FortPTR)this + 0x1969);
		bool bOnDirt = request->read<bool>((FortPTR)this + 0x196A);
		bool bOnGrass = request->read<bool>((FortPTR)this + 0x196B);
		bool bOnIce = request->read<bool>((FortPTR)this + 0x196C);
		bool bOnSnow = request->read<bool>((FortPTR)this + 0x196D);
		bool bOnMud = request->read<bool>((FortPTR)this + 0x196E);
		bool bOnVehicle = request->read<bool>((FortPTR)this + 0x196F);

		if (bOnRoad)
			return EVehicleSurface::bOnRoad;
		else if (bOnDirt)
			return EVehicleSurface::bOnDirt;
		else if (bOnGrass)
			return EVehicleSurface::bOnGrass;
		else if (bOnIce)
			return EVehicleSurface::bOnIce;
		else if (bOnSnow)
			return EVehicleSurface::bOnSnow;
		else if (bOnMud)
			return EVehicleSurface::bOnMud;
		else if (bOnVehicle)
			return EVehicleSurface::bOnVehicle;
		else if (bOnLandscape)
			return EVehicleSurface::bOnLandscape;

	}

	ETireSurfaces GetCurrentTireSurface()
	{
		if (!(FortPTR)this) return {};
		return (ETireSurfaces)request->read<uint8>((FortPTR)this + 0x1970);
	}

	ETireStates TireState()
	{
		if (!(FortPTR)this) return {};
		return (ETireStates)request->read<int>((FortPTR)this + 0x1998);
	}

	float GetCurrentFOV() const {
		if (!(FortPTR)this) return {};
		return request->read<float>((FortPTR)this + 0x764);
	}

	bool CarBoosting() {
		if (!(FortPTR)this) return {};
		bool PendingJumpCharge = request->read<bool>((FortPTR)this + 0x1B58);
		float JumpCooldownRemaining = request->read<float>((FortPTR)this + 0x1B4C);
		if (PendingJumpCharge || JumpCooldownRemaining != 0.f)
			return false;

		return request->read<bool>((FortPTR)this + 0x18A4);
	}

	float WaterLevel() {
		if (!(FortPTR)this) return {};
		if (this->GetCurrentTireSurface() == ETireSurfaces::Water)
			return request->read<float>((FortPTR)this + 0xCB4);
		return 0.f;
	}

	float VechileSpeed() {
		if (!(FortPTR)this) return {};
		return request->read<float>((FortPTR)this + 0xCB4);
	}
	
};

class USkeletalMeshComponent
{
public:
	FVector GetBone(int Bone)
	{
		if (!(FortPTR)this) return {};
		int is_cached = request->read<int>((FortPTR)this + 0x648);
		auto bone_transform = request->read<FTransform>(request->read<uintptr_t>((FortPTR)this + 0x10 * is_cached + 0x600) + 0x60 * Bone);

		FTransform ComponentToWorld = request->read<FTransform>((FortPTR)this + 0x240);

		D3DMATRIX Matrix;
		Matrix = MatrixMultiplication(bone_transform.ToMatrixWithScale(), ComponentToWorld.ToMatrixWithScale());

		return FVector(Matrix._41, Matrix._42, Matrix._43);
	}

	bool WasRecentlyRendered(float Tolerance = 0.06f) // 0.015f
	{
		if (!(FortPTR)this) return false;
		auto fLastSubmitTime = request->read<float>((FortPTR)this + 0x360);
		auto fLastRenderTimeOnScreen = request->read<float>((FortPTR)this + 0x368);
		return fLastRenderTimeOnScreen + Tolerance >= fLastSubmitTime;
	}

};

class AFortPawn
{
public:
	USkeletalMeshComponent* Mesh()
	{ 
		if (!(FortPTR)this) return {};
		return request->read<USkeletalMeshComponent*>((FortPTR)this + offsets::Mesh);
	}

	std::unordered_map<std::string, int> GetActorFName()
	{
		if (!(FortPTR)this) return {};
		std::unordered_map<std::string, int> HashedFName = { { ActorFName(request->read<INT32>((FortPTR)this + 0x18)), 1 } };
		return HashedFName;
	}

	int64_t GetClassId(uintptr_t targetedObject)
	{
		uintptr_t targetedClass = request->read<uintptr_t>(targetedObject + 0x10);
		int64_t targetedId = request->read<int64_t>(targetedClass + 0x18);
		return targetedId;
	}

	BOOL IsA(uintptr_t targetedObject, int64_t classId) const
	{
		uintptr_t targetedClass = request->read<uintptr_t>(targetedObject + 0x10);
		int64_t targetedId = request->read<int64_t>(targetedClass + 0x18);
		if (targetedId == classId)
			return true;
		return false;
	}

	AFortAthenaVehicle* CurrentVehicle()
	{
		if (!(FortPTR)this) return {};
		return request->read<AFortAthenaVehicle*>((FortPTR)this + offsets::CurrentVehicle);
	}

	APlayerState* PlayerState()
	{
		if (!(FortPTR)this) return {};
		return request->read<APlayerState*>((FortPTR)this + offsets::PlayerState);
	}

	AFortWeapon* CurrentWeapon()
	{
		if (!(FortPTR)this) return {};
		return request->read<AFortWeapon*>((FortPTR)this + offsets::CurrentWeapon);
	}

	USceneComponent* RootComponent()
	{
		if (!(FortPTR)this) return {};
		return (USceneComponent*)request->read<uintptr_t>((FortPTR)this + offsets::RootComponent);
	}

	double player_distance(FVector PlayerLocation)
	{
		if (!(FortPTR)this) return {};
		return (this->RootComponent()->RelativeLocation().Distance(PlayerLocation) / 100.0);
	}

	bool IsInWater() 
	{
		if (!(FortPTR)this) return {};
		if (this->CurrentVehicle()->GetCurrentTireSurface() == ETireSurfaces::Water || this->CurrentVehicle()->WaterLevel() != 0.f)
			return true;
		return request->read<bool>((FortPTR)this + 0x46E0);
	}

	double Time_when_you_ll_be_able_to_splash_again() 
	{
		if (!(FortPTR)this) return {};
		return request->read<double>((FortPTR)this + 0x46E8);
	}

	void RetrivePlayerData() 
	{
		if (!(FortPTR)this) return;
		uintptr_t UCharacterMovementComponent = request->read<uintptr_t>((FortPTR)this + 0x318);
		PlayerData->MovementMode = (EMovementMode)request->read<int>(UCharacterMovementComponent + 0x201);
		PlayerData->bIsSliding = (request->read<BYTE>((FortPTR)this + 0x713) & 0x10);
		PlayerData->bWeaponHolstered = (request->read<BYTE>((FortPTR)this + 0x831) & 0x10);
		PlayerData->bMovingEmote = (request->read<BYTE>((FortPTR)this + 0x711) & 0x10);
	}
};

class APlayerController
{
public:
	AFortPawn* AcknowledgedPawn() {
		if (!(FortPTR)this) return {};
		return (AFortPawn*)request->read<uintptr_t>((FortPTR)(this + offsets::AcknowledgedPawn));
	}

	APlayerCameraManager* PlayerCameraManager() {
		if (!(FortPTR)this) return {};
		return (APlayerCameraManager*)request->read<uintptr_t>((FortPTR)this + offsets::PlayerCameraManager);
	}

	AHUD* MyHUD() {
		if (!(FortPTR)this) return {};
		return (AHUD*)request->read<uintptr_t>((FortPTR)this + offsets::MyHUD);
	}

	uintptr_t ControlRotation() {
		if (!(FortPTR)this) return {};
		return request->read<uintptr_t>((FortPTR)this + offsets::ControlRotation);
	}

	uintptr_t TargetedBuild() {
		if (!(FortPTR)this) return {};
		return request->read<uintptr_t>((FortPTR)this + offsets::TargetedBuild);
	}

	float InputPitchScale() {
		if (!(FortPTR)this) return {};
		return request->read<float>((FortPTR)this + offsets::InputYawScale);
	}

	float InputYawScale() {
		if (!(FortPTR)this) return {};
		return request->read<float>((FortPTR)this + offsets::InputPitchScale);
	}

};

class ULocalPlayer {
public:
	FVector2D W2S(FVector WorldLocation)
	{
		D3DMATRIX tempMatrix = Matrix(camera.Rotation);

		FVector vAxisX = FVector(tempMatrix.m[0][0], tempMatrix.m[0][1], tempMatrix.m[0][2]);
		FVector vAxisY = FVector(tempMatrix.m[1][0], tempMatrix.m[1][1], tempMatrix.m[1][2]);
		FVector vAxisZ = FVector(tempMatrix.m[2][0], tempMatrix.m[2][1], tempMatrix.m[2][2]);

		FVector vDelta = WorldLocation - camera.Location;
		FVector vTransformed = FVector(vDelta.Dot(vAxisY), vDelta.Dot(vAxisZ), vDelta.Dot(vAxisX));

		if (vTransformed.z < 1.f)
			vTransformed.z = 1.f;

		return FVector2D((Width / 2.0f) + vTransformed.x * (((Width / 2.0f) / tanf(camera.FieldOfView * (float)M_PI / 360.f))) / vTransformed.z, (Height / 2.0f) - vTransformed.y * (((Width / 2.0f) / tanf(camera.FieldOfView * (float)M_PI / 360.f))) / vTransformed.z);
	}

	APlayerController* PlayerController()
	{
		if (!(FortPTR)this) return {};
		return (APlayerController*)request->read<uintptr_t>((FortPTR)this + 0x30);
	}

	UFortClientSettingsRecord* ClientSettingsRecord()
	{
		if (!(FortPTR)this) return {};
		return (UFortClientSettingsRecord*)request->read<uintptr_t>((FortPTR)this + 0x390);
	}

};

class ULocalPlayers
{
public:
	ULocalPlayer* LocalPlayer()
	{
		if (!(FortPTR)this) return {};
		return (ULocalPlayer*)request->read<uintptr_t>((FortPTR)this);
	}
};

class ULevel
{
public:
	uintptr_t AWorldSettings() {
		if (!(FortPTR)this) return {};
		return request->read<uintptr_t>((FortPTR)this + offsets::WorldSettings);
	}

	uintptr_t ActorArray() {
		if (!(FortPTR)this) return {};
		return request->read<uintptr_t>((FortPTR)this + 0x98);
	}

	int ActorCount() {
		if (!(FortPTR)this) return {};
		return request->read<int>((FortPTR)this + 0xA0);
	}
};

class UGameInstance 
{
public:
	ULocalPlayers* LocalPlayers() {
		if (!(FortPTR)this) return {};
		return (ULocalPlayers*)request->read<uintptr_t>((FortPTR)this + 0x38);
	}
};

class UWorld
{
public:
	UGameInstance* GameInstance()
	{
		if (!(FortPTR)this) return {};
		return (UGameInstance*)request->read<uintptr_t>((FortPTR)this + offsets::OwningGameInstance);
	}

	ULevel* PersistentLevel()
	{
		if (!(FortPTR)this) return {};
		return (ULevel*)request->read<uintptr_t>((FortPTR)this + 0x30);
	}

	FVector GetCameraLocation()
	{
		if (!(FortPTR)this) return {};
		return request->read<FVector>(request->read<uintptr_t>((FortPTR)this + 0x110));
	}
};

namespace global
{
	UWorld* GWorld;
	ULevel* PersistentLevel;
	ULocalPlayer* LocalPlayer;
	APlayerController* PlayerController;
	AFortPawn* LocalPawn;
	std::vector<AFortPawn*> Player;
	std::vector<AFortPawn*> Item;
	std::mutex data_mutex;
	int Players;
	int Items;
}

void cache_actors() 
{
	using namespace std::chrono;
	std::vector<AFortPawn*> temp_array;
	temp_array.reserve(global::Players);
	get_camera();

	while (true)
	{
		temp_array.clear();
		global::GWorld = request->read<UWorld*>(hook::base_address + offsets::Gworld);
		debug_pointer((FortPTR)global::GWorld, "GWorld");
		global::PersistentLevel = global::GWorld->PersistentLevel();
		debug_pointer((FortPTR)global::PersistentLevel, "PersistentLevel");
		UGameInstance* Gameinstance = global::GWorld->GameInstance();
		debug_pointer((FortPTR)Gameinstance, "GameInstance");
		global::LocalPlayer = Gameinstance->LocalPlayers()->LocalPlayer();
		debug_pointer((FortPTR)global::LocalPlayer, "LocalPlayer");
		global::PlayerController = global::LocalPlayer->PlayerController();
		debug_pointer((FortPTR)global::PlayerController, "PlayerController");
		global::LocalPawn = global::PlayerController->AcknowledgedPawn();
		debug_pointer((FortPTR)global::LocalPawn, "AcknowledgedPawn");

		for (global::Players = 0; global::Players < global::PersistentLevel->ActorCount(); global::Players++)
		{
			AFortPawn* pAActors = request->read<AFortPawn*>(global::PersistentLevel->ActorArray() + global::Players * sizeof(uintptr_t));
			debug_pointer((FortPTR)pAActors, "PlayerPawn");
			if (!(FortPTR)pAActors && pAActors == global::LocalPawn)
				continue;

			if (!pAActors->PlayerState() && !pAActors->Mesh())
				continue;

			temp_array.push_back(pAActors);
		}

		UCanvas* Canvas = global::PlayerController->MyHUD()->Canvas();
		Width = Canvas->SizeX(); center_x = Width / 2;
		Height = Canvas->SizeY(); center_y = Height / 2;

		std::unique_lock<std::mutex> lock(global::data_mutex);
		global::Player = std::move(temp_array);
		lock.unlock();

		if (misc->PreformenceMode)
			std::this_thread::sleep_for(milliseconds(11));
		std::this_thread::sleep_for(milliseconds(9));
	}

}

void cache_levels()
{
	using namespace std::chrono;
	std::vector<AFortPawn*> temp_array;
	temp_array.reserve(global::Items);

	while (true)
	{
		temp_array.clear();
		uintptr_t ulevel_array = request->read<uintptr_t>((FortPTR)global::GWorld + 0x170); // iterating uworld ulevels
		std::cout << " ulevel array " << ulevel_array << std::endl;
		for (int OwningWorld = 0; OwningWorld < request->read<int>((FortPTR)global::GWorld + (0x170 + sizeof(uintptr_t))); ++OwningWorld) { // running through levels and 0x8
			if (OwningWorld >= request->read<int>((FortPTR)global::GWorld + 0x178))
				break;

			ULevel* PersistentLevel = (ULevel*)request->read<uintptr_t>(ulevel_array + sizeof(uintptr_t) * OwningWorld);
			for (int AActors = 0; AActors < request->read<int>(ulevel_array + (PersistentLevel->ActorArray() + sizeof(uintptr_t))); ++AActors) { // running through levels actor array and 0x8
				if (AActors <= PersistentLevel->ActorCount())
					break;

				global::Items = PersistentLevel->ActorCount();
				AFortPawn* pAActors = (AFortPawn*)request->read<uintptr_t>(PersistentLevel->ActorArray() + sizeof(uintptr_t) * AActors); // Item Pawn, Actor Address -> ABuildingContainer
				std::cout << " ulevel pAActors " << pAActors << std::endl;
				debug_pointer((FortPTR)pAActors, "ulevel pAActor");
				temp_array.push_back(pAActors);
			}
		}

		std::unique_lock<std::mutex> lock(global::data_mutex);
		global::Player = std::move(temp_array);
		lock.unlock();

		if (misc->PreformenceMode)
			std::this_thread::sleep_for(milliseconds(11));
		std::this_thread::sleep_for(milliseconds(9));
	}
}

camera_ get_camera()
{
	char v1; // r8
	camera = request->read<camera_>(hook::base_address + offsets::ViewPoint);
	BYTE* v2 = (BYTE*)&camera;
	int i; // edx
	__int64 result; // rax

	v1 = 0x40;
	for (i = 0; i < 0x40; ++i)
	{
		*v2 ^= v1;
		result = (unsigned int)(i + 0x17);
		v1 += i + 0x17;
		v2++;
	}

	return camera;
}

std::string GetObjectName(int key)
{
	uint32_t ChunkOffset = (uint32_t)((int)(key) >> 16);
	uint16_t NameOffset = (uint16_t)key;

	uint64_t NamePoolChunk = request->read<uint64_t>(hook::base_address + 0xeeb2c80 + (8 * ChunkOffset) + 16) + (unsigned int)(4 * NameOffset); //((ChunkOffset + 2) * 8) ERROR_NAME_SIZE_EXCEEDED
	uint16_t nameEntry = request->read<uint16_t>(NamePoolChunk);

	int nameLength = nameEntry >> 6;
	char buff[1024];
	if ((uint32_t)nameLength)
	{
		for (int x = 0; x < nameLength; ++x)
		{
			buff[x] = request->read<char>(NamePoolChunk + 4 + x);
		}
		char* v2 = buff; // rbx
		unsigned int v4 = nameLength;
		unsigned int v5; // eax
		__int64 result; // rax
		int v7; // ecx
		unsigned int v8; // kr00_4
		__int64 v9; // ecx

		v5 = 0;
		result = request->read<unsigned int>(hook::base_address + 0x569fc70) >> 5;
		if (v4)
		{
			do
			{
				v7 = *v2++;
				v8 = result ^ (16 * v7) ^ (result ^ (v7 >> 4)) & 0xF;
				result = (unsigned int)(result + 4 * v5++);
				*(v2 - 1) = v8;
			} while (v5 < v4);
		}
		buff[nameLength] = '\0';
		return std::string(buff);
	}

	return "";
}

std::string ActorFName(int key)
{
	uint32_t ChunkOffset = (uint32_t)((int)(key) >> 16);
	uint16_t NameOffset = (uint16_t)key;

	uint64_t NamePoolChunk = request->read<uint64_t>(hook::base_address + 0xeeb2c80 + (8 * ChunkOffset) + 16) + (unsigned int)(4 * NameOffset); //((ChunkOffset + 2) * 8) ERROR_NAME_SIZE_EXCEEDED //gname
	if (request->read<uint16_t>(NamePoolChunk) < 64)
	{
		auto a1 = request->read<DWORD>(NamePoolChunk + 4);
		return GetObjectName(a1);
	}
	else
	{
		return GetObjectName(key);
	}
}
#endif // SDK_HPP