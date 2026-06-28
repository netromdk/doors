#include <kernel/Pit.h>
#include <kernel/Scheduler.h>
#include <programs/api/Input.h>
#include <programs/api/Screen.h>
#include <programs/snake/Snake.h>
#include <programs/snake/SnakeGame.h>

namespace {

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

void chooseMode(SnakeGame &game)
{
  constexpr int PROMPT_ROW = SnakeGame::BOARD_ROWS;
  constexpr const char prompt[] = "Classic (C) or Wrap (W)?";
  const int promptCol = (SnakeGame::BOARD_COLS + 2 - static_cast<int>(sizeof(prompt) - 1)) / 2;
  for (int i = 0; prompt[i]; ++i) {
    Screen::put(PROMPT_ROW, promptCol + i, prompt[i], 0x0F);
  }
  while (true) {
    if (const auto pk = Input::poll(); pk.key == Input::Key::Char) {
      if (pk.ch == 'c' || pk.ch == 'C') break;
      if (pk.ch == 'w' || pk.ch == 'W') {
        game.setWrapMode(true);
        break;
      }
    }
    __asm__("hlt");
  }
  for (int c = 1; c < SnakeGame::BOARD_COLS + 1; ++c) {
    Screen::put(PROMPT_ROW, c, ' ', 0);
  }
}

void countDown(SnakeGame &game)
{
  for (int i = 3; i >= 1; --i) {
    game.drawCountdown(i);
    const auto cdStart = Pit::uptimeMs();
    while (Pit::msSince(cdStart) < 1000) {
      __asm__("hlt");
    }
    game.clearOverlay();
  }
  game.drawBoard();
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
  Scheduler::suppressTaskbar();
  SnakeGame game{};
  const bool withObstacles{true};
  game.init(static_cast<uint32_t>(Pit::uptimeMs()), withObstacles);

  Screen::save();
  Screen::cls(0);
  Screen::cursorHide();
  game.drawBoard();

  chooseMode(game);
  countDown(game);

  uint64_t lastMove = Pit::uptimeMs();
  bool quit = false;
  bool paused = false;

  while (true) {
    const auto ke = Input::poll();
    if (ke.key == Input::Key::Char && (ke.ch == 'p' || ke.ch == 'P' || ke.ch == ' ')) {
      paused = !paused;
      if (paused) {
        game.drawPause();
      }
      else {
        game.clearOverlay();
      }
    }

    if (ke.key == Input::Key::Char && ke.ch == 'q') {
      quit = true;
      break;
    }

    if (paused) {
      __asm__("hlt");
      continue;
    }

    if (ke.key >= Input::Key::Up && ke.key <= Input::Key::Right) {
      game.setDir(keyToDir(ke.key));
    }

    if (Pit::msSince(lastMove) >= static_cast<uint64_t>(game.moveIntervalMs())) {
      const auto now = Pit::uptimeMs();
      const auto dt = now - lastMove;
      lastMove = now;
      if (!game.step(dt)) {
        break;
      }
    }

    __asm__("hlt");
  }

  if (!quit) {
    if (game.score() > SnakeGame::highScore()) {
      SnakeGame::setHighScore(game.score());
    }
    game.drawGameOver();

    // Only 'q' quits.
    while (true) {
      if (const auto pk = Input::poll();
          pk.key == Input::Key::Char && (pk.ch == 'q' || pk.ch == 'Q')) {
        break;
      }
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
