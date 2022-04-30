#pragma once
#include"Components.h"
#include <Core/EngineCore.h>

class TileManager
{
public:

	TileManager();
	~TileManager();

	void GenerateGrid(int a_rows, int a_cols, float a_tileSize = 1.0f);

	void GenerateGridv2(int a_rows, int a_cols ,std::vector<std::vector<int>>* a_gridData, std::vector<float>* a_gridHeightData, float a_tileSize = 1.0f);

	void RunFloodFill();

	void ResetFloodFillCost();

	void RunFlowField();

	glm::ivec2 FindLowestCostNeighbor(const glm::ivec2& gridPos);

	glm::vec3 FindAverageDirection(const glm::ivec2& gridPos);

	bool DidGridFill();

	bool IsInGrid(int row, int col);

	bool IsTileOccupied(const entt::entity& tile);

	bool IsTileOccupiedByEnemy(const entt::entity& tile);

	const entt::entity& GetTile(int row, int col);

	const entt::entity& GetAnEnemyOccupier(const entt::entity& tile);

	const entt::entity& GetSpecificTileOccupier(const entt::entity& tile, bool isEnemy);

	void PrintFloodFillGrid();

	const float GetTileSize() const;

	glm::vec3 GridToWorld(int row, int col);

	glm::ivec2 WorldToGrid(const glm::vec3& pos);

	void SetTileOccupier(const entt::entity& tile, const entt::entity occupier);
	void RemoveTileOccupier(const entt::entity& tile, const entt::entity occupier);

	void ClearTileEntities();

	const int GetAllRows() const;
	const int GetAllCols() const;

private:

	void SmoothenFlowField();

	float m_tileSize;

	int m_allRows;
	int m_allCols;

	std::vector<entt::entity> m_tiles;

	entt::registry& m_enttReg;
	Frac::EntityRegistry& m_fracReg;	
};

