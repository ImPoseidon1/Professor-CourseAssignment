#include <bits/stdc++.h>

using namespace std;

struct Edge {int to;double capacity;int rev;}; // Index of the reverse edge in the adjacency list of 'to'

vector<vector<Edge>> adj; // Global adjacency list

// Initializes the adjacency list for a given number of vertices
void initGraph(int vertices) {
  adj.clear();
  adj.resize(vertices);
}

// Adds an edge to the graph
void addEdge(int from, int to, double capacity) {
  Edge forward = {to, capacity, static_cast<int>(adj[to].size())};
  Edge backward = {from, 0, static_cast<int>(adj[from].size())};
  adj[from].push_back(forward);
  adj[to].push_back(backward);
}

// Returns a constant reference to the adjacency list of a vertex
const vector<Edge> &getAdj(int v) { return adj[v]; }

// // Returns a modifiable reference to the adjacency list of a vertex
vector<Edge> &modifiableGetAdj(int v) { return adj[v]; }

// // Returns the size of the adjacency list (number of vertices in the graph)
int graphSize() { return adj.size(); }

bool bfs(int source, int sink, vector<int> &parent) {
  fill(parent.begin(), parent.end(), -1);
  parent[source] = source;

  queue<int> q;
  q.push(source);

  random_device rd;
  mt19937 g(rd());

  while (!q.empty()) {
    int u = q.front();
    q.pop();

    // Randomize the order of adjacent nodes
    vector<int> indices(getAdj(u).size());
    for (int i = 0; i < indices.size(); ++i) {
      indices[i] = i;
    }
    shuffle(indices.begin(), indices.end(), g);

    for (int i : indices) {
      const Edge &e = getAdj(u)[i];
      if (parent[e.to] == -1 && e.capacity > 0) {
        parent[e.to] = u;
        if (e.to == sink) {
          return true; // Sink reached
        }
        q.push(e.to);
      }
    }
  }

  return false; // Sink not reached
}

double fordFulkerson(int s, int t) {
  double maxFlow = 0;
  vector<int> parent(graphSize());
  while (true) {
    fill(parent.begin(), parent.end(), -1);
    parent[s] = s;
    if (!bfs(s, t, parent))
      break;

    double pathFlow = 1e9;
    for (int v = t; v != s; v = parent[v]) {
      int u = parent[v];
      for (const Edge &e : getAdj(u)) {
        if (e.to == v) {
          pathFlow = min(pathFlow, e.capacity);
          break;
        }
      }
    }

    for (int v = t; v != s; v = parent[v]) {
      int u = parent[v];
      for (Edge &e : modifiableGetAdj(u)) {
        if (e.to == v) {
          e.capacity -= pathFlow;
          modifiableGetAdj(v)[e.rev].capacity += pathFlow;
          break;
        }
      }
    }

    maxFlow += pathFlow;
  }
  return maxFlow;
}
struct Professor {
  int id;
  double maxLoad;
  double minLoad;
  vector<int> preferredCourses;
  vector<int> nodeIndices;
};

struct Course {
  int id; // Course ID
};

void buildNetwork(const vector<int> &courses, vector<Professor> &professors) {
  int source = 0;             // Source node index
  int sink = graphSize() - 1; // Sink node index
  int nextNodeIndex = 1;      // Start indexing after the source node

  // Add nodes and edges for courses
  for (int courseID = 0; courseID < courses.size(); courseID++) {
    addEdge(source, nextNodeIndex,
            1); // Connect source to courses with capacity 1
    nextNodeIndex++;
  }
  // Add nodes and edges for professors
  for (auto &prof : professors) {
    int numNodes = static_cast<int>(ceil(prof.maxLoad / 0.5));

    // Clear any existing indices (in case of re-building the network)
    prof.nodeIndices.clear();

    // Create nodes for each 0.5 load unit and connect to sink
    for (int i = 0; i < numNodes; ++i) {
      addEdge(nextNodeIndex, sink, 0.5);
      prof.nodeIndices.push_back(nextNodeIndex);
      nextNodeIndex++;
    }

    // Connect preferred courses to professor nodes
    for (int courseId : prof.preferredCourses) {
      for (int profNodeIndex : prof.nodeIndices) {
        addEdge(courseId, profNodeIndex, 0.5);
      }
    }
  }
}

double calculateFlowIntoNode(int nodeIndex) {
  double totalFlow = 0.0;

  // Iterate over all edges leading to 'nodeIndex'
  for (const Edge &edge : getAdj(nodeIndex)) {
    // In the residual graph, the flow on an edge is the capacity of the reverse
    // edge
    const Edge &reverseEdge = getAdj(edge.to)[edge.rev];
    totalFlow += reverseEdge.capacity;
  }

  return totalFlow;
}

