#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <chrono>
#include <windows.h>
#include <psapi.h>

using namespace std;

const int delta = 30;
const int alpha[4][4] = {
    {0, 110, 48, 94},
    {110, 0, 118, 48},
    {48, 118, 0, 110},
    {94, 48, 110, 0}
};

int get_index(char c) {
    if (c == 'A') return 0;
    if (c == 'C') return 1;
    if (c == 'G') return 2;
    if (c == 'T') return 3;
    return -1;
}

string generate_string(string base, const vector<int>& indices) {
    string s = base;
    for (int idx : indices) {
        s = s.substr(0, idx + 1) + s + s.substr(idx + 1);
    }
    return s;
}

pair<string, string> parse_input(const string& filepath, string& s1, string& s2) {
    ifstream file(filepath);
    string line;
    vector<string> lines;
    while (getline(file, line)) {
        if (!line.empty()) lines.push_back(line);
    }

    string base1 = lines[0];
    vector<int> idx1;
    int i = 1;
    while (i < lines.size() && lines[i].find_first_not_of("0123456789") == string::npos)
        idx1.push_back(stoi(lines[i++]));

    string base2 = lines[i++];
    vector<int> idx2;
    while (i < lines.size())
        idx2.push_back(stoi(lines[i++]));

    s1 = generate_string(base1, idx1);
    s2 = generate_string(base2, idx2);
    return {s1, s2};
}

tuple<int, string, string> align(const string& x, const string& y) {
    int m = x.size(), n = y.size();
    vector<vector<int>> dp(m + 1, vector<int>(n + 1));

    for (int i = 0; i <= m; ++i) dp[i][0] = i * delta;
    for (int j = 0; j <= n; ++j) dp[0][j] = j * delta;

    for (int i = 1; i <= m; ++i) {
        for (int j = 1; j <= n; ++j) {
            int match = dp[i-1][j-1] + alpha[get_index(x[i-1])][get_index(y[j-1])];
            int del = dp[i-1][j] + delta;
            int ins = dp[i][j-1] + delta;
            dp[i][j] = std::min(std::min(match, del), ins);

        }
    }

    string aligned_x = "", aligned_y = "";
    int i = m, j = n;
    while (i > 0 && j > 0) {
        int score = dp[i][j];
        int diag = dp[i-1][j-1];
        int up = dp[i-1][j];
        int left = dp[i][j-1];

        if (score == diag + alpha[get_index(x[i-1])][get_index(y[j-1])]) {
            aligned_x = x[i-1] + aligned_x;
            aligned_y = y[j-1] + aligned_y;
            i--; j--;
        } else if (score == up + delta) {
            aligned_x = x[i-1] + aligned_x;
            aligned_y = "_" + aligned_y;
            i--;
        } else {
            aligned_x = "_" + aligned_x;
            aligned_y = y[j-1] + aligned_y;
            j--;
        }
    }

    while (i > 0) {
        aligned_x = x[i-1] + aligned_x;
        aligned_y = "_" + aligned_y;
        i--;
    }
    while (j > 0) {
        aligned_x = "_" + aligned_x;
        aligned_y = y[j-1] + aligned_y;
        j--;
    }

    return {dp[m][n], aligned_x, aligned_y};
}

long getMemoryUsage() {
    PROCESS_MEMORY_COUNTERS memCounter;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &memCounter, sizeof(memCounter))) {
        return memCounter.PeakWorkingSetSize / 1024;
    }
    return -1;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Usage: ./basic <input_file> <output_file>\n";
        return 1;
    }

    string s1, s2;
    parse_input(argv[1], s1, s2);

    auto start = chrono::high_resolution_clock::now();
    auto [cost, aligned1, aligned2] = align(s1, s2);
    auto end = chrono::high_resolution_clock::now();

    double time_ms = chrono::duration<double, std::milli>(end - start).count();
    long memory_kb = getMemoryUsage();

    ofstream out(argv[2]);
    out << cost << "\n" << aligned1 << "\n" << aligned2 << "\n" << time_ms << "\n" << memory_kb << "\n";
    return 0;
}
