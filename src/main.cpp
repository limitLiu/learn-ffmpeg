#include "app.h"
#include <memory>

int main(int argc, char **argv) {
  auto app = std::make_unique<Player::App>();

  while (app->running()) {
    app->render();
    app->handleEvents();
  }

  return 0;
}
