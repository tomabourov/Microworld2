#include "ai.hpp"

// HELPERS

namespace Cells
{
  inline const std::string Wall = "w";
  inline const std::string OpenSpace = "o";
  inline const std::string Start = "@";
  inline const std::string Trap = "^";
  inline const std::string ExplodedTrap = "#";
  inline const std::string DisarmedTrap = "+";
  inline const std::string Treasure = "$";

  inline const std::string TeleporterOneOpen = "[";
  inline const std::string TeleporterTwoOpen = "{";
  inline const std::string TeleporterThreeOpen = "(";

  inline const std::string TeleporterOneClosed = "]";
  inline const std::string TeleporterTwoClosed = "}";
  inline const std::string TeleporterThreeClosed = ")";

  static bool isWall(std::string cell)
  {
    if (cell == Cells::Wall)
      return true;
    return false;
  }

  static bool isOpenSpace(std::string cell)
  {
    if (cell == Cells::OpenSpace)
      return true;
    return false;
  }

  static bool isDisarmedTrap(std::string cell)
  {
    if (cell == Cells::DisarmedTrap)
      return true;
    return false;
  }

  static bool isExplodedTrap(std::string cell)
  {
    if (cell == Cells::ExplodedTrap)
      return true;
    return false;
  }

  static bool isTreasure(std::string cell)
  {
    if (cell == Cells::Treasure)
      return true;
    return false;
  }

  static bool isTeleporter(std::string cell)
  {
    if (
        cell == Cells::TeleporterOneClosed || cell == Cells::TeleporterOneOpen || cell == Cells::TeleporterTwoClosed || cell == Cells::TeleporterTwoOpen || cell == Cells::TeleporterThreeClosed || cell == Cells::TeleporterThreeOpen)
    {
      return true;
    }
    return false;
  }
};

namespace Cmdss
{
  inline const std::string MoveForward = "F";
  inline const std::string MoveBackward = "B";
  inline const std::string RotateLeft = "L";
  inline const std::string RotateRight = "R";
  inline const std::string UseTeleporter = "U";
  inline const std::string CollectTreasure = "T";
  inline const std::string DisarmTrap = "D";

  static void EmptyQueue(std::vector<std::string> &cmd_queue)
  {
    cmd_queue.clear();
  };

  static void QueueCmd(std::vector<std::string> &cmd_queue, std::string cmd)
  {
    cmd_queue.insert(cmd_queue.begin(), cmd);
  }

  static std::vector<std::string> ExecuteCurrentCommand(std::vector<std::string> &cmd_queue)
  {
    if (cmd_queue.empty())
    {
      std::cout << "[DEBUG] No more commands to execute\n\n";
      return {};
    }
    else
    {
      std::string cmd_to_execute = cmd_queue.back();

      cmd_queue.pop_back();

      return {cmd_to_execute};
    }
  }
};

class Cmds
{
  Cmds()
  {
    this->cmd_queue = {{}};
  };

private:
  std::vector<std::vector<std::string>> cmd_queue;

public:
  const std::string MoveForward = "F";
  const std::string MoveBackward = "B";
  const std::string RotateLeft = "L";
  const std::string RotateRight = "R";
  const std::string UseTeleporter = "U";
  const std::string CollectTreasure = "T";
  const std::string DisarmTrap = "D";

  void QueueCommandGroup(std::vector<std::string> cmd_group)
  {
    cmd_queue.push_back(cmd_group);
  }


};

/***************************************************************
AI CLASS DEFINITION
*/
AI::AI()
{
}
AI::AI(
    unsigned id,
    unsigned agent_speed,
    std::mt19937_64 *rng,
    Symbols symbols,
    Costs costs,
    int max_turn)
    : id(id), agent_speed(agent_speed), rng(rng),
      symbols(symbols), costs(costs), max_turn(max_turn)
{
}

// STATICS
int AI::defuses = 0;

Cmds cmds = Cmds();

// PRINT
void AI::PrintPercepts(const Percepts &percepts)
{
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
  for (size_t i = 0; i < percepts.others.size(); i++)
  {
    std::cout << "   " << i << ": " << percepts.others[i].to_string() << std::endl;
  }
}

// HELPER METHODS

void AI::DefuseMine(
    Percepts &percepts)
{
  // will always disarm with net positive point value
  Cmds::QueueCmd(cmd_queue, Cmds::DisarmTrap);
  Cmds::QueueCmd(cmd_queue, Cmds::RotateLeft);
  Cmds::QueueCmd(cmd_queue, Cmds::DisarmTrap);
  Cmds::QueueCmd(cmd_queue, Cmds::RotateRight);
  Cmds::QueueCmd(cmd_queue, Cmds::RotateRight);
  Cmds::QueueCmd(cmd_queue, Cmds::DisarmTrap);
  Cmds::QueueCmd(cmd_queue, Cmds::RotateRight);
  Cmds::QueueCmd(cmd_queue, Cmds::DisarmTrap);

  
}

void AI::TakeTreasure()
{
  Cmds::QueueCmd(cmd_queue, Cmds::CollectTreasure);
}

void AI::SearchTreasure(
    Percepts &percepts)
{
  // iterate through all percepts to see if there is a treasure;
  // if found, return a vector of instuctions on how to get to it;

  percepts.backward;
  percepts.current;
  percepts.left;
  percepts.right;
  percepts.forward;

  if (Cells::isTreasure(percepts.current[0]))
    return AI::TakeTreasure();
}

std::vector<std::string> AI::Run(
    Percepts &percepts,
    AgentComm *comms)
{
  PrintPercepts(percepts);

  if (percepts.detector == 1 && cmd_queue.empty())
  {
    AI::DefuseMine(percepts);
  }

  return Cmds::ExecuteCurrentCommand(cmd_queue);
}
