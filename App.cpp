#include "App.h"

#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <list>
#include <menu.h>
#include <ncurses.h>
#include <random>
#include <sstream>
#include <unordered_set>

#define ERROR_COLOR COLOR_PAIR(1)
#define HIGHLIGHT COLOR_PAIR(2)
#define WARNING_COLOR COLOR_PAIR(3)

using namespace std;

string getstring(WINDOW *w) {
  string input;

  // let the terminal do the line editing
  nocbreak();
  echo();

  // this reads from buffer after <ENTER>, not "raw"
  // so any backspacing etc. has already been taken care of
  int ch = wgetch(w);

  while (ch != '\n') {
    input.push_back(ch);
    ch = wgetch(w);
  }

  // restore your cbreak / echo settings here

  cbreak();
  noecho();
  return input;
}
int strdiff(string a, string b) {
  int dif = 0;
  for (int i = 0; i < a.length(); i++)
    if (a[i] != b[i])
      dif++;
  return dif;
}

void UserData::readData(int nr_game) {
  ifstream in(username + ".csv");
  string line;
  while (nr_game > 0) {
    getline(in, line);
    nr_game--;
  }
  stringstream ss(line);
  vector<string> v;
  while (ss.good()) {
    string s;
    getline(ss, s, ',');
    v.push_back(s);
  }
  this->nr_game = stoi(v[0]);
  hints = stoi(v[1]);
  nr_user_moves = stoi(v[2]);
  nr_efficient_moves = stoi(v[3]);
  start_word = v[4];
  target_word = v[5];
  date_and_time = v[6];
  for (int i = 7; i < v.size(); i++)
    user_path.push_back(v[i]);
  in.close();
}
void UserData::writeData() {
  ifstream in(username + ".csv");
  string line;
  int lines;

  for (lines = 0; std::getline(in, line); lines++)
    ;
  nr_game = lines + 1;
  ofstream file(username + ".csv", ios_base::app);
  file << nr_game << ',' << hints << ',' << nr_user_moves << ','
       << nr_efficient_moves << ',' << start_word << ',' << target_word << ','
       << date_and_time;
  for (auto it = user_path.begin(); it != user_path.end(); it++)
    file << ',' << *it;
  file << endl;
  file.close();
}
int UserData::getNrGames() {
  int nr_lines = 0;
  ifstream in(username + ".csv");
  string line;
  while (!in.eof()) {
    getline(in, line);
    nr_lines++;
  }
  in.close();
  return nr_lines - 1;
}

App::App() {
  // initialize ncurses
  initscr();
  cbreak();
  noecho();
  curs_set(0);
  keypad(stdscr, TRUE);
  // define the look of the app
  title = newwin(3, COLS, 0, 0);
  menu = newwin(LINES - 3, COLS / 3, 3, 0);
  description = newwin(LINES - 3, 2 * COLS / 3, 3, COLS / 3);
  box(title, 0, 0);
  box(menu, 0, 0);
  box(description, 0, 0);
  // define the menu structures

  ITEM **main_menu = new ITEM *[4];
  ITEM **automatic = new ITEM *[2];
  ITEM **playing = new ITEM *[2];
  ITEM **analitic = new ITEM *[2];

  // TODO: cum o sa te folosesti de user pointerul fiecarui item?
  main_menu[0] = new_item("1.Automatic mode", "Automatic mode");
  main_menu[1] = new_item("2.Playing mode", "Playing mode");
  main_menu[2] = new_item("3.Analytics module", "Analytics module");
  main_menu[3] = new_item("0.Exit", "Exit");
  automatic[0] = new_item("1.See best path", "See best path");
  automatic[1] = new_item("0.Get back to Menu", "Get back to Menu");
  playing[0] = new_item("1.Play!", "Play");
  playing[1] = new_item("0.Get back to Menu", "Get back to Menu");
  // TODO: maybe a rank based output of users
  analitic[0] = new_item("1.See stats of a user", "See stats");
  analitic[1] = new_item("0.Get back to Menu", "Get back to Menu");

  menus["Menu"] = new_menu(main_menu);
  menus["Automatic mode"] = new_menu(automatic);
  menus["Playing mode"] = new_menu(playing);
  menus["Analytics module"] = new_menu(analitic);
  for (auto it = menus.begin(); it != menus.end(); it++) {
    set_menu_win(it->second, menu);
    set_menu_sub(it->second, derwin(menu, LINES - 4, COLS / 3 - 1, 1, 1));
    menu_opts_off(it->second, O_SHOWDESC);
    if (it->first == "Menu")
      post_menu(it->second);
    wrefresh(menu);
  }
  box(menu, 0, 0);
  curr_menu = "Menu";
}
App::~App() {
  // TODO: delete the windows
  endwin();
}

