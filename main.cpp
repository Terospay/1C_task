
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <limits>
#include <utility>
#include <algorithm>


uint32_t INF = std::numeric_limits<uint32_t>::max();


class Branch {
public:
    void AddStation(std::string station_name) {
        stations.push_back(station_name);
    }

    void AddTime(uint32_t minutes) {
        time_between_station.push_back(minutes);
    }

    void SetInterval(uint32_t minutes) {
        interval_between_trains = minutes;
    }

    std::vector<std::string> GetStation() const {
        return stations;
    }

    std::vector<uint32_t> GetTimeBetweenStation() const {
        return time_between_station;
    }

    uint32_t GetInterval() const {
        return interval_between_trains;
    }

    void AddTimeFromStart(uint32_t time) {
        time_from_start.push_back(time);
    }

    std::vector<uint32_t> GetTimeFromStart() const {
        return time_from_start;
    }

    uint32_t GetNumber() {
        return number;
    }

    void SetNumber(uint32_t new_number) {
        number = new_number;
    }

private:
    uint32_t number = 0;
    std::vector<std::string> stations;
    std::vector<uint32_t> time_between_station;
    uint32_t interval_between_trains = 0;
    std::vector<uint32_t> time_from_start;

};

class Subway {
public:
    void AddBranch(Branch branch_name) {
        branches.push_back(branch_name);
    }

    uint32_t size() {
        return real_count_station;
    }

    void CreateMap();

    std::pair<std::vector<std::string>, uint32_t> Request(std::string from, std::string to, uint32_t time_come);

    uint32_t LengthCalculate(uint32_t from, uint32_t to, uint32_t time_come, std::vector<uint32_t> parents);

private:
    std::vector<Branch> branches;
    std::unordered_map<std::string, uint32_t> name_in_number;   // for convenient work with a graph whose vertices are
    std::unordered_map<uint32_t, std::string> number_in_name;   // numbers
    std::unordered_map<uint32_t, Branch> branch_where_station;
    std::unordered_map<std::pair<Branch, std::string>, uint32_t> time_from_start;
    std::vector<std::vector<uint32_t>> graph;
    uint32_t real_count_station = 0;

};

void Subway::CreateMap() {

    uint32_t counter = 0;        // counting the number of stations without repetition and filling in the service fields
    for (auto branch : branches) {
        std::vector<std::string> stations = branch.GetStation();
        std::vector<uint32_t> time_to_station = branch.GetTimeFromStart();
        for (uint32_t i = 0; i < branch.GetStation().size(); ++i) {
            time_from_start[std::make_pair(branch, stations[i])] = time_to_station[i];
        }
        for (auto station : branch.GetStation()) {
            if (name_in_number.find(station) != name_in_number.end()) {
                name_in_number[station] = counter;
                number_in_name[counter] = station;
                branch_where_station[counter] = branch;
                ++counter;
            }
        }
    }
    graph = std::vector<std::vector<uint32_t>>(counter, std::vector<uint32_t>(counter, INF));
    for (auto branch : branches) {
        std::vector<std::string> stations = branch.GetStation();
        std::vector<uint32_t> times = branch.GetTimeBetweenStation();
        for (uint32_t i = 0; i < stations.size() - 1; ++i) {
            graph[name_in_number[stations[i]]][name_in_number[stations[i + 1]]] = times[i];
        }
    }
    real_count_station = counter;
}


// calculation of the distance between two stations, given that at this
// moment we can do a transfer and we will have to wait for the train
uint32_t Subway::LengthCalculate(uint32_t from, uint32_t to, uint32_t time_come, std::vector<uint32_t> parents) {
    if (branch_where_station[from].GetNumber() == branch_where_station[to].GetNumber()) {
        return graph[from][to];
    } else {
        Branch our_branch = branch_where_station[to];
        uint32_t interval = our_branch.GetInterval();
        uint32_t time_from_branch_beginning = time_from_start[std::make_pair(our_branch, number_in_name[to])];
        return graph[from][to] + ((time_come - time_from_branch_beginning) % interval);
    }
}


// Looking for distance using dijkstra
std::pair<std::vector<std::string>, uint32_t> Subway::Request(std::string from, std::string to, uint32_t time_come) {
    uint32_t start = name_in_number[from];
    uint32_t finish = name_in_number[to];
    std::vector<char> used(real_count_station, 0);
    std::vector<uint32_t> parent(real_count_station);
    parent[start] = start;
    std::vector<uint32_t> distance(real_count_station, INF);
    distance[start] = 0;
    for (uint32_t i = 0; i < size(); ++i) {
        uint32_t min_distance = -1;
        for (uint32_t j = 0; j < size(); ++j) {
            if (!used[j] && (min_distance == -1 || distance[j] < distance[min_distance])) {
                min_distance = j;
            }
        }
        if (distance[min_distance] == INF) {
            break;
        }
        used[min_distance] = true;
        for (uint32_t j = 0; j < size(); ++j) {
            if (graph[min_distance][j] != INF) {
                uint32_t length = LengthCalculate(min_distance, j, time_come, parent);
                if (distance[min_distance] + length < distance[j]) {
                    distance[j] = distance[min_distance] + length;
                    parent[j] = min_distance;
                }
            }
        }
    }
    uint32_t result_distance = distance[finish];
    if (result_distance == INF) {
        return std::make_pair(std::vector<std::string>(0), INF);
    } else {
        std::vector<std::string> path;
        for (int v = finish; v != start; v = parent[v]) {
            path.push_back(number_in_name[v]);
        }
        path.push_back(from);
        reverse(path.begin(), path.end());
        return std::make_pair(path, result_distance);
    }

}


int main() {
    uint32_t count_branches;
    std::cout << "Specify the number of branches: ";
    std::cin >> count_branches;
    Subway subway;
    for (uint32_t i = 0; i < count_branches; ++i) {
        Branch branch;
        branch.SetNumber(i);
        std::cout << "Enter the number of stations on this branch: ";
        uint32_t count_station;
        std::cin >> count_station;
        std::cout << "Enter the names of all stations: ";
        for (uint32_t j = 0; j < count_station; ++j) {
            std::string station_name;
            std::cin >> station_name;
            branch.AddStation(station_name);
        }
        uint32_t time_to_station = 0;
        for (uint32_t j = 0; j < count_station - 1; ++j) {
            std::cout << "Enter all the distances (how many minutes the journey takes): ";
            uint32_t minutes;
            std::cin >> minutes;
            branch.AddTime(minutes);
            branch.AddTimeFromStart(time_to_station);
            time_to_station += minutes;
        }
        std::cout << "Enter the interval between trains (in minutes):";
        uint32_t minutes;
        std::cin >> minutes;
        branch.SetInterval(minutes);
        subway.AddBranch(branch);
    }
    subway.CreateMap();
    std::cout << "Make a request: ";
    std::string from, to;
    uint32_t minute;
    std::cin >> from >> to >> minute;
    std::pair<std::vector<std::string>, uint32_t> ans = subway.Request(from, to, minute);
    if (ans.second == INF) {
        std::cout << "No way";
    } else {
        for (auto i : ans.first) {
            std::cout << i << " ";
        }
        std::cout << "\nLength : " << ans.second;
    }
    return 0;
}