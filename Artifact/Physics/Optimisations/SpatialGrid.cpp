#include "pch.h"
#include "SpatialGrid.h"
#include <World/Entity.h>
#include <Physics/Colliders/AABBCollider.h>
#include <Rendering/IMGUIIncludes.h>
#include <System/AssetManagement.h>
#include <System/AssetLoader.h>

ModelRef SpatialGrid::m_GridCellModel = nullptr;

SpatialGrid::SpatialGrid()
{
    m_GridMap = GridStorageType();
    m_CollisionPairsThisFrame = 0;
    
    if (m_GridCellModel == nullptr)
    {
        m_GridCellModel = ServiceLocator::Locate<AssetManager>()->GetModel("Colliders//BoxCollider.glb");
    }
}

SpatialGrid::~SpatialGrid()
{
    for (auto& itr : m_GridMap)
    {
        for (size_t i = 0; i < itr.second.size(); i++)
        {
            delete itr.second[i];
            itr.second[i] = nullptr;
        }

        itr.second.clear();
    }

    m_GridMap.clear();

    m_GridCellModel = nullptr;
}

bool SpatialGrid::DetermineCollisionPairs(std::vector<std::pair<Entity*, Entity*>>& collisionPairs)
{
    m_CollisionPairsThisFrame = 0;

    for (auto& bucket : m_GridMap)
    {
        if (bucket.second.size() > 1)
        {
            for (size_t i = 0; i < bucket.second.size(); i++)
            {
                for (size_t j = i + 1; j < bucket.second.size(); j++)
                {
                    collisionPairs.push_back({ bucket.second[i], bucket.second[j] });
                }
            }
        }
    }

    m_CollisionPairsThisFrame = (int)collisionPairs.size();

    return true;
}

bool SpatialGrid::AddObject(Entity* entity)
{
    if (entity->GetCollider() == nullptr)
    {
        return false;
    }

    glm::vec3 origin = { entity->GetPosition().x, entity->GetPosition().y, entity->GetPosition().z };
    glm::vec3 extents = { entity->GetCollider()->GetBoundingVolumeExtents().x, entity->GetCollider()->GetBoundingVolumeExtents().y, entity->GetCollider()->GetBoundingVolumeExtents().z };

    glm::vec3 max = { origin + extents };
    glm::vec3 min = { origin - extents };

    GridIndex maxIndex = GetIndexFromPosition(max);
    GridIndex minIndex = GetIndexFromPosition(min);

    if (minIndex == maxIndex)
    {
        m_GridMap[minIndex].push_back(entity);
        printf("EQEQ %i %i %i - %f %f %f\n", minIndex.X, minIndex.Y, minIndex.Z, origin.x, origin.y, origin.z);
    }
    else
    {
        GridIndex index;
        int diffX = maxIndex.X - minIndex.X;
        int diffY = maxIndex.Y - minIndex.Y;
        int diffZ = maxIndex.Z - minIndex.Z;
        printf("DIFFS %i %i %i\n", diffX, diffY, diffZ);
        for (int z = 0; z < diffZ + 1; z++)
        {
            for (int y = 0; y < diffY + 1; y++)
            {
                for (int x = 0; x < diffX + 1; x++)
                {
                    index.X = minIndex.X + x;
                    index.Y = minIndex.Y + y;
                    index.Z = minIndex.Z + z;
                    m_GridMap[index].push_back(entity);
                    printf("DIFF %i %i %i - %f %f %f\n", index.X, index.Y, index.Z, origin.x, origin.y, origin.z);
                }
            }
        }
    }

    return true;
}

GridIndex SpatialGrid::GetIndexFromPosition(const glm::vec3& position)
{
    GridIndex index;
    float ooCellSize = (1.0f / c_GridCellSize);
    index.X = (int)(std::floorf(position.x * ooCellSize));
    index.Y = (int)(std::floorf(position.y * ooCellSize));
    index.Z = (int)(std::floorf(position.z * ooCellSize));
    return index;
}

void SpatialGrid::RenderIMGUIDetails()
{
    ImGui::Text("%f units per grid cell.\n", c_GridCellSize);
    ImGui::Text("%i grid cells in use.\n", m_GridMap.size());
    ImGui::Text("%i broadphase collision pairs found vs. bruteforce's 499500\n", m_CollisionPairsThisFrame);
}

void SpatialGrid::Render(Renderer& renderer)
{
    Matrix4x4 cellMatrix = IdentityMatrix;
    float offset = (c_GridCellSize * 0.5f);
    renderer.SetDebugDrawMode();

    for (auto& cell : m_GridMap)
    {
        if (cell.second.size() <= 1)
            continue;

        Vector3 position = {};
        position.x = (cell.first.X * c_GridCellSize) + offset;
        position.y = (cell.first.Y * c_GridCellSize) + offset;
        position.z = (cell.first.Z * c_GridCellSize) + offset;

        DirectX::XMStoreFloat4x4(&cellMatrix,
            DirectX::XMMatrixScaling(c_GridCellSize, c_GridCellSize, c_GridCellSize) *
            DirectX::XMMatrixTranslation(position.x, position.y, position.z));

        renderer.Render(m_GridCellModel, cellMatrix);
    }
}