map<int, int> countUtilizedEdges(const vector<Professor> &professors,
                                 int sinkIndex) {
  map<int, int> utilizedEdgesCount;

  for (const auto &prof : professors) {
    int count = 0;
    for (int nodeIndex : prof.nodeIndices) {
      // Check the edge from professor node to sink
      for (const Edge &edge : getAdj(nodeIndex)) {
        if (edge.to == sinkIndex) {
          // Check if the edge is fully utilized (original capacity 0.5 fully
          // used)
          const Edge &reverseEdge = getAdj(edge.to)[edge.rev];
          if (reverseEdge.capacity == 0.5) {
            count++;
            break; // Move to the next node (since each node represents a part
                   // of the load)
          }
        }
      }
    }
    utilizedEdgesCount[prof.id] = count;
  }

  return utilizedEdgesCount;
}

void printUtilizedEdges(const map<int, int> &utilizedEdgesCount) {
  for (const auto &entry : utilizedEdgesCount) {
    cout << "Professor ID: " << entry.first
         << " - Utilized Parts: " << entry.second << endl;
  }
}

void assignAndPrintNewCourses(map<int, int> &utilizedEdgesCount,
                              const vector<Professor> &professors) {
  vector<int> onePartProfessors; // Professors with minLoad = maxLoad = 1 and
                                 // one utilized part
  vector<int> oneTo1_5PartProfessors; // Professors with minLoad = 1, maxLoad
                                      // = 1.5 and one or two utilized parts
  int newCourseIndex = 1;             // For NEW course naming
  vector<int> halfPartProfessors;

  for (const auto &prof : professors) {
    int utilizedParts = utilizedEdgesCount.at(prof.id);

    if (prof.minLoad == 1 && prof.maxLoad == 1) {
      if (utilizedParts == 1) {
        onePartProfessors.push_back(prof.id);
      } else if (utilizedParts == 0) {
        cout << "Professor ID " << prof.id << " is assigned a new course NEW"
             << newCourseIndex++ << endl;
        utilizedEdgesCount.at(prof.id) += 2;
      }
    } else if (prof.minLoad == 1 && prof.maxLoad == 1.5 &&
               (utilizedParts == 1 || utilizedParts == 0 ||
                utilizedParts == 2)) {
      oneTo1_5PartProfessors.push_back(prof.id);
    } else if (prof.minLoad == 0.5 && prof.maxLoad == 0.5 &&
               utilizedParts == 0) {
      halfPartProfessors.push_back(prof.id);
    }
  }
  // start
  for (int i = 0; i < halfPartProfessors.size(); i++) {
    if (i + 1 < halfPartProfessors.size()) {
      cout << "Professors ID " << halfPartProfessors[i] << " and "
           << halfPartProfessors[i + 1];
      cout << " are both assigned a new course NEW" << newCourseIndex++ << endl;
      utilizedEdgesCount.at(halfPartProfessors[i]) += 1;
      utilizedEdgesCount.at(halfPartProfessors[i + 1]) += 1;

      i++; // Incrementing to skip the paired professor
    } else {
      // Attempt to pair with a professor from the 1-1.5 category
      if (!onePartProfessors.empty()) {
        int pairedProf = onePartProfessors.back();
        onePartProfessors.pop_back();
        cout << "Professors ID " << halfPartProfessors[i] << " and "
             << pairedProf;
        cout << " are both assigned a new course NEW" << newCourseIndex++
             << endl;

        utilizedEdgesCount.at(halfPartProfessors[i]) += 1;
        utilizedEdgesCount.at(pairedProf) += 1;

      } else if (!oneTo1_5PartProfessors.empty()) {
        int pairedProf = oneTo1_5PartProfessors.back();
        oneTo1_5PartProfessors.pop_back();
        cout << "Professors ID " << halfPartProfessors[i] << " and "
             << pairedProf;
        cout << " are both assigned a new course NEW" << newCourseIndex++
             << endl;

        utilizedEdgesCount.at(halfPartProfessors[i]) += 1;
        utilizedEdgesCount.at(pairedProf) += 1;

      } else {
        // No eligible professor found for pairing
        cout << "CRASH TEST. Constraints couldnt be satisfied." << endl;
      }
    }
  }
  // end

  // Pair professors with one utilized part
  for (int i = 0; i < onePartProfessors.size(); i++) {
    if (i + 1 < onePartProfessors.size()) {
      cout << "Professors ID " << onePartProfessors[i] << " and "
           << onePartProfessors[i + 1];
      cout << " are both assigned a new course NEW" << newCourseIndex++ << endl;
      utilizedEdgesCount.at(onePartProfessors[i]) += 1;
      utilizedEdgesCount.at(onePartProfessors[i + 1]) += 1;

      i++; // Increment to skip the paired professor
    } else {
      // Attempt to pair with a professor from the 1-1.5 category
      if (!oneTo1_5PartProfessors.empty()) {
        int pairedProf = oneTo1_5PartProfessors.back();
        oneTo1_5PartProfessors.pop_back();
        cout << "Professors ID " << onePartProfessors[i] << " and "
             << pairedProf;
        cout << " are both assigned a new course NEW" << newCourseIndex++
             << endl;

        utilizedEdgesCount.at(onePartProfessors[i]) += 1;
        utilizedEdgesCount.at(pairedProf) += 1;

      } else {
        // No eligible professor found for pairing
        cout << "CRASH TEST. Constraints couldnt be satisfied." << endl;
      }
    }
  }

  // Assign new full courses to professors in the 1-1.5 category with 0 or 1
  // utilized part
  for (int profId : oneTo1_5PartProfessors) {
    int x = utilizedEdgesCount.at(profId);
    if (x == 0 || x == 1) {
      cout << "Professor ID " << profId << " is assigned a new course NEW"
           << newCourseIndex++ << endl;
    }
  }
}
// csv function
vector<Professor> loadProfessorsFromCSV(const string &filename) {
  vector<Professor> professors;
  ifstream file(filename);
  string line;

  while (getline(file, line)) {
    stringstream ss(line);
    string token;
    Professor prof;

    // Parse ID
    getline(ss, token, ',');
    prof.id = stoi(token);

    // Parse MaxLoad
    getline(ss, token, ',');
    prof.maxLoad = stod(token);

    // Parse MinLoad
    getline(ss, token, ',');
    prof.minLoad = stod(token);

    // Parse PreferredCourses
    while (getline(ss, token, ',')) {
      prof.preferredCourses.push_back(stoi(token));
    }

    // Add the populated professor struct to the vector
    professors.push_back(prof);
  }

  return professors;
}

