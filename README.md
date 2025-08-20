# Sequence_Alignment_with_Memory_and_Time_Profiling

This program implements an optimized version of the Needleman-Wunsch global sequence alignment algorithm using a divide and conquer approach and space-efficient dynamic programming techniques. The algorithm aligns two sequences (e.g., DNA or protein sequences) and calculates the optimal alignment score, along with the aligned sequences, while profiling execution time and memory usage.

Features:

Divide and Conquer: The program divides the problem into smaller subproblems for better performance, especially for large sequences.

Space-Efficient Dynamic Programming: Instead of using a full 2D matrix for dynamic programming, the program reduces memory usage by keeping only two rows in memory at any given time.

Profiling: Measures execution time in milliseconds and memory usage in kilobytes.

Gap Penalty and Substitution Matrix: Supports customizable gap penalty and substitution matrices (for nucleotide sequences like DNA).
