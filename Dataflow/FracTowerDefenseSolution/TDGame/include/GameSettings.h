#pragma once
/// <summary>
/// This file contains variables used by gameplay in various places.
/// </summary>

#include "Components.h"

namespace GameSettings
{
	//Fixed Delta Time in order for the game to still play normally when we scale the window.
	static const float FixedDT = 0.1f;

	/// <summary>
	/// Turrets & Projectile stats
	/// </summary>

	static const TurretStats Sniper
	{
		ETurretType::Sniper,	//Type
		glm::vec3(0.3f),		//SpawnSize
		10,						//Range
		3.0f,					//FireDelay
		5.0f,					//TurretRotationSpeed
		12,						//ResourcesWorth
		"Models/Turrets2.0/Basic_turret.gltf",//Model
	};
	static const ProjectileStats SniperProjectile
	{
		EProjectileType::Bullet,		//Type
		75,								//Damage
		60.0f,							//Speed
		"Models/Projectiles/Bullet.gltf"//Model
	};
	
	static const TurretStats Machinegun
	{
		ETurretType::Machinegun,//Type
		glm::vec3(0.3f),		//SpawnSize
		5,						//Range
		0.2f,					//FireDelay
		5.0f,					//TurretRotationSpeed
		16,						//ResourcesWorth
		"Models/Turrets2.0/Laser_Turret.gltf",//Model
	};
	static const ProjectileStats MachinegunProjectile
	{
		EProjectileType::Bullet,		//Type
		10,								//Damage
		20.0f,							//Speed
		"Models/Projectiles/Bullet.gltf"//Model
	};
	
	static const TurretStats RocketLauncher
	{
		ETurretType::RocketLauncher,//Type
		glm::vec3(0.3f),		//SpawnSize
		8,						//Range
		2.5f,					//FireDelay
		5.0f,					//TurretRotationSpeed
		24,						//ResourcesWorth
		"Models/Turrets2.0/Launcher_turret.gltf",//Model
	};
	static const ProjectileStats RocketLauncherProjectile
	{
		EProjectileType::Missile,		//Type
		40,								//Damage
		30.0f,							//Speed
		"Models/Projectiles/Rocket.gltf"//Model
	};
	static const int RocketLauncherProjectileRange = 2;

	/// <summary>
	/// Enemy Data & Flocking Stats
	/// </summary>

	//Enemy Data
	static const EnemyData SlowEnemyData
	{
		EEnemyType::Slow,						//Type
		20,										//Damage to Base
		4,										//ResourcesWorth
		3.0f,									//Rotation Speed
		550.0f,									//Max Health
		{2.5f, 2.5f, 2.5f},						//Size
		"Models/Enemies/Enemy_Flamer.gltf"		//Model	
	};

	static const EnemyData NormalEnemyData
	{
		EEnemyType::Normal,						//Type
		10,										//Damage to Base
		4,										//ResourcesWorth
		4.0f,									//Rotation Speed
		250.0f,									//Max Health
		{2.5f, 2.5f, 2.5f},						//Size
		"Models/Enemies/Enemy_MG.gltf"			//Model	
	};

	static const EnemyData FastEnemyData
	{
		EEnemyType::Fast,						//Type
		5,										//Damage to Base
		2,										//ResourcesWorth
		5.0f,									//Rotation Speed
		150.0f,									//Max Health
		{1.5f, 1.5f, 1.5f},						//Size
		"Models/Enemies/Enemy_TeslaGun.gltf"	//Model	
	};

	//Flocking
	static const EnemyFlocking SlowEnemyFlock
	{
		1.0f,	//AlignmentRadius
		2.5f,	//SeparationRadius
		2.0f,	//CohesionRadius
		0.1f,	//AlignmentMultiplier
		1.0f,	//SeparationMultiplier
		0.2f,	//CohesionMultiplier
		1.0f,	//MaxSpeed
		0.33f,	//MaxForce (Should be lower than MaxSpeed)
	};

	static const EnemyFlocking NormalEnemyFlock
	{
		1.0f,	//AlignmentRadius
		1.0f,	//SeparationRadius
		2.0f,	//CohesionRadius
		0.1f,	//AlignmentMultiplier
		1.0f,	//SeparationMultiplier
		0.5f,	//CohesionMultiplier
		2.0f,	//MaxSpeed
		0.66f,	//MaxForce (Should be lower than MaxSpeed)
	};

