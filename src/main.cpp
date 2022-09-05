#include "app.h"

int main(int argc, char **argv) {
  Player::App app;

  while (app.running()) {
    app.render();
    app.handleEvents();
  }

  return 0;
}
