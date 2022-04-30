#pragma once
/*
* Headers
* TODO: Replace with PCH
*/
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtx/quaternion.hpp>
#include <entt/entt.hpp>


/*
* Enemy type
* This is used to define different enemy types.
*/
enum class EEnemyType
{
	Slow = 0,
	Normal,
	Fast
};

/*
* Turret type
* This is used to define different Turret types.
*/
enum class ETurretType
{
	Sniper = 0,
	Machinegun,
	RocketLauncher
};

/*
* Projectile type
* This is used to define different Projectile types.
*/
enum class EProjectileType
{
	Bullet = 0,
	Missile
};

/*
* Box Component
* This is used to draw Debug Lines around Entities.
* @Color -> The color of the Box.
* @isEnabled -> If true, the Box will be rendered, else not. 
*/
struct BoxComponent
{
	glm::vec4 Color = { 1.f, 1.f, 1.f, 1.f };
	bool isEnabled = true;
};

/*
* Position Component
* This is a temporary Component which will be replaced by the Transform later on.
* @GridPos -> Holds the Grid Position of the Entity.
* @WorldPos -> Holds the World Position of the Entity.
*/
struct GridComponent
{
	glm::ivec2 GridPos = { -1,-1 };
};

/*
* Tile States Data
* This is a specific Component that only Tiles should have.
* @isActive -> If true, this is an active Tile (seen by Player).
* @isWalkable -> If true, AI can walk on this Tile.
* @isBuildable -> If true, Player can Build on this Tile.
* @isClickable -> If true, when hovering this Tile we can see its info.
* @isLocked -> If true, this Tile is Locked by an AI.
*/
struct TileStatesData
{
	bool isActive = true;
	bool isWalkable = true;
	bool isBuildable = false;
	bool isClickable = true;
};

/*
* Flood-Fill Data
* This is a specific Component that only Tiles should have.
* @isChecked -> If true, this Tile has been assigned a cost from the FF Algorithm.
* @Cost -> The Flood Fill cost of this specific Tile.
* @FlowDirection -> This value represents the Flow Field Direction vector.
*/
struct FloodFillData
{
	bool isChecked = false;
	int Cost = 9999;
	glm::vec3 FlowDirection = { 0.0f, 0.0f, 0.0f };
};

/*
* Tile Component
* This is a specific Component that only Tiles should have. 
* It holds Tile States and Flood Fill Data values.
*/
struct TileComponent
{
	static const int MaxOccupiers = 5;
	TileStatesData States;
	FloodFillData FloodFill;
	entt::entity TileOccupiers[MaxOccupiers];
};

/*
* Health Component
* This is a specific Component that can be attached to every entity that needs health logic.
* @Health -> The damage that this enemies does to our Base.
* @MaxHealth -> Patience Value used for finding different routes to the Base.
*/
struct HealthComponent
{
	float Health;
	float MaxHealth;
};

/*
* Enemy Data
* This is a specific Component that only Enemies should have.
* @AlignmentRadius -> Radius used for Allignment with Flock
* @SeparationRadius -> Radius used for Seperation with Flock
* @CohesionRadius -> Radius used for Cohesion with Flock
* @AlignmentMultiplier -> Multiplier used for the Allignment Vector (Sets Magnitute) 
* @SeparationMultiplier -> Multiplier used for the Seperation Vector (Sets Magnitute) 
* @CohesionMultiplier -> Multiplier used for the Cohesion Vector (Sets Magnitute) 
* @MaxSpeed -> Speed of the Enemy. Set in Velocity Component
* @MaxForce -> This is used after we add the 3 forces together. Set Magnitute of Acceleration/Flocking Force.
* @Acceleration -> The final pushing force that steers the enemy.
*/
struct EnemyFlocking
{
	float AlignmentRadius;
	float SeparationRadius;
	float CohesionRadius;	
	float AlignmentMultiplier;	
	float SeparationMultiplier;	
	float CohesionMultiplier;	
	float MaxSpeed;
	float MaxForce;				
	glm::vec3 Acceleration = { 0.0f, 0.0f, 0.0f };
};

/*
* Enemy Data
* This is a specific Component that only Enemies should have.
* @EEnemyType -> Enum of Enemy Type
* @DamageToBase -> The damage that this enemies does to our Base.
* @ResourcesWorth-> How many resources this enemy is worth.
* @RotationSpeed -> float that indicates the rotation speed when moving to targets.
* @MaxHealth -> Max Health of Enemy.
* @Size -> Size of Enemy
* @Model -> The GLTF model the Enemy uses for its visuals.
* @PreviousPosition -> Previous Grid Position.
* @PreviousDirection -> This is the previous Direction of the Enemy which is used once it hits a wall.
*/
struct EnemyData
{
	EEnemyType Type;
	int DamageToBase;	
	int ResourcesWorth;
	float RotationSpeed;
	float MaxHealth;
	glm::vec3 Size;
	std::string Model;
	glm::ivec2 PreviousPosition = { -1, -1 };
	glm::vec3 PreviousDirection = { 0.0f, 0.0f, 0.0f };
};

/*
* Enemy States
* This is a specific Component that only Enemies should have.
* @didCollide -> If true, the Enemy has hit a wall/object.
*/
struct EnemyStates
{
	bool didCollide = false;
};

/*
* Enemy Component
* This is a specific Component that only Enemies should have.
* It holds the Enemy States and Enemy Data values.
*/
struct EnemyComponent
{
	EnemyData Data;
	EnemyStates States;
	EnemyFlocking Flocking;
};