	static const EnemyFlocking FastEnemyFlock
	{
		1.0f,	//AlignmentRadius
		1.0f,	//SeparationRadius
		3.0f,	//CohesionRadius
		0.1f,	//AlignmentMultiplier
		0.4f,	//SeparationMultiplier
		0.5f,	//CohesionMultiplier
		5.0f,	//MaxSpeed
		1.5f,	//MaxForce (Should be lower than MaxSpeed)
	};

	//Helper function to retrieve TurretStats with an index for loops
	static const TurretStats& IndexToTurretStats(int index)
	{
		switch (ETurretType(index))
		{
		case ETurretType::Sniper:
			return Sniper;
			break;
		case ETurretType::Machinegun:
			return Machinegun;
			break;
		case ETurretType::RocketLauncher:
			return RocketLauncher;
			break;
		}
	}

	//Helper function to retrieve EnemyData with an index for loops
	static const EnemyData& IndexToEnemyData(int index)
	{
		switch (EEnemyType(index))
		{
		case EEnemyType::Slow:
			return SlowEnemyData;
			break;
		case EEnemyType::Normal:
			return NormalEnemyData;
			break;
		case EEnemyType::Fast:
			return FastEnemyData;
			break;
		}
	}

	//Helper function to retrieve EnemyFlocking with an index for loops
	static const EnemyFlocking& IndexToEnemyFlocking(int index)
	{
		switch (EEnemyType(index))
		{
		case EEnemyType::Slow:
			return SlowEnemyFlock;
			break;
		case EEnemyType::Normal:
			return NormalEnemyFlock;
			break;
		case EEnemyType::Fast:
			return FastEnemyFlock;
			break;
		}
	}

