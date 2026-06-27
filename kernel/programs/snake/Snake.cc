#include <kernel/Pit.h>
#include <kernel/Scheduler.h>
#include <programs/api/Input.h>
#include <programs/api/Screen.h>
#include <programs/snake/Snake.h>
#include <programs/snake/SnakeGame.h>

namespace {

constexpr uint64_t MOVE_MS = 200;

SnakeGame::Dir keyToDir(Input::Key k)
{
  switch (k) {
  case Input::Key::Up:
    return SnakeGame::Dir::Up;
  case Input::Key::Down:
    return SnakeGame::Dir::Down;
  case Input::Key::Left:
    return SnakeGame::Dir::Left;
  case Input::Key::Right:
    return SnakeGame::Dir::Right;
  default:
    return SnakeGame::Dir::Right;
  }
}

} // namespace

namespace Snake {

int shellTaskId_ = -1;

void setShellTaskId(int id)
{
  shellTaskId_ = id;
}

void snakeMain()
{
  SnakeGame game{};
  game.init(static_cast<uint32_t>(Pit::uptimeMs()));
  Screen::save();
  Screen::cls(0);
  Screen::cursorHide();
  game.drawBoard();

  uint64_t lastMove = Pit::uptimeMs();
  bool quit = false;

  while (true) {
    const auto ke = Input::poll();
    if (ke.key == Input::Key::Char && ke.ch == 'q') {
      quit = true;
      break;
    }

    if (ke.key != Input::Key::Unknown) {
      game.setDir(keyToDir(ke.key));
    }

    if (Pit::msSince(lastMove) >= MOVE_MS) {
      lastMove = Pit::uptimeMs();
      if (!game.step()) {
        break;
      }
    }

    __asm__("hlt");
  }

  if (!quit) {
    game.drawGameOver();
    const auto endTime = Pit::uptimeMs();
    while (Pit::msSince(endTime) < 2000) {
      __asm__("hlt");
    }
  }

  Screen::restore();
  Screen::cursorShow();

  __asm__("cli");
  Scheduler::unblockTask(shellTaskId_);
  Scheduler::exitCurrentTask();
}

} // namespace Snake
