#include "ai.hpp"
#include "percepts.hpp"
#include "vec2.hpp"
#include <cstddef>
#include <cstdint>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

// HELPERS

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

  WorldModel() { exploredWorld.insert({currentCoords, {1, "o"}}); }

  void ExploreWorld(Percepts &percepts) {

    // only add visit if after step you land in the same node, me thinks
    auto it = exploredWorld.find(currentCoords);
    if (it != exploredWorld.end()) {
      int exploredX = it->first.x;
      int exploredY = it->first.y;

      if (exploredX == currentCoords.x && exploredY == currentCoords.y) {
        it->second.visits++;
        it->second.cellType = percepts.current[0];
      }

    } else
      exploredWorld.insert({currentCoords, {0, percepts.current[0]}});

    switch (orientation) {
    case Up:
      for (int i = 0; i < percepts.forward.size(); i++) {
        exploredWorld.insert({{currentCoords.x, currentCoords.y + i + 1},
                              {0, percepts.forward[i]}});
      };
      for (int i = 0; i < percepts.left.size(); i++) {
        exploredWorld.insert({{currentCoords.x - i - 1, currentCoords.y},
                              {0, percepts.left[i]}});
      };
      for (int i = 0; i < percepts.right.size(); i++) {
        exploredWorld.insert({{currentCoords.x + i + 1, currentCoords.y},
                              {0, percepts.right[i]}});
      };
      break;

    case Down:
      for (int i = 0; i < percepts.forward.size(); i++) {
        exploredWorld.insert({{currentCoords.x, currentCoords.y - i - 1},
                              {0, percepts.forward[i]}});
      };
      for (int i = 0; i < percepts.left.size(); i++) {
        exploredWorld.insert({{currentCoords.x + i + 1, currentCoords.y},
                              {0, percepts.left[i]}});
      };
      for (int i = 0; i < percepts.right.size(); i++) {
        exploredWorld.insert({{currentCoords.x - i - 1, currentCoords.y},
                              {0, percepts.right[i]}});
      };
      break;
    case Left:
      for (int i = 0; i < percepts.forward.size(); i++) {
        exploredWorld.insert({{currentCoords.x - i - 1, currentCoords.y},
                              {0, percepts.forward[i]}});
      };
      for (int i = 0; i < percepts.left.size(); i++) {
        exploredWorld.insert({{currentCoords.x, currentCoords.y - i - 1},
                              {0, percepts.left[i]}});
      };
      for (int i = 0; i < percepts.right.size(); i++) {
        exploredWorld.insert({{currentCoords.x, currentCoords.y + i + 1},
                              {0, percepts.right[i]}});
      };
      break;
    case Right:
      for (int i = 0; i < percepts.forward.size(); i++) {
        exploredWorld.insert({{currentCoords.x + i + 1, currentCoords.y},
                              {0, percepts.forward[i]}});
      };
      for (int i = 0; i < percepts.left.size(); i++) {
        exploredWorld.insert({{currentCoords.x, currentCoords.y + i + 1},
                              {0, percepts.left[i]}});
      };
      for (int i = 0; i < percepts.right.size(); i++) {
        exploredWorld.insert({{currentCoords.x, currentCoords.y - i - 1},
                              {0, percepts.right[i]}});
      };
      break;
    }
  }

  Vec2 UpdateMinMaxCoords(Vec2 coords) {
    if (coords.x < minX)
      minX = coords.x;
    if (coords.x > maxX)
      maxX = coords.x;
    if (coords.y < minY)
      minY = coords.y;
    if (coords.y > maxY)
      maxY = coords.y;

    return coords;
  };

  std::vector<std::string> UpdateCoords(std::vector<std::string> cmd) {
    if (cmd.empty())
      return {};

    std::string currCmd = cmd[0];

    switch (orientation) {
    case Up:
      if (currCmd == "F")
        currentCoords =
            UpdateMinMaxCoords({currentCoords.x, currentCoords.y + 1});
      else if (currCmd == "B")
        currentCoords =
            UpdateMinMaxCoords({currentCoords.x, currentCoords.y - 1});
      else if (currCmd == "L")
        orientation = Left;
      else if (currCmd == "R")
        orientation = Right;
      break;
    case Down:
      if (currCmd == "F")
        currentCoords =
            UpdateMinMaxCoords({currentCoords.x, currentCoords.y - 1});
      else if (currCmd == "B")
        currentCoords =
            UpdateMinMaxCoords({currentCoords.x, currentCoords.y + 1});
      else if (currCmd == "L")
        orientation = Right;
      else if (currCmd == "R")
        orientation = Left;
      break;
    case Left:
      if (currCmd == "F")
        currentCoords =
            UpdateMinMaxCoords({currentCoords.x - 1, currentCoords.y});
      else if (currCmd == "B")
        currentCoords =
            UpdateMinMaxCoords({currentCoords.x + 1, currentCoords.y});
      else if (currCmd == "L")
        orientation = Down;
      else if (currCmd == "R")
        orientation = Up;
      break;
    case Right:
      if (currCmd == "F")
        currentCoords =
            UpdateMinMaxCoords({currentCoords.x + 1, currentCoords.y});
      else if (currCmd == "B")
        currentCoords =
            UpdateMinMaxCoords({currentCoords.x - 1, currentCoords.y});
      else if (currCmd == "L")
        orientation = Up;
      else if (currCmd == "R")
        orientation = Down;
      break;
    }

    return cmd;
  }

  void PrintCoords() {
    std::cout << "Current agent coords: ";
    std::cout << currentCoords.x;
    std::cout << " ";
    std::cout << currentCoords.y << "\n";

    std::cout << "Current orientation: ";
    std::cout << orientation << "\n";

    std::cout << "Explored World size: ";
    std::cout << exploredWorld.bucket_count() << ", " << exploredWorld.size()
              << "\n";
  }

  void PrintExploredMap() {
    std::string heatmapShade = " .:-=+*#%@";
    std::string explored = "+";
    std::string unexplored = "_";
    int beautifyExpand = 10;

    std::cout << "\n\n";

    for (int y = maxY + beautifyExpand; y >= minY - beautifyExpand; y--) {
      for (int x = minX - beautifyExpand; x <= maxX + beautifyExpand; x++) {
        auto it = exploredWorld.find({x, y});
        if (it == exploredWorld.end()) {
          std::cout << unexplored;
          continue;
        }
        if (it->second.visits > 9) {
          std::cout << it->second.cellType;
        } else {
          std::cout << heatmapShade[it->second.visits];
        }
      }
      std::cout << "\n";
    }

    std::cout << "\n\n";
  }
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

  std::string OppositeCmd(std::string cmd) {

    if (cmd == MoveForward) {
      return MoveBackward;

    } else if (cmd == MoveBackward) {
      return MoveForward;

    } else if (cmd == RotateLeft) {
      return RotateRight;

    } else if (cmd == RotateRight) {
      return RotateLeft;
    }

    return "";
  };

  void PrintCmdQueue(std::queue<std::string> q) {
    if (q.empty() || q.size() < 0)
      return;

    while (!q.empty()) {
      std::cout << q.front() + " ";
      q.pop();
    }

    std::cout << std::endl;
  }

