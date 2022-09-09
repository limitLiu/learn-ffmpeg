#include "app.h"

int main(int argc, char **argv) {
  Player::App app;
  app.render();

  while (app.running()) {
    app.handleEvents();
  }

  return 0;
}