int main() {
  // Example data - this should be replaced with actual data
  for (int i = 0; i < 10; i++) {
    vector<int> courses = {
        1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13,
        14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25}; // Ids of courses
    string filename = "prof.csv";
    vector<Professor> professors =
        loadProfessorsFromCSV(filename); // csv file input

    // Create a graph with enough nodes for source, sink, all courses, and
    // professors
    int totalNodes =
        2 + courses.size() +
        professors.size() * 3; // Adjust multiplier as needed based on maxLoad
    initGraph(totalNodes);

    // Build the network
    buildNetwork(courses, professors);

    // Run the Ford-Fulkerson algorithm to find the maximum flow
    int sourceNodeIndex = 0;

    int sinkNodeIndex = totalNodes - 1;
    double maxFlow = fordFulkerson(sourceNodeIndex, sinkNodeIndex);

    cout << "Maximum Flow: " << maxFlow << endl;
    int count = courses.size();
    if (maxFlow < count) {
      cout << "CRASH TEST Professors are less in number, can't assign all "
              "courses."
           << endl;
      return 0;
    }
    map<int, vector<int>> courseToProfessorMapping;
    for (int course : courses) {
      // Get the adjacency list for the course node
      const vector<Edge> &courseEdges = getAdj(course);

      for (const auto &prof : professors) {
        for (int profNode : prof.nodeIndices) {
          // Check if there is an edge from this course to this professor node
          auto it =
              find_if(courseEdges.begin(), courseEdges.end(),
                      [profNode](const Edge &e) { return e.to == profNode; });

          if (it != courseEdges.end()) {
            // Edge exists, now check its reverse edge
            const Edge &reverseEdge = getAdj(it->to)[it->rev];

            // Check if the reverse edge has capacity used (indicating flow was
            // sent through the forward edge)
            if (reverseEdge.capacity >
                0) { // Indicates flow was sent through the forward edge
              courseToProfessorMapping[course].push_back(prof.id);
              break; // Assuming one course can be assigned to at most one
                     // professor
            }
          }
        }
      }
    }

    map<int, int> utilizedEdges = countUtilizedEdges(professors, sinkNodeIndex);
    printUtilizedEdges(utilizedEdges);

    cout << "Course to Professor Mapping:" << endl;
    for (const auto &mapping : courseToProfessorMapping) {
      cout << "Course " << mapping.first << " assigned to Professor(s): ";
      for (int profId : mapping.second) {
        cout << profId << " ";
      }
      cout << endl;
    }
    assignAndPrintNewCourses(utilizedEdges, professors);
  }

  return 0;
}
