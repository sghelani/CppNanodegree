#include "route_planner.h"
#include <algorithm>

/* This comparator will be used two compare two nodes in the heap */
bool RoutePlanner::CompareNodes::operator()(RouteModel::Node *node1, RouteModel::Node *node2)
{
    return (node1->g_value + node1->h_value) > (node2->g_value + node2->h_value);
}

RoutePlanner::RoutePlanner(RouteModel &model, float start_x, float start_y, float end_x, float end_y) : m_Model(model)
{
    // Convert inputs to percentage:
    start_x *= 0.01;
    start_y *= 0.01;
    end_x *= 0.01;
    end_y *= 0.01;
    // Store the nodes you find in the RoutePlanner's start_node and end_node attributes.
    start_node = &m_Model.FindClosestNode(start_x, start_y);
    end_node = &m_Model.FindClosestNode(end_x, end_y);
}

float RoutePlanner::CalculateHValue(RouteModel::Node const *node)
{
    return node->distance(*(end_node));
}

void RoutePlanner::AddNeighbors(RouteModel::Node *current_node)
{
    current_node->FindNeighbors();
    for (auto *node : current_node->neighbors)
    {
        node->parent = current_node;
        node->h_value = CalculateHValue(node);
        node->g_value = current_node->g_value + node->distance(*current_node);
        node->visited = true;
        open_queue.push(node);
    }
}

RouteModel::Node *RoutePlanner::NextNode()
{
    /* The top node of the heap will have minimum g + h value */
    RouteModel::Node *firstNode = open_queue.top();
    open_queue.pop();
    return firstNode;
}

std::vector<RouteModel::Node> RoutePlanner::ConstructFinalPath(RouteModel::Node *current_node)
{

    distance = 0.0f;
    std::vector<RouteModel::Node> path_found;
    /* Iteratively traversing the path from the end to start node */
    while (current_node != nullptr)
    {
        path_found.emplace_back(*current_node);
        current_node = current_node->parent;
    }
    std::reverse(path_found.begin(), path_found.end());
    return path_found;
}

float RoutePlanner::CalculateDistance(std::vector<RouteModel::Node> path)
{
    float newDistance = 0.0f;
    for (int i = path.size() - 1; i >= 1; i--)
    {
        newDistance += path[i].distance(*(path[i].parent));
    }
    // Multiply the distance by the scale of the map to get meters.
    return newDistance * m_Model.MetricScale();
}

void RoutePlanner::AStarSearch()
{

    RouteModel::Node *current_node = nullptr;
    start_node->visited = true;
    RoutePlanner::open_queue.push(start_node);
    std::vector<RouteModel::Node> path;
    /* keeping iterating till the queue has nodes to process */
    while (!open_queue.empty())
    {
        current_node = NextNode();
        if (current_node == end_node)
        {
            path = ConstructFinalPath(end_node);
            distance = CalculateDistance(path);
            m_Model.path = path;
            break;
        }
        AddNeighbors(current_node);
    }
}