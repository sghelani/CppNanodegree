#ifndef ROUTE_PLANNER_H
#define ROUTE_PLANNER_H

#include <iostream>
#include <vector>
#include <string>
#include <queue>
#include "route_model.h"

class RoutePlanner
{
public:
  class CompareNodes
  {
  public:
    bool operator()(RouteModel::Node *node1, RouteModel::Node *node2);
  };
  RoutePlanner(RouteModel &model, float start_x, float start_y, float end_x, float end_y);
  // Add public variables or methods declarations here.
  float GetDistance() const { return distance; }
  float CalculateDistance(std::vector<RouteModel::Node> path);
  void AStarSearch();

  // The following methods have been made public so we can test them individually.
  void AddNeighbors(RouteModel::Node *current_node);
  float CalculateHValue(RouteModel::Node const *node);
  std::vector<RouteModel::Node> ConstructFinalPath(RouteModel::Node *);
  RouteModel::Node *NextNode();

private:
  // Add private variables or methods declarations here.

  std::priority_queue<RouteModel::Node *, std::vector<RouteModel::Node *>, CompareNodes> open_queue;

  RouteModel::Node *start_node;
  RouteModel::Node *end_node;

  float distance = 0.0f;
  RouteModel &m_Model;
};

#endif