#include "TDpch.h"
#include "TileManager.h"
#include "SceneManagement/SceneManager.h"
#include <iostream>
#include <iomanip>

TileManager::TileManager() : m_enttReg(Frac::EngineCore::GetInstance().GetEnttRegistry()), m_fracReg(Frac::EngineCore::GetInstance().GetFracRegistry())
{
	m_tileSize = 1.0f;
    m_allRows = 1;
    m_allCols = 1; 
}

TileManager::~TileManager()
{

}

void TileManager::GenerateGrid(int a_rows, int a_cols, float a_tileSize)
{

	if (a_rows > 0 && a_cols > 0)
	{
        m_allRows = a_rows;
        m_allCols = a_cols;
        m_tiles.resize(a_rows * a_cols); //allocating space for all tiles

		if (a_tileSize > 0.0f)
		{
			m_tileSize = a_tileSize;
		}
        
		for (int row = 0; row < a_rows; row++)
		{
			for (int col = 0; col < a_cols; col++)
			{
                int currentTileID = (col + row * m_allCols);
                
                Frac::Entity* TileEnt = new Frac::Entity(m_fracReg, "Tile" + std::to_string(currentTileID));
                TileEnt->SetParent(Frac::EngineCore::GetInstance().GetSceneManager().GetActiveScene());

                m_fracReg.AddComponent<BoxComponent>(*TileEnt);
                m_fracReg.AddComponent<GridComponent>(*TileEnt, glm::ivec2{ row, col });
                m_fracReg.AddComponent<TOR::Transform>(*TileEnt, GridToWorld(row, col), glm::vec3{ m_tileSize, 0.0f, m_tileSize });
                m_fracReg.AddComponent<TileComponent>(*TileEnt);

                (m_tiles)[currentTileID] = TileEnt->GetHandle();
			}
		}
	}
}

void TileManager::GenerateGridv2(int a_rows, int a_cols, std::vector<std::vector<int>>* a_gridData, std::vector<float>* a_gridHeightData, float a_tileSize)
{
    Frac::Entity* ParentTile = new Frac::Entity(m_fracReg, "ParentTile");
    m_fracReg.AddComponent<TOR::Transform>(*ParentTile, glm::vec3(0,0,0) , glm::vec3(1,1,1));
    ParentTile->SetParent(Frac::EngineCore::GetInstance().GetSceneManager().GetActiveScene());

    m_allRows = a_rows;
    m_allCols = a_cols;

    if (a_rows > 0 && a_cols > 0)
    {
        m_allRows = a_rows;
        m_allCols = a_cols;
        m_tiles.resize(a_rows * a_cols); //allocating space for all tiles

        if (a_tileSize > 0.0f)
        {
            m_tileSize = a_tileSize;
        }

        for (int row = 0; row < a_rows; row++)
        {
            for (int col = 0; col < a_cols; col++)
            {
                //x = id / amounnt of rows.
                //y = id % amount of rows.
                bool isActive = true;
                bool isBuildAble = false;

                if ((*a_gridData)[col][row] < 1)
                {
                    isActive = false;
                }

                if ((*a_gridData)[col][row] == 3)
                {
                    isBuildAble = true;
                }

                int currentTileID = (col + row * m_allCols);
                int currentHeightTileID = (row + col * m_allRows);
                if (isActive == true)
                {
                    Frac::Entity& TileEnt = * new Frac::Entity(m_fracReg, "Tile" + std::to_string(currentTileID));
                    TileEnt.SetParent(*ParentTile);

                    m_fracReg.AddComponent<BoxComponent>(TileEnt);
                    m_fracReg.AddComponent<GridComponent>(TileEnt, glm::ivec2{ row, col });
                    glm::vec3 worldPos = GridToWorld(row, col);
                    worldPos.y = (*a_gridHeightData)[currentHeightTileID];
                    m_fracReg.AddComponent<TOR::Transform>(TileEnt, worldPos, glm::vec3{ m_tileSize, 0.0f, m_tileSize });
                    m_fracReg.AddComponent<TileComponent>(TileEnt);

                    TileComponent& tileComp = m_fracReg.GetComponent<TileComponent>(TileEnt);

                    for (int i = 0; i < tileComp.MaxOccupiers; i++)
                    {
                        tileComp.TileOccupiers[i] = entt::null;
                    }

                    (m_tiles)[currentTileID] = TileEnt.GetHandle();

                    if (isBuildAble)
                    {
                        m_fracReg.GetComponent<BoxComponent>(TileEnt).Color = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
                        m_fracReg.GetComponent<TileComponent>(TileEnt).States.isWalkable = false;
                        m_fracReg.GetComponent<TileComponent>(TileEnt).States.isBuildable = true;
                    }
                }
                else {
                    (m_tiles)[currentTileID] = entt::null;
                }
            }
        }
    }
}

