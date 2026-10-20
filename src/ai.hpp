#pragma once

#include "comm.hpp"
#include "percepts.hpp"
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <random>
#include <string>
#include <vector>

enum AgentMode { Defusing = 0, Exploring, Idle };

enum Orientation { Up = 0, Down, Left, Right };

struct Cells {
  const std::string Wall = "w";
  const std::string OpenSpace = "o";
  const std::string Start = "@";
  const std::string Trap = "^";
  const std::string ExplodedTrap = "#";
  const std::string DisarmedTrap = "+";
  const std::string Treasure = "$";

  const std::string TeleporterOneOpen = "[";
  const std::string TeleporterTwoOpen = "{";
  const std::string TeleporterThreeOpen = "(";

  const std::string TeleporterOneClosed = "]";
  const std::string TeleporterTwoClosed = "}";
  const std::string TeleporterThreeClosed = ")";
};

struct Vec2Hash {
  std::size_t operator()(const Vec2 &vec2) const {
    uint64_t h = (uint64_t(uint32_t(vec2.x)) << 32) | uint32_t(vec2.y);
    h ^= h >> 30;
    h *= 0xbf58476d1ce4e5b9; // split mix values
    h ^= h >> 27;
    h *= 0x94d049bb133111eb; // split mix values
    h ^= h >> 31;
    return h;

    return h;
  }
};

struct Cell {
  int visits;
  std::string cellType;
};

class WorldModel {
private:
  int minX = 0;
  int maxX = 0;
  int minY = 0;
  int maxY = 0;

public:
  std::unordered_map<Vec2, Cell, Vec2Hash> exploredWorld;
  Vec2 currentCoords = {0, 0};
  Orientation orientation = Up; // Up = 0, Down = 1, Left = 2, Right = 3

  WorldModel();

  void ExploreWorld(Percepts &percepts);

  Vec2 UpdateMinMaxCoords(Vec2 coords);
  std::vector<std::string> UpdateCoords(std::vector<std::string> cmd);

  // DEBUG + PRINT
  void PrintCoords();
  void PrintExploredMap();
};

class CommandHandler {

private:
  const std::string MoveForward = "F";
  const std::string MoveBackward = "B";
  const std::string RotateLeft = "L";
  const std::string RotateRight = "R";
  const std::string UseTeleporter = "U";
  const std::string CollectTreasure = "T";
  const std::string DisarmTrap = "D";

  std::queue<std::string> cmd_queue;

public:
  CommandHandler();
  CommandHandler(WorldModel worldModel);

  AgentMode agentMode = AgentMode::Idle;
  WorldModel worldModel;

  int mineDetector;
  int noDisarmCmds;
  std::string lastCmd;
  bool hasStartedDefusing = false;

  std::vector<std::string> defuse_cmd_sequence = // seq is 8 cmds in total;
      { // net positive point defusal of bombs
          DisarmTrap,  RotateLeft, DisarmTrap,  RotateRight,
          RotateRight, DisarmTrap, RotateRight, DisarmTrap};

  std::vector<std::string> random_seq = {
      RotateRight, MoveBackward, MoveBackward, MoveBackward, RotateRight,
      MoveForward, MoveForward,  MoveForward,  MoveForward,
  };

  std::string OppositeCmd(std::string cmd);

  // Queue stuff

  void ClearQueue();
  void QueueCommandGroup(std::vector<std::string> cmd_group);
  void QueueFrontCommandGroup(std::vector<std::string> cmd_group);
  void ClearDisarmQueue();

  void DefuseMine();

  void Explore(const Percepts &percepts);
  
  void PrintCmdQueue(std::queue<std::string> q);

  std::vector<std::string> ExecuteCommand(const Percepts &percepts);
};

class AI {
protected:
  // Necessary, do not delete.
  unsigned id;
  unsigned agent_speed;
  std::mt19937_64 *rng;
  Symbols symbols;
  Costs costs;
  int max_turn;

  WorldModel worldModel;
  CommandHandler cmds{worldModel};

public:
  AI();
  AI(unsigned id, unsigned agent_speed, std::mt19937_64 *rng, Symbols symbols,
     Costs costs, int max_turn);
  void PrintPercepts(const Percepts &percepts);
  std::vector<std::string> Run(Percepts &percepts, AgentComm *comms);
};
