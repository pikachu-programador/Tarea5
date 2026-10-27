#include <iostream>
#include <vector>
#include <queue>
#include <stack>
#include <map>
#include <memory>
#include <stdexcept>

using namespace std;

class Node {
private:
    vector<int> neighbors;

    friend class Graph;
    friend class BFSIterator;
    friend class DFSIterator;
};

class Graph {
private:
    map<int, Node> nodes;

    friend class BFSIterator;
    friend class DFSIterator;

public:
    void addEdge(int from, int to) {
        nodes[from].neighbors.push_back(to);
        nodes[to]; // Asegura que el nodo destino exista en el mapa
    }
};

class GraphIterator {
public:
    virtual ~GraphIterator() = default;
    virtual bool hasNext() const = 0;
    virtual int next() = 0;
};

class BFSIterator : public GraphIterator {
private:
    const Graph& graph;
    queue<int> q;
    map<int, bool> visited;

public:
    BFSIterator(const Graph& g, int startNode) : graph(g) {
        if (graph.nodes.find(startNode) != graph.nodes.end()) {
            q.push(startNode);
            visited[startNode] = true;
        }
    }

    bool hasNext() const override { return !q.empty(); }

    int next() override {
        if (!hasNext()) throw out_of_range("BFS: no hay mas nodos");
        int current = q.front();
        q.pop();

        for (int neighbor : graph.nodes.at(current).neighbors) {
            if (!visited[neighbor]) {
                visited[neighbor] = true;
                q.push(neighbor);
            }
        }
        return current;
    }
};

class DFSIterator : public GraphIterator {
private:
    const Graph& graph;
    stack<int> s;
    map<int, bool> visited;

public:
    DFSIterator(const Graph& g, int startNode) : graph(g) {
        if (graph.nodes.find(startNode) != graph.nodes.end()) {
            s.push(startNode);
        }
    }

    bool hasNext() const override { return !s.empty(); }

    int next() override {
        if (!hasNext()) throw out_of_range("DFS: no hay mas nodos");
        int current = s.top();
        s.pop();
        visited[current] = true;

        const auto& neighbors = graph.nodes.at(current).neighbors;
        for (int i = static_cast<int>(neighbors.size()) - 1; i >= 0; --i) {
            if (!visited[neighbors[i]]) {
                s.push(neighbors[i]);
            }
        }

        while (!s.empty() && visited[s.top()]) {
            s.pop();
        }
        return current;
    }
};

int main() {
    Graph g;
    g.addEdge(1, 2);
    g.addEdge(1, 3);
    g.addEdge(2, 4);

    unique_ptr<GraphIterator> it = make_unique<BFSIterator>(g, 1);

    cout << "BFS: ";
    while (it->hasNext()) {
        cout << it->next() << " ";
    }
    cout << endl;

    // Cambio dinamico del iterador
    it = make_unique<DFSIterator>(g, 1);
    cout << "DFS: ";
    while (it->hasNext()) {
        cout << it->next() << " ";
    }
    cout << endl;

    // Se agregan ciclos (4 -> 1, 3 -> 1): el recorrido no debe quedarse en bucle
    g.addEdge(4, 1);
    g.addEdge(3, 1);
    cout << "\nCon ciclos:" << endl;

    it = make_unique<BFSIterator>(g, 1);
    cout << "BFS: ";
    while (it->hasNext()) {
        cout << it->next() << " ";
    }
    cout << endl;

    it = make_unique<DFSIterator>(g, 1);
    cout << "DFS: ";
    while (it->hasNext()) {
        cout << it->next() << " ";
    }
    cout << endl;

    return 0;
}