	/// <summary>
	/// Wave & Spawner stats
	/// </summary>
	static const SpawnerStats Level1SpawnerStats //Level 1 has 3 Spawners
	{
		{
			//Wave 1
			{{EEnemyType::Normal}, //Enemies
			{0.0f}},			   //SpawnDelays

			//Wave 2
			{{EEnemyType::Fast}, //Enemies
			{0.0f}},			 //SpawnDelays

			//Wave 3
			{{EEnemyType::Slow},//Enemies
			{0.0f}},			//SpawnDelays

			//Wave 4
			{{EEnemyType::Normal, EEnemyType::Normal}, //Enemies
			{0.0f,				  3.0f}},			     //SpawnDelays

			//Wave 5
			{{EEnemyType::Slow, EEnemyType::Fast}, //Enemies
			{0.0f,			    2.0f}}			 //SpawnDelays
		}
	};
	static const SpawnerStats Level2SpawnerStats //Level 2 has 4 Spawners
	{
		{
			//Wave 1
			{{EEnemyType::Normal, EEnemyType::Normal}, //Enemies
			{0.0f,				  3.0f}},			   //SpawnDelays

			//Wave 2
			{{EEnemyType::Fast, EEnemyType::Fast}, //Enemies
			{0.0f,              4.0f}},			 //SpawnDelays

			//Wave 3
			{{EEnemyType::Slow, EEnemyType::Normal},//Enemies
			{0.0f,              4.0f}},			//SpawnDelays

			//Wave 4
			{{EEnemyType::Normal, EEnemyType::Fast, EEnemyType::Fast}, //Enemies
			{0.0f,				  3.0f,             2.0f}},			     //SpawnDelays

			//Wave 5
			{{EEnemyType::Fast, EEnemyType::Normal, EEnemyType::Normal}, //Enemies
			{0.0f,			    1.0f,			    1.0f}}			 //SpawnDelays
		}
	};
	static const SpawnerStats Level3SpawnerStats //Level 3 has 4 Spawners
	{
		{
			//Wave 1
			{{EEnemyType::Fast, EEnemyType::Normal}, //Enemies
			{0.0f,				 1.0f}},			//SpawnDelays

			//Wave 2
			{{EEnemyType::Normal, EEnemyType::Normal, EEnemyType::Slow}, //Enemies
			{0.0f,			    3.0f,			  1.0f}},			//SpawnDelays

			//Wave 3
			{{EEnemyType::Slow, EEnemyType::Slow, EEnemyType::Slow}, //Enemies
			{0.0f,			   3.0f,             3.0f}},			  //SpawnDelays

			//Wave 4
			{{EEnemyType::Fast, EEnemyType::Fast, EEnemyType::Fast, EEnemyType::Fast}, //Enemies
			{0.0f,				0.0f,				0.0f,			0.0f}},			    //SpawnDelays

			//Wave 5
			{{EEnemyType::Normal, EEnemyType::Fast, EEnemyType::Slow}, //Enemies
			{0.0f,			    1.5f,			   1.5f}}		     //SpawnDelays
		}
	};
	static const SpawnerStats Level4SpawnerStats //Level 4 has 8 Spawners
	{
		{
			//Wave 1
			{{EEnemyType::Normal, EEnemyType::Slow}, //Enemies
			{0.0f,              0.0f}},             //SpawnDelays

			//Wave 2
			{{EEnemyType::Normal, EEnemyType::Slow, EEnemyType::Fast, EEnemyType::Fast}, //Enemies
			{0.0f,			    1.0f,            1.0f,              1.0f}},			  //SpawnDelays

			//Wave 3
			{{EEnemyType::Slow, EEnemyType::Slow, EEnemyType::Slow, EEnemyType::Slow}, //Enemies
			{0.0f,			   0.5f,            1.0f,            1.0f}},			 //SpawnDelays

			//Wave 4
			{{EEnemyType::Fast, EEnemyType::Fast, EEnemyType::Fast, EEnemyType::Fast}, //Enemies
			{0.0f,				 0.5f,              1.5f,              0.5f}},			 //SpawnDelays

			//Wave 5
			{{EEnemyType::Normal, EEnemyType::Normal, EEnemyType::Slow, EEnemyType::Slow, EEnemyType::Slow, EEnemyType::Slow}, //Enemies
			{0.0f,			    0.5f,			  0.5f,            0.5f,            0.5f,            0.5f}}		     //SpawnDelays
		}
	};
	static const SpawnerStats Level5SpawnerStats //Level 5 has 10 Spawners
	{
		{
			//Wave 1
			{{EEnemyType::Slow}, //Enemies
			{0.0f}},             //SpawnDelays

			//Wave 2
			{{EEnemyType::Normal, EEnemyType::Slow}, //Enemies
			{0.0f,			      3.0f}},		   //SpawnDelays

			//Wave 3
			{{EEnemyType::Slow, EEnemyType::Fast, EEnemyType::Normal}, //Enemies
			{0.0f,			    5.0f,             2.0f}},			 //SpawnDelays

			//Wave 4
			{{EEnemyType::Fast, EEnemyType::Fast, EEnemyType::Slow}, //Enemies
			{0.0f,				 0.0f,            1.5f}},			 //SpawnDelays

			//Wave 5
			{{EEnemyType::Normal, EEnemyType::Normal, EEnemyType::Slow, EEnemyType::Fast, EEnemyType::Slow}, //Enemies
			{0.0f,			    0.5f,			  0.5f,					3.0f,			  0.0f}}		     //SpawnDelays
		}
	};
	static const SpawnerStats Level6SpawnerStats //Level 6 has 9 Spawners
	{
		{
			//Wave 1
			{{EEnemyType::Fast, EEnemyType::Normal}, //Enemies
			{0.0f,              2.5f}},             //SpawnDelays

			//Wave 2
			{{EEnemyType::Slow, EEnemyType::Normal}, //Enemies
			{0.0f,			    6.0f}},			  //SpawnDelays

			//Wave 3
			{{EEnemyType::Slow, EEnemyType::Fast, EEnemyType::Fast}, //Enemies
			{0.0f,			    5.0f,             3.0f}},			 //SpawnDelays

			//Wave 4
			{{EEnemyType::Normal, EEnemyType::Normal, EEnemyType::Normal, EEnemyType::Normal}, //Enemies
			{0.0f,				  2.0f,               2.0f,               2.0f}},			 //SpawnDelays

			//Wave 5
			{{EEnemyType::Fast, EEnemyType::Fast, EEnemyType::Fast, EEnemyType::Slow, EEnemyType::Fast, EEnemyType::Fast, EEnemyType::Fast}, //Enemies
			{0.0f,			    0.0f,			  0.0f,             0.0f,             5.0f,             0.0f,			  0.0f}}		     //SpawnDelays
		}
	};
}