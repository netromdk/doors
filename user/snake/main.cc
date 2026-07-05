#include <cstdint>

#include "Game.h"
#include "Input.h"
#include "Screen.h"
#include "lib/Syscall.h"

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
      if (pk.ch == 'c' || pk.ch == 'C') {
        break;
      }
      if (pk.ch == 'w' || pk.ch == 'W') {
        game.setWrapMode(true);
        break;
      }
    }
  }
  for (int c = 1; c < SnakeGame::BOARD_COLS + 1; ++c) {
    Screen::put(PROMPT_ROW, c, ' ', 0);
  }
}

void countDown(SnakeGame &game, uint64_t startMs)
{
  for (int i = 3; i >= 1; --i) {
    game.drawCountdown(i);
    const uint64_t target = startMs + (4 - i) * 1000;
    while (static_cast<uint64_t>(sys_sysinfo(SYSINFO_UPTIME, 0)) < target) {
    }
    game.clearOverlay();
  }
  game.drawBoard();
}

} // namespace

extern "C" [[noreturn]] void _start()
{
  sys_suppressTaskbar();
  Screen::save();
  Screen::cls(0);
  Screen::cursorHide();

  SnakeGame game{};
  const bool withObstacles{true};
  const auto seedMs = static_cast<uint32_t>(sys_sysinfo(SYSINFO_UPTIME, 0));
  game.init(seedMs, withObstacles);

  game.drawBoard();

  chooseMode(game);
  countDown(game, static_cast<uint64_t>(sys_sysinfo(SYSINFO_UPTIME, 0)));

  uint64_t lastMove = static_cast<uint64_t>(sys_sysinfo(SYSINFO_UPTIME, 0));
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
      continue;
    }

    if (ke.key >= Input::Key::Up && ke.key <= Input::Key::Right) {
      game.setDir(keyToDir(ke.key));
    }

    const auto now = static_cast<uint64_t>(sys_sysinfo(SYSINFO_UPTIME, 0));
    if (now - lastMove >= static_cast<uint64_t>(game.moveIntervalMs())) {
      const auto dt = now - lastMove;
      lastMove = now;
      if (!game.step(dt)) {
        break;
      }
    }
  }

  if (!quit) {
    if (game.score() > SnakeGame::highScore()) {
      SnakeGame::setHighScore(game.score());
    }
    game.drawGameOver();

    while (true) {
      if (const auto pk = Input::poll();
          pk.key == Input::Key::Char && (pk.ch == 'q' || pk.ch == 'Q')) {
        break;
      }
    }
  }

  Screen::restore();
  Screen::cursorShow();

  sys_exit();
}