/*
* Velocity Component
* This is a specific Component that only entities that need a velocity should have.
* @Speed -> float value that indicates the speed amount.
*/
struct VelocityComponent
{
	float Speed = 1.0f;
};

/*
* Direction Component
* This is a specific Component that only entities that need a direction should have.
* @Direction -> vector 3 that indicates the direction using x, y and z axis.
*/
struct DirectionComponent
{
	glm::vec3 Direction = {0.0f, 0.0f, 0.0f};
};

/*
* Data that is used to instantiate the Turrets.
* @Type: The specific type of Turret (eg. TurretType::Machinegun).
* @SpawnSize: The size of the Turret, used by the animation system to 'grow' to that size.
* @Range: The range (in tiles) the Turret can reach to shoot enemies.
* @FireDelay: (in seconds) how long between each Shooting of a projectile.
* @TurretRotationSpeed: Used by the targeting system, how fast the Turret can rotate towards the target.
* @ResourceWorth: How expensive this turret costs to build.
* @Model: The GLTF model the Turret uses for its visuals.
*/
struct TurretStats
{
	ETurretType Type;
	glm::vec3 SpawnSize;
	int Range;
	float FireDelay;
	float TurretRotationSpeed;

	int ResourcesWorth;

	std::string Model;
	float FireDelayRandomness = 0;
};

/*
* Data that is used to instantiate the Projectiles.
* @Type: The specific type of Project (eg. ProjectileType::Bullet).
* @Damage: The amount of damage this Projectile deals to enemies.
* @Speed: The velocity speed of the Projectile when fired.
* @Model: The GLTF model the Bullet uses for its visuals.
*/
struct ProjectileStats
{
	EProjectileType Type;
	int Damage;
	float Speed;
	std::string Model;
};

/*
* Turret Component
* This is a specific Component that only entities that are a turret should have.
* @hasTarget -> boolean that indicates if the turret has a target.
* @Target -> vector 3 that indicates the target position.
* @Range -> integer that indicates the fire range of the turret.
* @Firerate -> float that indicates the time betweet shooting a single bullet.
* @TimeElapsed -> Float that is a local time elapsed variable.
* @TurretRotationSpeed -> float that indicates the rotating speed of the turret.
* @TargetHeightOffset -> float that indicates the offset height when targeting a specific position.
*/
struct TurretComponent
{
	TurretStats Stats;
	bool hasTarget = false;
	entt::entity Target = entt::null;
	float TimeElapsed = 0.0f;

	float TargetHeightOffset;

	bool DespawnState = false;
	bool SpawnState = true;
	bool FirstShot = true;
};

/*
* Bullet Component
* This is a specific Component that only entities that are bullets should have.
*/
struct BulletComponent
{
	ProjectileStats Stats;
	entt::entity Target;
};

/*
* Resource Component
* @ResourcesWorth -> This value holds the amount of resources the Player receives when an Entity is destroyed. 
*/
struct ResourceComponent
{
	int ResourcesWorth = 2;
};

/*
* Wave Data
* @Enemies -> All of the enemies that this spawner will spawn in a consecutive order.
* @SpawnDelays -> All of the delays between spawns that indicate after how many seconds the next enemy will spawn.
*/
struct WaveData
{
	std::vector<EEnemyType> Enemies;
	std::vector<float> SpawnDelays;
};
/*
* Spawner stats
* @Waves -> A collection of Wave Data of the current Level (enemies and their spawn delays).
*/
struct SpawnerStats
{
	std::vector<WaveData> Waves;
};
/*
* Spawner Component
* @Stats -> The wave data
* @TimeElapsed -> Keeps track of how much time it passed from the previous spawned enemy.
* @ToSpawn -> The amount of enemies left to spawn on this spawner.
*/
struct SpawnerComponent
{
	SpawnerStats Stats;
	float TimeElapsed = 0.0f;
	int ToSpawn = 0;
};

/*
* Base Component
* This is a specific Component that only entities that are a base should have.
* @dummyvalue -> integer that acts as a dummy variable. Components without variable are not recognized.
*/
struct BaseComponent
{
	int dummyvalue; //TODO replace with usable variable
};

struct HeathBarComponent
{
	bool invertLoadDirection = false;
	glm::vec3 emptyColor = { 1.f,0.f,0.f };
	glm::vec3 fullColor = { 0.f,1.f,0.f };
	glm::vec3 ComponentSize{ 0.006f,0.0005f,0.001f };
	bool overrideScale = false;
};

struct DeathComponent
{
	float Duration{ 1.5f };
	glm::quat deathRotation{ glm::radians(glm::vec3{0.f,0.f,180.f}) };
};

/**
 * \brief
 * @duration is the time of the animation
 * @endScale is the scale the entity should be at the end of the animation
 * @beginScale is the scale on the beginning
 * @timer is the time in of the animation
 */
struct SpawnComponent
{
	float duration{ 1.f };
	glm::vec3 endScale{ 1.f };
	glm::vec3 beginScale{ 0.f };
	float timer{ 0.f };
};

/*
* Remove Self Component
* Component that can be added on any entity that will remove the entity after an X amount of seconds.
* @Duration -> After how many seconds after initialization this entity will remove itself.
* @Timer -> Keeps track of the time since initialization.
*/
struct RemoveSelfComponent
{
	float Duration{ 5.0f };
	float Timer{ 0.0f };
};