void TileManager::RunFloodFill()
{
    // Get m_registry
    entt::registry& reg = Frac::EngineCore::GetInstance().GetEnttRegistry();

    // Consts
    const int dRow[] = { -1, 0, 1, 0 };
    const int dCol[] = { 0, 1, 0, -1 };
    const int maxSteps = 1000;

    //Find Base Entity 
    int curRow = -1;
    int curCol = -1;
    int indexCount = 0;

    reg.view<BoxComponent, GridComponent, TOR::Transform, BaseComponent>().each([this, &reg, indexCount, &curRow, &curCol](const auto entity, BoxComponent& box, GridComponent& grid, TOR::Transform& pos, BaseComponent& base)
    {
        glm::ivec2 gridpos = grid.GridPos;
        TileComponent& tile = m_enttReg.get<TileComponent>(GetTile((int)gridpos.x, (int)gridpos.y));
        
        tile.FloodFill.Cost = indexCount;
        tile.FloodFill.isChecked = true;
        curRow = gridpos.x;
        curCol = gridpos.y;
        LOGINFO("[TileManager] Base Found");
    });


    //Run Flood Fill starting from the Base's Position.
    LOGINFO("[TileManager] Flood Fill Started");
    int stepsRow = 1;
    int stepsCol = 1;
    int curStepsRow = stepsRow + curRow;
    int curStepsCol = stepsCol + curCol;
    
    bool isGridFilled = false;


    while (isGridFilled == false)
    {
        for (int x = curRow; x < curStepsRow; x++)
        {
            for (int y = curCol; y < curStepsCol; y++)
            {
                //Get Current Tile
                const entt::entity& curEntt = GetTile(x, y);  

                if (curEntt != entt::null)
                {
                    TileComponent& curTile = m_enttReg.get<TileComponent>(curEntt);

                    const bool isWalkable = curTile.States.isWalkable;
                    const bool isChecked = curTile.FloodFill.isChecked;
                    const int floodCost = curTile.FloodFill.Cost;

                    //Is Current Tile Accessible?
                    if ((isWalkable) && (isChecked) && (floodCost == indexCount))
                    {
                        // Check neighbors
                        for (int a = 0; a < 4; a++)
                        {
                            int adjx = x + dRow[a];
                            int adjy = y + dCol[a];

                            int adjID = (adjy + adjx * m_allCols);

                            if (adjID < m_allRows * m_allCols && IsInGrid(adjx, adjy))
                            {
                                const entt::entity& nextEntt = GetTile(adjx, adjy);
                                if (nextEntt != entt::null)
                                {
                                    TileComponent& nextTile = m_enttReg.get<TileComponent>(nextEntt);

                                    if ((nextTile.States.isActive) && (nextTile.FloodFill.isChecked == false) && (nextTile.States.isWalkable) && !(IsTileOccupied(nextEntt)))
                                    {
                                        nextTile.FloodFill.isChecked = true;
                                        nextTile.FloodFill.Cost = indexCount + 1;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        indexCount++;
        stepsRow += 2;
        stepsCol += 2;
        if (stepsRow > m_allRows) stepsRow = m_allRows;
        if (stepsCol > m_allCols) stepsCol = m_allCols;

        if (curRow != 0) curRow--;
        if (curCol != 0) curCol--;

        curStepsRow = stepsRow + curRow;
        curStepsCol = stepsCol + curCol;

        if (curStepsRow > m_allRows) curStepsRow = m_allRows;
        if (curStepsCol > m_allCols) curStepsCol = m_allCols;

        //Stops when all Tiles have been reached
        isGridFilled = DidGridFill();
        if (indexCount >= maxSteps)
        {
            LOGERROR("[TileManager] Flood Fill Cannot Reach Target");
            isGridFilled = true;
        }
    }
    LOGINFO("[TileManager] Flood Fill Completed");
}

void TileManager::ResetFloodFillCost()
{
    entt::registry& reg = Frac::EngineCore::GetInstance().GetEnttRegistry();

    reg.view<TileComponent>().each([](TileComponent& tile)
        {
            if (tile.States.isActive && tile.FloodFill.Cost != 9999)
            {
                tile.FloodFill.Cost = 9999;
                tile.FloodFill.isChecked = false;
            }           
        });
}

void TileManager::RunFlowField()
{
    LOGINFO("Flow Field Started");

    for (int x = 0; x < m_allRows; x++)
    {
        for (int y = 0; y < m_allCols; y++)
        {
            const entt::entity& curTileEnt = GetTile(x, y);

            //Does the Tile exist?
            if (curTileEnt != entt::null)
            {
                TileComponent& curTileComp = m_enttReg.get<TileComponent>(curTileEnt);
                
                //Find Lowest Cost Neighbor
                if (curTileComp.States.isActive && curTileComp.States.isWalkable && !IsTileOccupied(curTileEnt))
                {
                    const TOR::Transform& curTransComp = m_enttReg.get<TOR::Transform>(curTileEnt);
                    const GridComponent& curGridComp = m_enttReg.get<GridComponent>(curTileEnt);

                    const glm::ivec2& targetGridPos = FindLowestCostNeighbor(curGridComp.GridPos);
                    if (targetGridPos != curGridComp.GridPos)
                    {
                        const glm::vec3& targetWorldPos = GridToWorld(targetGridPos.x, targetGridPos.y);

                        //Set Flow Field Direction
                        glm::vec3 direction = { targetWorldPos.x - curTransComp.Position.x , 0.0f, targetWorldPos.z - curTransComp.Position.z };
                        curTileComp.FloodFill.FlowDirection = glm::normalize(direction);
                    }
                }
            }

        }
    }

    //SmoothenFlowField();
    LOGINFO("Flow Field Completed");
}

glm::ivec2 TileManager::FindLowestCostNeighbor(const glm::ivec2& gridPos)
{

    int bestCost = 99999;
    glm::ivec2 bestPosition = { -1,-1 };

    //8 Neighbors
    const int dCol[] = { 1, 1, 1, 0, 0, 0, -1, -1, -1 };
    const int dRow[] = { -1, 0, 1, -1, 0, 1, -1, 0, 1 };

    //4 Neighbors
    //const int dRow[] = { -1, 0, 1, 0 };
    //const int dCol[] = { 0, 1, 0, -1 };

    // Check neighbors
    for (int i = 0; i < 8; i++)
    {
        int adjx = gridPos.x + dRow[i];
        int adjy = gridPos.y + dCol[i];

        int adjID = (adjy + adjx * m_allCols);

        //Is the Neighbor in the Grid?
        if (adjID < m_allRows * m_allCols && IsInGrid(adjx, adjy))
        {
            const entt::entity& curTileEnt = GetTile(adjx, adjy);

            //Does the Tile exist?
            if (curTileEnt != entt::null)
            {
                const TileComponent& curTileComp = m_enttReg.get<TileComponent>(curTileEnt);

                if (curTileComp.FloodFill.Cost <= bestCost)
                {
                    bestCost = curTileComp.FloodFill.Cost;
                    bestPosition = { adjx, adjy };
                }
            }
        }
    }

    if (bestPosition == glm::ivec2(-1))
    {
        LOGWARNING("[Tile Manager] Flow Field tried to access invalid Tile");
        return gridPos;
    }

    return glm::vec2(bestPosition);
}

glm::vec3 TileManager::FindAverageDirection(const glm::ivec2& gridPos)
{

        //Get current Tile
        const entt::entity& curTileEntt = GetTile(gridPos.x, gridPos.y);
        const TileComponent& curTileComp = m_enttReg.get<TileComponent>(curTileEntt);

        //8 Neighbors
        const int dCol[] = { 1, 1, 1, 0, 0, 0, -1, -1, -1 };
        const int dRow[] = { -1, 0, 1, -1, 0, 1, -1, 0, 1 };

        //Average
        glm::vec3 averageDirection = curTileComp.FloodFill.FlowDirection;

        //Check neighbors
        for (int i = 0; i < 8; i++)
        {
            int adjx = gridPos.x + dRow[i];
            int adjy = gridPos.y + dCol[i];

            int adjID = (adjy + adjx * m_allCols);

            //Is the Neighbor in the Grid?
            if (adjID < m_allRows * m_allCols && IsInGrid(adjx, adjy))
            {
                const entt::entity& nextTileEntt = GetTile(adjx, adjy);

                if (nextTileEntt != entt::null)
                {
                    TileComponent& nextTileComp = m_enttReg.get<TileComponent>(nextTileEntt);

                    if ((nextTileComp.States.isActive) && (nextTileComp.States.isWalkable))
                    {
                        averageDirection += nextTileComp.FloodFill.FlowDirection;
                    }
                    else
                    {
                        averageDirection += glm::vec3(-dRow[i], 0.0f, -dCol[i]); //Vector Pointing to our current Tile
                    }
                }
                else
                {
                    averageDirection += glm::vec3(-dRow[i], 0.0f, -dCol[i]);    //Vector Pointing to our current Tile
                }
            }
        }

        return glm::normalize(averageDirection);
    
}


bool TileManager::DidGridFill()
{
    bool result = true;

    m_enttReg.view<TileComponent>().each([this, &result](auto entity, TileComponent& tile)
        {
            if (tile.States.isActive && tile.States.isWalkable && tile.FloodFill.Cost == 9999 && !IsTileOccupied(entity))
            {
                result = false;
            }
        });

    return result;
}

bool TileManager::IsInGrid(int row, int col)
{
    if ((row < 0) || (row >= m_allRows) || (col < 0) || (col >= m_allCols))
    {
        return false;
    }
    return true;
}

const entt::entity& TileManager::GetTile(int row, int col)
{
    assert(IsInGrid(row, col) && "no tile can be returned");
    int currentTileID = (col + row * m_allCols);
    return (m_tiles)[currentTileID];
}

const entt::entity& TileManager::GetAnEnemyOccupier(const entt::entity& tile)
{
    if (m_enttReg.any_of<TileComponent>(tile))
    {
        const TileComponent& tileComp = m_enttReg.get<TileComponent>(tile);

        //Get one Occupier
        for (int i = 0; i < tileComp.MaxOccupiers; i++)
        {           
            if (m_enttReg.valid(tileComp.TileOccupiers[i]) && m_enttReg.any_of<EnemyComponent>(tileComp.TileOccupiers[i]))
            {
                return tileComp.TileOccupiers[i];
            }
        } 

        return entt::null;
    }
    LOGERROR("[Tile Manager] Tried to get an Occupier from an empty Tile");
    return entt::null;
}

const entt::entity& TileManager::GetSpecificTileOccupier(const entt::entity& tile, bool isEnemy)
{
    if (m_enttReg.any_of<TileComponent>(tile))
    {
        const TileComponent& tileComp = m_enttReg.get<TileComponent>(tile);

        if (isEnemy)
        {
            //Get first Enemy in vector
            for (int i = 0; i < tileComp.MaxOccupiers; i++)
            {
                if (m_enttReg.valid(tileComp.TileOccupiers[i]) && m_enttReg.any_of<EnemyComponent>(tileComp.TileOccupiers[i]))
                {
                    return tileComp.TileOccupiers[i];
                }
            }
        }
        else //Turret
        {
            //Get first Turret in vector
            for (int i = 0; i < tileComp.MaxOccupiers; i++)
            {
                if (m_enttReg.valid(tileComp.TileOccupiers[i]) && m_enttReg.any_of<TurretComponent>(tileComp.TileOccupiers[i]))
                {
                    return tileComp.TileOccupiers[i];
                }
            }
        }

        return entt::null;
    }
    LOGERROR("[Tile Manager] Tried to get Specific Occupier from an empty Tile");
    return entt::null;
}

void TileManager::PrintFloodFillGrid()
{
    std::cout << "Flood Fill: " << std::endl;

    for (int i = 0; i < m_allRows; i++)
    {
        for (int k = 0; k < m_allCols; k++)
        {
            const entt::entity& curEntt = GetTile(i, k);
            if (curEntt != entt::null)
            {
                const TileComponent& curTile = m_enttReg.get<TileComponent>(curEntt);
                std::cout << std::setw(5) << curTile.FloodFill.Cost;
            }
            else
            {
                std::cout << std::setw(5) << "-";
            }
        }
        std::cout << std::endl;
    }
}

bool TileManager::IsTileOccupied(const entt::entity& tile)
{
    if (m_enttReg.any_of<TileComponent>(tile))
    {
        const TileComponent& tileComp = m_enttReg.get<TileComponent>(tile);

        //Make sure it isn't the Base
        if (tileComp.FloodFill.Cost != 0)
        {
            //Check if there is at least one Occupier in the vector
            for (int i = 0; i < tileComp.MaxOccupiers; i++)
            {
                if (tileComp.TileOccupiers[i] != entt::null)
                {
                    return true;
                }
            }
        }
        return false;
    }

    LOGWARNING("Tried to access a non-tile Entity");
    return true;
}

bool TileManager::IsTileOccupiedByEnemy(const entt::entity& tile)
{
    if (m_enttReg.any_of<TileComponent>(tile))
    {
        const TileComponent& tileComp = m_enttReg.get<TileComponent>(tile);

        //Make sure it isn't the Base
        if (tileComp.FloodFill.Cost != 0)
        {
            //Check if there is at least one Occupier in the vector
            for (int i = 0; i < tileComp.MaxOccupiers; i++)
            {
                if (m_enttReg.valid(tileComp.TileOccupiers[i]) && m_enttReg.any_of<EnemyComponent>(tileComp.TileOccupiers[i]))
                {
                    return true;
                }
            }
        }
        return false;
    }

    LOGWARNING("Tried to access a non-tile Entity");
    return true;
}

const float TileManager::GetTileSize() const
{
    return m_tileSize;
}

glm::vec3 TileManager::GridToWorld(int row, int col)
{
    return glm::vec3{ ((col * m_tileSize) + (m_tileSize / 2.0f)), 0.0f ,-((row * m_tileSize) + (m_tileSize / 2.0f)) };
}


glm::ivec2 TileManager::WorldToGrid(const glm::vec3& pos)
{
    glm::ivec2 gridPos = (glm::ivec2(-(pos.z / m_tileSize), (pos.x / m_tileSize)));
    if ((gridPos.x < 0) || (gridPos.x >= m_allRows) || (gridPos.y < 0) || (gridPos.y >= m_allCols))
    {
        //LOGERROR("[Tile Manager] WorldToGrid invalid Position: %f, %f, %f", pos.x, pos.y, pos.z);
        return glm::ivec2(-1);
    }
    return gridPos;
}

void TileManager::ClearTileEntities()
{
    //Destroy Tiles
    m_enttReg.view<TileComponent>().each([this](const auto entity, TileComponent& tileComp)
        {
            m_enttReg.destroy(entity);
    });
}

const int TileManager::GetAllRows() const
{
    return m_allRows;
}

const int TileManager::GetAllCols() const
{
    return m_allCols;
}

void TileManager::SmoothenFlowField()
{
    for (int x = 0; x < m_allRows; x++)
    {
        for (int y = 0; y < m_allCols; y++)
        {
            const entt::entity& curEntity = GetTile(x, y);

            if (curEntity != entt::null)
            {
                TileComponent& curTile = m_enttReg.get<TileComponent>(curEntity);

                if (curTile.States.isActive && curTile.States.isWalkable && !IsTileOccupied(curEntity))
                {
                    curTile.FloodFill.FlowDirection = FindAverageDirection(glm::ivec2(x, y));
                }
            }
        }
    }
}

void TileManager::SetTileOccupier(const entt::entity& tile, const entt::entity occupier)
{
    if (m_enttReg.valid(tile) && m_enttReg.any_of<TileComponent>(tile))
    {
        TileComponent& tileComp = m_enttReg.get<TileComponent>(tile);
        bool shouldAddEntity = true;

        //Make sure it isn't the Base
        if (tileComp.FloodFill.Cost != 0)
        {

            //Check if Occupier is already in the vector
            for (int i = 0; i < tileComp.MaxOccupiers; i++)
            {
                if (tileComp.TileOccupiers[i] == occupier)
                {
                    shouldAddEntity = false;
                    break;
                }
                    
            }
            if (shouldAddEntity)
            {
                for (int i = 0; i < tileComp.MaxOccupiers; i++)
                {
                    if (tileComp.TileOccupiers[i] == entt::null)
                    {
                        tileComp.TileOccupiers[i] = occupier;
                        //Used for Debugging
                        BoxComponent& boxComp = m_enttReg.get<BoxComponent>(tile);
                        boxComp.Color = glm::vec4{ 0.0f, 1.0f, 0.0f, 1.0f };
                        break;
                    }
                }
            }
        }
    }
}

void TileManager::RemoveTileOccupier(const entt::entity& tile, const entt::entity occupier)
{
    if (m_enttReg.valid(tile) && m_enttReg.any_of<TileComponent>(tile))
    {
        TileComponent& tileComp = m_enttReg.get<TileComponent>(tile);

        //Make sure it isn't the Base
        if (tileComp.FloodFill.Cost != 0)
        {

            //Find and Remove Occupier
            for (int i = 0; i < tileComp.MaxOccupiers; i++)
            {
                if (tileComp.TileOccupiers[i] == occupier)
                {
                    tileComp.TileOccupiers[i] = entt::null;
                    //Used for Debugging
                    BoxComponent& boxComp = m_enttReg.get<BoxComponent>(tile);
                    boxComp.Color = glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f };
                    break;
                }

            }
        }
    }
}


