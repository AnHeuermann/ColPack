#include "ColPackHeaders.h"
#include "SMPGCColoring.h"

#include <iostream>

using namespace ColPack;

int main()
{
    const int rowCount = 5;
    // Row i: number of non-zeros followed by their column indices.
    unsigned int pattern[rowCount][3] = {
        {1, 1, 0}, {2, 0, 2}, {2, 1, 3}, {2, 2, 4}, {1, 3, 0}};
    unsigned int *rows[rowCount];
    for (int i = 0; i < rowCount; i++) rows[i] = pattern[i];

    GraphColoringInterface g(SRC_MEM_ADOLC, rows, rowCount);
    g.Coloring("SMALLEST_LAST", "STAR");

    int seedRows = 0, seedColumns = 0;
    double **seed = g.GetSeedMatrix(&seedRows, &seedColumns);
    std::cout << "colors: " << g.GetVertexColorCount() << ", seed matrix: "
              << seedRows << "x" << seedColumns << std::endl;

    HessianRecovery recovery;
    return (seed != nullptr && seedRows == rowCount && seedColumns > 0) ? 0 : 1;
}