public:
  CommandHandler() {}
  CommandHandler(WorldModel worldModel) { this->worldModel = worldModel; }

  // general agent status
  AgentMode agentMode = AgentMode::Idle;
  WorldModel worldModel;

  // bomb stuff
  int mineDetector;
  int noDisarmCmds;
  std::string lastCmd;
  bool hasStartedDefusing = false;

  // exploration stuff

  std::vector<std::string> defuse_cmd_sequence = // seq is 8 cmds in total;
      { // net positive point defusal of bombs
          DisarmTrap,  RotateLeft, DisarmTrap,  RotateRight,
          RotateRight, DisarmTrap, RotateRight, DisarmTrap};

  std::vector<std::string> random_seq = {
      RotateRight, MoveBackward, MoveBackward, MoveBackward, RotateRight,
      MoveForward, MoveForward,  MoveForward,  MoveForward,
  };

  void ClearQueue() { cmd_queue = {}; };

  void QueueCommandGroup(std::vector<std::string> cmd_group) {
    for (auto it = cmd_group.begin(); it != cmd_group.end(); it++) {
      cmd_queue.push(*it);
    };
  }

  void QueueFrontCommandGroup(std::vector<std::string> cmd_group) {
    std::queue<std::string> old_queue = cmd_queue;

    cmd_queue = {};

    QueueCommandGroup(cmd_group);

    while (!old_queue.empty()) {
      cmd_queue.push(old_queue.front());
      old_queue.pop();
    }
  }

  void ClearDisarmQueue() {
    int cmdsToClear = 8 - noDisarmCmds;

    if (cmd_queue.empty())
      return;

    for (int i = 0; i < cmdsToClear; i++) {
      cmd_queue.pop();
      if (cmd_queue.empty())
        continue;
    }

    noDisarmCmds = 0;
    std::cout << "return to original course; last cmd: " + lastCmd << "\n";
    if (!lastCmd.empty())
      QueueFrontCommandGroup({OppositeCmd(lastCmd)});

    hasStartedDefusing = false;
    lastCmd = "";
  }

  void DefuseMine() { // Start defusing in all directions one by one;

    if (hasStartedDefusing)
      return;

    hasStartedDefusing = true;

    return QueueFrontCommandGroup(defuse_cmd_sequence);
  }

  void EvalPercepts(const Percepts &percepts) {}

  void DetermineNextMove(const Percepts &percepts,
                         std::vector<std::string> &exploreSeq) {
    // I want to determine the next best move, by seeing my percepts and what I have explored and then
    // based on that select the next cell that i have least explored - if two are the same, then the first one
    // so the calculation would be the amount of points it would cost divided by the amount of visits (cost/visits);
    // biggest number of all directions wins and that's the way the agent goes

    
  }

  void Explore(const Percepts &percepts) {
    // I want to explore based on the percepts I have;
    // i want my next move to be determined based on the amount of points i lose
    // if they are all the same,
    auto exploredWorld = worldModel.exploredWorld;
    auto orient = worldModel.orientation;
    int currX = worldModel.currentCoords.x;
    int currY = worldModel.currentCoords.y;

    std::vector<std::string> exploreSeq;

    switch (orient) {
    case Up:
      for (int i = 0; i <= percepts.forward.size(); i++) {
        if (exploredWorld.count({currX, currY + i})) {
          continue;
        } else {
          DetermineNextMove(percepts, exploreSeq);
        }
      }
      break;
    case Down:
      for (int i = 1; i <= percepts.forward.size(); i++) {
        if (exploredWorld.count({currX, currY - i})) {
          continue;
        } else {
          DetermineNextMove(percepts, exploreSeq);
          return;
        }
      }
      break;
    case Left:
      for (int i = 1; i <= percepts.forward.size(); i++) {
        if (exploredWorld.count({currX - i, currY})) {
          continue;
        } else {
          DetermineNextMove(percepts, exploreSeq);
          return;
        }
      }
      break;
    case Right:
      for (int i = 1; i <= percepts.forward.size(); i++) {
        if (exploredWorld.count({currX + i, currY})) {
          continue;
        } else {
          DetermineNextMove(percepts, exploreSeq);
          return;
        }
      }
      break;
    }

    QueueCommandGroup(exploreSeq);
  }

  std::vector<std::string> ExecuteCommand(const Percepts &percepts) {

    // if current detector is not the same as what was,
    // clear the queue, the mine has been defused.
    if (percepts.detector != mineDetector && agentMode == AgentMode::Defusing) {
      ClearDisarmQueue();

      // set agent in idle state
      agentMode = AgentMode::Idle;
      mineDetector = percepts.detector;
    }
    if (percepts.detector == 1) {

      // set agent in defuse state
      mineDetector = percepts.detector;
      agentMode = AgentMode::Defusing;
    }

    switch (agentMode) {
    case AgentMode::Idle:
      if (cmd_queue.empty())
        if (cmd_queue.empty())
          Explore(percepts);

      break;

    case AgentMode::Defusing:

      DefuseMine(); // todo don't add if it's already there
      break;

    case AgentMode::Exploring:

      break;

    default:
      return {};
    };

    if (cmd_queue.empty()) {
      agentMode = AgentMode::Idle;
      return {};
    }

    std::string curr_cmd = cmd_queue.front();
    cmd_queue.pop();

    if (agentMode == AgentMode::Defusing) {
      noDisarmCmds++;
      if (curr_cmd != DisarmTrap)
        lastCmd = curr_cmd;
    }

    std::cout << curr_cmd << "\n\n";

    PrintCmdQueue(cmd_queue);

    return {curr_cmd};
  };
};

