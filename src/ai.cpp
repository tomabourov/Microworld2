#include "ai.hpp"
#include "percepts.hpp"
#include "vec2.hpp"
#include <cstddef>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

// ############# WORLD MODEL #############

WorldModel::WorldModel() { exploredWorld[currentCoords] = {1, "o"}; };

void WorldModel::ExploreWorld(Percepts &percepts) {

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
    exploredWorld[currentCoords] = {0, percepts.current[0]};

  switch (orientation) {
  case Up:
    for (int i = 0; i < percepts.forward.size(); i++) {
      exploredWorld[{currentCoords.x, currentCoords.y + i + 1}] = {
          0, percepts.forward[i]};
    };
    for (int i = 0; i < percepts.left.size(); i++) {
      exploredWorld[{currentCoords.x - i - 1, currentCoords.y}] = {
          0, percepts.left[i]};
    };
    for (int i = 0; i < percepts.right.size(); i++) {
      exploredWorld[{currentCoords.x + i + 1, currentCoords.y}] = {
          0, percepts.right[i]};
    };
    break;
  case Down:
    for (int i = 0; i < percepts.forward.size(); i++) {
      exploredWorld[{currentCoords.x, currentCoords.y - i - 1}] = {
          0, percepts.forward[i]};
    };
    for (int i = 0; i < percepts.left.size(); i++) {
      exploredWorld[{currentCoords.x + i + 1, currentCoords.y}] = {
          0, percepts.left[i]};
    };
    for (int i = 0; i < percepts.right.size(); i++) {
      exploredWorld[{currentCoords.x - i - 1, currentCoords.y}] = {
          0, percepts.right[i]};
    };
    break;
  case Left:
    for (int i = 0; i < percepts.forward.size(); i++) {
      exploredWorld[{currentCoords.x - i - 1, currentCoords.y}] = {
          0, percepts.forward[i]};
    };
    for (int i = 0; i < percepts.left.size(); i++) {
      exploredWorld[{currentCoords.x, currentCoords.y - i - 1}] = {
          0, percepts.left[i]};
    };
    for (int i = 0; i < percepts.right.size(); i++) {
      exploredWorld[{currentCoords.x, currentCoords.y + i + 1}] = {
          0, percepts.right[i]};
    };
    break;
  case Right:
    for (int i = 0; i < percepts.forward.size(); i++) {
      exploredWorld[{currentCoords.x + i + 1, currentCoords.y}] = {
          0, percepts.forward[i]};
    };
    for (int i = 0; i < percepts.left.size(); i++) {
      exploredWorld[{currentCoords.x, currentCoords.y + i + 1}] = {
          0, percepts.left[i]};
    };
    for (int i = 0; i < percepts.right.size(); i++) {
      exploredWorld[{currentCoords.x, currentCoords.y - i - 1}] = {
          0, percepts.right[i]};
    };
    break;
  }
};

Vec2 WorldModel::UpdateMinMaxCoords(Vec2 coords) {
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
std::vector<std::string>
WorldModel::UpdateCoords(std::vector<std::string> cmd) {
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

void WorldModel::PrintCoords() {
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
void WorldModel::PrintExploredMap() {
  std::string heatmapShade2 = " .:-=+*#%@";
  std::string heatmapShade = "0123456789";
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
      if (it->second.visits > heatmapShade.size()) {
        std::cout << it->second.cellType;
      } else {
        std::cout << heatmapShade[it->second.visits];
      }
    }
    std::cout << "\n";
  }

  std::cout << "\n\n";
}

// ############# COMMAND HANDLER #############

// Queue stuff & helpers

void CommandHandler::ClearQueue() { cmd_queue = {}; };
void CommandHandler::QueueCommandGroup(std::vector<std::string> cmd_group) {
  for (auto it = cmd_group.begin(); it != cmd_group.end(); it++) {
    cmd_queue.push(*it);
  };
}
void CommandHandler::QueueFrontCommandGroup(
    std::vector<std::string> cmd_group) {
  std::queue<std::string> old_queue = cmd_queue;

  cmd_queue = {};

  QueueCommandGroup(cmd_group);

  while (!old_queue.empty()) {
    cmd_queue.push(old_queue.front());
    old_queue.pop();
  }
}
void CommandHandler::ClearDisarmQueue() {
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

void CommandHandler::DefuseMine() {
  // Start defusing in all directions one by one;

  if (hasStartedDefusing)
    return;

  hasStartedDefusing = true;

  return QueueFrontCommandGroup(defuse_cmd_sequence);
}

struct CellChoice {
  int x;
  int y;
  float cellScore;
};

static int Manhattan(Vec2 a, Vec2 b) {
  return std::abs(a.x - b.x) +
         std::abs(a.y -
                  b.y); // abs so direction doesn't matter, only step count
}

// Helper: is this cell a wall we have already seen?
static bool IsWall(const WorldModel &wm, Vec2 p) {
  auto it = wm.exploredWorld.find(p); // look the cell up in our map
  return it != wm.exploredWorld.end() &&
         it->second.cellType == "w"; // wall only if we've seen it AND it's "w"
}

void CommandHandler::Explore(const Percepts &percepts, const WorldModel &wm) {}

std::vector<std::string>
CommandHandler::ExecuteCommand(const Percepts &percepts, const WorldModel &wm) {

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
      Explore(percepts, wm);

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

// DEBUG + PRINT
std::string CommandHandler::OppositeCmd(std::string cmd) {

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
void CommandHandler::PrintCmdQueue(std::queue<std::string> q) {
  if (q.empty() || q.size() < 0)
    return;

  while (!q.empty()) {
    std::cout << q.front() + " ";
    q.pop();
  }

  std::cout << std::endl;
};

// I want to determine the next best move, by seeing my percepts and what I
// have explored and then based on that select the next cell that i have
// least explored - if two are the same, then the first one so the
// calculation would be the amount of points it would cost divided by the
// amount of visits (cost/visits); biggest number of all directions wins and
// that's the way the agent goes

/***************************************************************
AI CLASS DEFINITION
*/
AI::AI() {}
AI::AI(unsigned id, unsigned agent_speed, std::mt19937_64 *rng, Symbols symbols,
       Costs costs, int max_turn)
    : id(id), agent_speed(agent_speed), rng(rng), symbols(symbols),
      costs(costs), max_turn(max_turn) {}

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

  return worldModel.UpdateCoords(cmds.ExecuteCommand(percepts, worldModel));
};
