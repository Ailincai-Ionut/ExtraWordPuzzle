#pragma once

#include <iostream>
#include <unordered_map>
#include <unordered_set>

#include <list>
#include <queue>

using namespace std;

template <class Elem> class Graph {
public:
  Graph(){};
  ~Graph(){};

  void addVertex(Elem vertex);
  void addEdge(Elem a, Elem b, bool directed = false);
  unordered_set<Elem> &getNeighbours(Elem vertex);
  void deleteVertex(Elem vertex);
  list<Elem> findPath(Elem start, Elem end);

  friend ostream &operator<<(ostream &out, const Graph &g) {
    typename unordered_map<Elem, unordered_set<Elem>>::const_iterator itm;
    out << "[";
    for (itm = g.adj_map.begin(); itm != g.adj_map.end(); itm++) {
      unordered_set<Elem> s = itm->second;
      for (typename unordered_set<Elem>::iterator it = s.begin(); it != s.end();
           it++)
        out << "{" << itm->first << "->" << *it << "},";
    }
    out << "]";
    return out;
  }

private:
  unordered_map<Elem, unordered_set<Elem>> adj_map;
};

template <class Elem> void Graph<Elem>::addVertex(Elem vertex) {
  adj_map.insert(make_pair(vertex, unordered_set<Elem>()));
}
template <class Elem> void Graph<Elem>::addEdge(Elem a, Elem b, bool directed) {
  try {
    if (adj_map.find(a) == adj_map.end())
      adj_map[a] = unordered_set<Elem>();
    if (adj_map.find(b) == adj_map.end())
      adj_map[b] = unordered_set<Elem>();
    // adj_map.insert(make_pair(a, unordered_set<Elem>()));
    // adj_map.insert(make_pair(b, unordered_set<Elem>()));
    adj_map[a].insert(b);
    if (!directed)
      adj_map[b].insert(a);
  } catch (out_of_range) {
    cout << "This shit shouldn't happen!(addEdge)\n";
  }
}
template <class Elem> void Graph<Elem>::deleteVertex(Elem vertex) {
  typename unordered_map<Elem, unordered_set<Elem>>::iterator mapIt;
  // Go through all the adj lists
  for (mapIt = adj_map.begin(); mapIt != adj_map.end(); mapIt++) {
    // If the index is not the element we want we delete from the other adj
    // lists
    if (mapIt->first != vertex) {
      unordered_set<int> &neighbours = mapIt->second;
      for (unordered_set<int>::iterator it = neighbours.begin();
           it != neighbours.end(); it++)
        if (*it == vertex)
          neighbours.erase(it);
    } else {
      // If we found the vertex we delete it
      adj_map.erase(mapIt);
    }
  }
}
template <class Elem>
unordered_set<Elem> &Graph<Elem>::getNeighbours(Elem vertex) {
  return adj_map.at(vertex);
}

template <class Elem> list<Elem> Graph<Elem>::findPath(Elem start, Elem end) {
  queue<Elem> q;
  unordered_map<Elem, bool> traversed;
  unordered_map<Elem, Elem> predecessor;
  for (typename unordered_map<Elem, unordered_set<Elem>>::iterator it =
           adj_map.begin();
       it != adj_map.end(); it++)
    traversed[it->first] = false;
  q.push(start);
  predecessor[start] = "";
  while (!q.empty()) {
    Elem v = q.front();
    q.pop();
    if (v == end) {
      break;
    }
    unordered_set<Elem> n = getNeighbours(v);
    for (typename unordered_set<Elem>::iterator it = n.begin(); it != n.end();
         it++)
      if (!traversed[*it]) {
        predecessor[*it] = v;
        traversed[*it] = true;
        q.push(*it);
      }
  }

  // Reusing q
  for (typename unordered_map<Elem, unordered_set<Elem>>::iterator it =
           adj_map.begin();
       it != adj_map.end(); it++)
    traversed[it->first] = false;
  q = queue<Elem>();
  Elem v = end;
  while (predecessor.find(v) != predecessor.end() && traversed[v] == false) {
    q.push(v);
    traversed[v] = true;
    if (v == start)
      break;
    v = predecessor[v];
  }
  list<Elem> path;
  while (!q.empty()) {
    path.push_front(q.front());
    q.pop();
  }

  return path;
  // TODO: what do you do if there is no path?
}