/***************************************************************
AI CLASS DEFINITION
*/
AI::AI() {}
AI::AI(unsigned id, unsigned agent_speed, std::mt19937_64 *rng, Symbols symbols,
       Costs costs, int max_turn)
    : id(id), agent_speed(agent_speed), rng(rng), symbols(symbols),
      costs(costs), max_turn(max_turn) {}

WorldModel worldModel = WorldModel();
CommandHandler *cmds = new CommandHandler(worldModel);

// PRINT
void AI::PrintPercepts(const Percepts &percepts) {
  std::cout << "DISTANCE: " << percepts.detector << std::endl;
  std::cout << "CURRENT:  " << percepts.current[0] << std::endl;
  std::cout << "FORWARD:  ";
  for (std::vector<std::string>::const_iterator it = percepts.forward.begin();
       it != percepts.forward.end(); it++)
    std::cout << *it << " ";
  std::cout << std::endl;
  std::cout << "LEFT:     ";
  for (std::vector<std::string>::const_iterator it = percepts.left.begin();
       it != percepts.left.end(); it++)
    std::cout << *it << " ";
  std::cout << std::endl;
  std::cout << "BACKWARD: ";
  for (std::vector<std::string>::const_iterator it = percepts.backward.begin();
       it != percepts.backward.end(); it++)
    std::cout << *it << " ";
  std::cout << std::endl;
  std::cout << "RIGHT:    ";
  for (std::vector<std::string>::const_iterator it = percepts.right.begin();
       it != percepts.right.end(); it++)
    std::cout << *it << " ";
  std::cout << std::endl;
  std::cout << "Others:\n";
  for (size_t i = 0; i < percepts.others.size(); i++) {
    std::cout << "   " << i << ": " << percepts.others[i].to_string()
              << std::endl;
  }
}

// ########################## RUN ##########################

std::vector<std::string> AI::Run(Percepts &percepts, AgentComm *comms) {
  PrintPercepts(percepts);
  worldModel.PrintCoords();
  worldModel.PrintExploredMap();

  worldModel.ExploreWorld(percepts);

  return worldModel.UpdateCoords(cmds->ExecuteCommand(percepts));
  // return (cmds->ExecuteCommand(percepts));
};
