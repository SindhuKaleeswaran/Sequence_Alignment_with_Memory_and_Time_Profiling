#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <windows.h>
#include <psapi.h>

using namespace std;

const int GAP_PENALTY = 30;
const int ALPHA[4][4] = {
    {0, 110, 48, 94},
    {110, 0, 118, 48},
    {48, 118, 0, 110},
    {94, 48, 110, 0}
};

int get_index(char c) {
    switch (c) {
        case 'A': return 0;
        case 'C': return 1;
        case 'G': return 2;
        case 'T': return 3;
        default: return -1;
    }
}

string generate_string(string base, const vector<int>& indices) {
    string s = base;
    for (int idx : indices) {
        s = s.substr(0, idx + 1) + s + s.substr(idx + 1);
    }
    return s;
}

bool is_base_string(const string& line) {
    return line.find_first_not_of("ACGT") == string::npos;
}

pair<string, string> parse_input(const string& filepath, string& s1, string& s2) {
    ifstream file(filepath);
    string line;
    vector<string> lines;
    while (getline(file, line)) {
        if (!line.empty()) lines.push_back(line);
    }

    string base1 = lines[0];
    vector<int> insert1, insert2;
    int i = 1;
    while (i < lines.size() && !is_base_string(lines[i])) {
        insert1.push_back(stoi(lines[i++]));
    }
    string base2 = lines[i++];
    while (i < lines.size()) {
        insert2.push_back(stoi(lines[i++]));
    }

    s1 = generate_string(base1, insert1);
    s2 = generate_string(base2, insert2);
    return {s1, s2};
}

vector<int> space_efficient_alignment(const string& X, const string& Y, bool reverse = false) {
    int m = X.length(), n = Y.length();
    vector<vector<int>> dp(2, vector<int>(n + 1));

    for (int j = 0; j <= n; ++j) {
        dp[0][j] = j * GAP_PENALTY;
    }

    for (int i = 1; i <= m; ++i) {
        dp[1][0] = i * GAP_PENALTY;
        for (int j = 1; j <= n; ++j) {
            char a = reverse ? X[m - i] : X[i - 1];
            char b = reverse ? Y[n - j] : Y[j - 1];
            int cost = ALPHA[get_index(a)][get_index(b)];
            dp[1][j] = min({
                dp[0][j - 1] + cost,
                dp[0][j] + GAP_PENALTY,
                dp[1][j - 1] + GAP_PENALTY
            });
        }
        dp[0] = dp[1];
    }
    return dp[1];
}

pair<string, string> basic_alignment(const string& X, const string& Y, int& total_cost) {
    int m = X.size(), n = Y.size();
    vector<vector<int>> dp(m + 1, vector<int>(n + 1));

    for (int i = 0; i <= m; ++i) dp[i][0] = i * GAP_PENALTY;
    for (int j = 0; j <= n; ++j) dp[0][j] = j * GAP_PENALTY;

    for (int i = 1; i <= m; ++i) {
        for (int j = 1; j <= n; ++j) {
            int cost = ALPHA[get_index(X[i - 1])][get_index(Y[j - 1])];
            dp[i][j] = min({
                dp[i - 1][j - 1] + cost,
                dp[i - 1][j] + GAP_PENALTY,
                dp[i][j - 1] + GAP_PENALTY
            });
        }
    }

    string aligned_X = "", aligned_Y = "";
    int i = m, j = n;
    while (i > 0 && j > 0) {
        int cost = ALPHA[get_index(X[i - 1])][get_index(Y[j - 1])];
        if (dp[i][j] == dp[i - 1][j - 1] + cost) {
            aligned_X = X[i - 1] + aligned_X;
            aligned_Y = Y[j - 1] + aligned_Y;
            i--, j--;
        } else if (dp[i][j] == dp[i - 1][j] + GAP_PENALTY) {
            aligned_X = X[i - 1] + aligned_X;
            aligned_Y = "_" + aligned_Y;
            i--;
        } else {
            aligned_X = "_" + aligned_X;
            aligned_Y = Y[j - 1] + aligned_Y;
            j--;
        }
    }

    while (i > 0) {
        aligned_X = X[i - 1] + aligned_X;
        aligned_Y = "_" + aligned_Y;
        i--;
    }
    while (j > 0) {
        aligned_X = "_" + aligned_X;
        aligned_Y = Y[j - 1] + aligned_Y;
        j--;
    }

    total_cost = dp[m][n];
    return {aligned_X, aligned_Y};
}

tuple<string, string, int> divide_and_conquer(const string& X, const string& Y) {
    int m = X.size(), n = Y.size();
    if (m == 0) return {string(n, '_'), Y, n * GAP_PENALTY};
    if (n == 0) return {X, string(m, '_'), m * GAP_PENALTY};
    if (m == 1 || n == 1) {
        int cost;
        auto [ax, ay] = basic_alignment(X, Y, cost);
        return {ax, ay, cost};
    }

    int mid = m / 2;
    vector<int> left = space_efficient_alignment(X.substr(0, mid), Y, false);
    vector<int> right = space_efficient_alignment(X.substr(mid), Y, true);

    int min_cost = INT32_MAX, split = 0;
    for (int i = 0; i <= n; ++i) {
        int cost = left[i] + right[n - i];
        if (cost < min_cost) {
            min_cost = cost;
            split = i;
        }
    }

    auto [l1, l2, c1] = divide_and_conquer(X.substr(0, mid), Y.substr(0, split));
    auto [r1, r2, c2] = divide_and_conquer(X.substr(mid), Y.substr(split));
    return {l1 + r1, l2 + r2, c1 + c2};
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
        cerr << "Usage: ./efficient input.txt output.txt\n";
        return 1;
    }

    string s1, s2;
    parse_input(argv[1], s1, s2);

    auto start = chrono::high_resolution_clock::now();
    auto [aligned1, aligned2, cost] = divide_and_conquer(s1, s2);
    auto end = chrono::high_resolution_clock::now();

    double time_taken = chrono::duration<double, milli>(end - start).count();
    long memory_kb = getMemoryUsage();

    ofstream out(argv[2]);
    out << cost << "\n";
    out << aligned1 << "\n";
    out << aligned2 << "\n";
    out << time_taken << "\n";
    out << memory_kb << "\n";
    return 0;
}
