#include "Graph.h"

#include <iostream>
#include <list>
#include <menu.h>
#include <ncurses.h>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace std;

class UserData {
public:
  void readData(int nr_game);
  void writeData();
  int getNrGames();

  int nr_game;
  int hints;
  int nr_user_moves, nr_efficient_moves;
  string username;
  string start_word, target_word;
  string date_and_time;
  list<string> user_path;
};

class App {
public:
  App();
  ~App();
  bool init(string dictFile, bool dictOn = true);
  void run();

private:
  void refreshWindows();
  bool optionSelected(string option);
  bool playFunction();

  Graph<string> g;
  WINDOW *title;
  WINDOW *menu;
  WINDOW *description;
  string curr_menu;
  unordered_map<string, MENU *> menus;
  unordered_map<int, vector<string>> words;
};
