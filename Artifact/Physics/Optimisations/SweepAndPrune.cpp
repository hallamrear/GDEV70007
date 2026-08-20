#include "pch.h"
#include "SweepAndPrune.h"

SweepAndPrune::SweepAndPrune() : BroadPhase()
{
    m_Intervals = std::deque<Interval>();
}

SweepAndPrune::~SweepAndPrune()
{

}

void SweepAndPrune::InsertionSort()
{
    for (int i = 1; i < m_Intervals.size(); i++)
    {
        Interval& keyPair = m_Intervals[i];
        int j = i - 1;

        while (j >= 0 && m_Intervals[j].Value > keyPair.Value)
        {
            m_Intervals[j + 1] = m_Intervals[j];
            j = j - 1;
        }

        m_Intervals[j + 1] = keyPair;
    }
}


#include <World/Entity.h>
#include <Physics/Colliders/AABBCollider.h>

bool SweepAndPrune::DetermineCollisionPairs(std::vector<std::pair<Entity*, Entity*>>& collisionPairs)
{
    if (m_NeedsUpdate)
    {
        collisionPairs.clear();

        //std::map<const Entity*, std::vector<
        //
        //for (int i = 0; i < m_Intervals.size() - 1; i++)
        //{
        //    int j = i + 1;
        //
        //    if (m_Intervals[i].IsMin)
        //    {
        //
        //    }
        //}


    }

    return true;
}

bool SweepAndPrune::AddObject(Entity* entity)
{
    //Sorting on the X axis
    int chosenAxis = 0;

    glm::vec3 origin = { entity->GetPosition().x, entity->GetPosition().y, entity->GetPosition().z };
    glm::vec3 extents = { entity->GetCollider()->GetBoundingVolumeExtents().x, entity->GetCollider()->GetBoundingVolumeExtents().y, entity->GetCollider()->GetBoundingVolumeExtents().z };

    Interval max;
    max.Object = entity;
    max.IsMin = true;
    max.Value = origin[chosenAxis] + extents[chosenAxis];

    Interval min;
    min.Object = entity;
    min.IsMin = true;
    min.Value = origin[chosenAxis] - extents[chosenAxis];

    m_Intervals.push_back(min);
    m_Intervals.push_back(max);

    InsertionSort();

    return true;
}

void SweepAndPrune::RenderIMGUIDetails()
{

}

void SweepAndPrune::Render(Renderer& renderer)
{
    UNREFERENCED_PARAMETER(renderer);
}

std::string SweepAndPrune::GetBroadPhaseStatsString()
{
    std::string str;


    return str;
}