bool App::init(string dictFile, bool dictOn) {
  printw("Starting...\n");
  refresh();
  if (has_colors() == FALSE) {
    endwin();
    cout << "Your terminal doesn't support colors!\n";
    return false;
  }

  start_color();
  init_pair(1, COLOR_RED, COLOR_BLACK);
  init_pair(2, COLOR_BLUE, COLOR_BLACK);
  init_pair(3, COLOR_YELLOW, COLOR_BLACK);
  if (dictOn) {
    ifstream in(dictFile.c_str());
    if (!in.is_open()) {
      endwin();
      cout << "Cannot open the given dictionary!\n";
      cin.get();
      return false;
    }
    unordered_map<string, unordered_set<string>>
        wdict; // The wildcard unordered_map
    string s;
    while (!in.eof()) {
      in >> s;
      words[s.length()].push_back(s);
      for (int i = 0; i < s.length(); i++) {
        string wildcard = s;
        wildcard[i] = '*';
        wdict[wildcard].insert(s);
      }
    }
    printw("Finised reading the dictionary...\n");
    refresh();
    g = Graph<string>();
    unordered_map<string, unordered_set<string>>::iterator it;
    for (it = wdict.begin(); it != wdict.end(); it++) {
      unordered_set<string>::iterator it_set, it_set_next;
      unordered_set<string> s = it->second;
      for (it_set = s.begin(); it_set != s.end(); it_set++) {
        for (it_set_next = it_set; it_set_next != s.end(); it_set_next++) {
          if (it_set_next != it_set)
            g.addEdge(*it_set, *it_set_next);
        }
      }
    }
    string w1, w2;
    printw("Finised creating the graph structure...\n");
  }
  refreshWindows();
  return true;
}

bool App::optionSelected(string option) {
  if (option == "Get back to Menu") {
    post_menu(menus[curr_menu]);
    curr_menu = "Menu";
    // Refresh menu bullshit
    // NOTE: This might be in itself a separated function
    wclear(menus[curr_menu]->userwin);
    post_menu(menus[curr_menu]);
    wrefresh(menu);
    wrefresh(menus[curr_menu]->userwin);
    menu_driver(menus[curr_menu], REQ_FIRST_ITEM);
    refreshWindows();
    return true;
  } else if (option == "Exit") {
    return false;
  } else if (option == "See best path") {
    refreshWindows();
    wmove(description, 1, 1);
    wprintw(description, "Start word:");
    string start = getstring(description);
    wmove(description, 2, 1);
    wprintw(description, "End word:");
    string end = getstring(description);
    list<string> path = g.findPath(start, end);
    wmove(description, 3, 1);
    if (path.size() == 0) {
      wprintw(description, "There is no path between %s and %s!", start.c_str(),
              end.c_str());
    } else {
      wprintw(description, "The path is:");
      wmove(description, 4, 1);
      for (auto it = path.begin(); it != path.end(); it++) {
        wprintw(description, " %s", it->c_str());
      }
    }
    wgetch(description);
    return true;
  } else if (option == "Play") {
    return playFunction();
  } else if (option == "See stats") {
    refreshWindows();
    wmove(description, 1, 1);
    wprintw(description, "Username:");
    string user = getstring(description);
    refreshWindows();
    ifstream in(user + ".csv");
    if (!in.is_open()) {
      wmove(description, 1, 1);
      wprintw(description, "The user %s doesn't have valid saves!",
              user.c_str());
      wgetch(description);
      return true;
    }
    in.close();
    unordered_set<string> user_words;
    UserData data;
    data.username = user;
    int nr_games = data.getNrGames();
    for (int i = 1; i <= nr_games; i++) {
      data.readData(i);
      for (auto it = data.user_path.begin(); it != data.user_path.end(); it++)
        user_words.insert(*it);
    }
    wmove(description, 1, 1);
    wprintw(description, "The user used the following words:");
    wmove(description, 2, 1);
    int x, y;
    for (auto it = user_words.begin(); it != user_words.end(); it++) {
      getyx(description, y, x);
      if (x > getmaxx(description) - 8) {
        wmove(description, y + 1, 1);
      }
      wprintw(description, "%s ", it->c_str());
    }
    wgetch(description);
    return true;
  }
  return false;
}

