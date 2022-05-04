#include "App.h"
#include "Graph.h"

#include <iostream>

using namespace std;

int main() {
  App app;
  if (!app.init("words_alpha.txt", false))
    return -1;
  app.run();
  return 0;
}