bool App::playFunction() {
  refreshWindows();
  wmove(description, 1, 1);
  wprintw(description, "Username:");
  string user = getstring(description);
  string first_word, second_word;
  wmove(description, 2, 1);
  wprintw(description, "The number of letters:");
  string number = getstring(description);
  int n = stoi(number);
  try {
    words.at(n);
  } catch (out_of_range) {
    return false;
  }
  random_device dev;
  mt19937 rng(dev());
  uniform_int_distribution<mt19937::result_type> dist(1, words[n].size() - 1);
  first_word = words[n].at(dist(rng));
  second_word = words[n].at(dist(rng));
  refreshWindows();

  int line = 4;
  int nr_hints = 0;
  string curr_word = first_word;
  list<string> efficient_path = g.findPath(first_word, second_word), user_path;
  int eff_moves = efficient_path.size() - 1;

  user_path.push_back(first_word);
  efficient_path.pop_front();
  wmove(description, 1, 1);
  wprintw(description, "You need to get from %s to %s!", first_word.c_str(),
          second_word.c_str());
  wmove(description, 2, 1);
  wprintw(description, "If you need a hint just enter the letter h!");
  wmove(description, 3, 1);
  wprintw(description, first_word.c_str());
  // Here starts the game
  while (curr_word != second_word) {
    wmove(description, line, 1);
    string word = getstring(description);
    wmove(description, line + 1, 1);
    wclrtoeol(description);
    if (word == "n") {
      wmove(description, line, 1);
      word = *efficient_path.begin();
      wprintw(description, word.c_str());
    }
    if (word == "h") {
      nr_hints++;
      int i;
      for (i = 0; i < n; i++)
        if (curr_word[i] != (*efficient_path.begin())[i])
          break;
      char s[2] = "a";
      s[0] = curr_word[i];
      wattron(description, HIGHLIGHT);
      wmove(description, line - 1, i + 1);
      wprintw(description, s);
      wattroff(description, HIGHLIGHT);
      wmove(description, line + 1, 1);
      wattron(description, WARNING_COLOR);
      wprintw(description,
              "For an efficient path change the char at position %d!", i);
      wattroff(description, WARNING_COLOR);
    } else if (word.length() == n && g.getNeighbours(curr_word).find(word) !=
                                         g.getNeighbours(curr_word).end()) {
      curr_word = word;
      user_path.push_back(curr_word);
      wmove(description, ++line, 1);
      // box(description, 0, 0);
      wclrtoeol(description);
      if (*efficient_path.begin() != word) {
        // we dont have the efficient word chosen
        // so we calculate the new efficient path
        efficient_path = g.findPath(curr_word, second_word);
      }
      efficient_path.pop_front();
    } else if (word.length() != n) {
      wmove(description, line, 1);
      wattron(description, ERROR_COLOR);
      wprintw(description, word.c_str());
      wattroff(description, ERROR_COLOR);
      wmove(description, line + 1, 1);
      wattron(description, WARNING_COLOR);
      wprintw(description, "The word must have %d letters!", n);
      wattroff(description, WARNING_COLOR);
    } else if (strdiff(word, curr_word) == 1) {
      wmove(description, line, 1);
      wattron(description, ERROR_COLOR);
      wprintw(description, word.c_str());
      wattroff(description, ERROR_COLOR);
      wmove(description, line + 1, 1);
      wattron(description, WARNING_COLOR);
      wprintw(description, "That word doesn't exist!");
      wattroff(description, WARNING_COLOR);
    } else {
      wmove(description, line, 1);
      wattron(description, ERROR_COLOR);
      wprintw(description, word.c_str());
      wattroff(description, ERROR_COLOR);
      wmove(description, line + 1, 1);
      wattron(description, WARNING_COLOR);
      wprintw(description, "The word must change one letter!");
      wattroff(description, WARNING_COLOR);
    }
    refresh();
    wrefresh(description);
  }
  wgetch(description);
  refreshWindows();
  wmove(description, 1, 1);
  wprintw(description, "YOU WON!");
  wgetch(description);

  time_t result = time(nullptr);
  string dateAndTime = ctime(&result);

  UserData data;
  data.username = user;
  data.nr_user_moves = user_path.size() - 1;
  data.nr_efficient_moves = eff_moves;
  data.hints = nr_hints;
  data.start_word = first_word;
  data.target_word = second_word;
  data.user_path = user_path;
  data.writeData();
  return true;
}

void App::run() {
  // NOTE: aici o sa ai un while ce ia input si cheama update si process input
  int c = 'a';
  while (c != KEY_F(1)) {
    c = getch();
    switch (c) {

    case KEY_DOWN:
      menu_driver(menus[curr_menu], REQ_DOWN_ITEM);
      break;
    case KEY_UP:
      menu_driver(menus[curr_menu], REQ_UP_ITEM);
      break;
    case KEY_NPAGE:
      menu_driver(menus[curr_menu], REQ_SCR_DPAGE);
      break;
    case KEY_PPAGE:
      menu_driver(menus[curr_menu], REQ_SCR_UPAGE);
      break;
    case '\n':
      // Enter case
      ITEM *curr_it = current_item(menus[curr_menu]);
      if (menus.find(curr_it->description.str) != menus.end()) {
        unpost_menu(menus[curr_menu]);
        curr_menu = curr_it->description.str;
        wclear(menus[curr_menu]->userwin);
        post_menu(menus[curr_menu]);
        wrefresh(menu);
        wrefresh(menus[curr_menu]->userwin);
        menu_driver(menus[curr_menu], REQ_FIRST_ITEM);
        break;
      }
      if (!optionSelected(curr_it->description.str))
        c = KEY_F(1);
      break;
    }
    refreshWindows();
  }
}
void App::refreshWindows() {
  clear();
  wclear(title);
  wclear(description);
  refresh();
  box(menus[curr_menu]->userwin, 0, 0);
  box(title, 0, 0);
  box(description, 0, 0);
  wmove(title, 1, COLS / 2 - curr_menu.size() / 2);
  wprintw(title, curr_menu.c_str());
  wrefresh(menus[curr_menu]->userwin);
  wrefresh(title);
  wrefresh(description);
}
