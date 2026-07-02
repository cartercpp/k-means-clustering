#include <iostream>
#include <iomanip>
#include <string_view>
#include <format>
#include <array>
#include <vector>
#include <utility>
#include <random>
#include <limits>
#include <chrono>
#include <thread>

int main()
{
    constexpr int rows = 80,
                  columns = 120;

    char grid[rows][columns];

    auto resetGrid = [&grid]() {
        for (int row = 0; row < rows; ++row)
            for (int column = 0; column < columns; ++column)
                grid[row][column] = ' ';
    };

    auto moveCursor = [](int row, int column) {
        std::cout << std::format("\033[{};{}H", row + 1, column + 1) << std::flush;
    };

    constexpr int K = 9;
    constexpr int pointsPerCluster = 250;
    constexpr std::array<char, K> clusterCharacters{'1', '2', '3', '4', '5', '6', '7', '8', '9'};
    constexpr std::array<std::string_view, K> clusterColors{
        "\033[31m", "\033[32m", "\033[33m", "\033[34m", "\033[35m", "\033[36m", "\033[37m", "\033[91m", "\033[41m"
    };

    std::array<std::pair<double, double>, K> centroids;
    std::vector<std::pair<double, double>> points;
    points.reserve(K * pointsPerCluster);

    std::random_device rd;
    std::uniform_real_distribution<double> xDist(0, columns - 1),
                                           yDist(0, rows - 1),
                                           radiusDist(1, (columns - 1) / 2.0),
                                           thetaDist(0, 3.14159 * 2);

    std::cout << "\033[?25l";
    std::cout << "\033[2J" << std::flush;

    for (int i = 0; i < K; ++i)
    {
        auto& [centerX, centerY] = centroids[i];

        centerX = xDist(rd);
        centerY = yDist(rd);

        for (int i2 = 0; i2 < pointsPerCluster; ++i2)
        {
            const double x = centerX + radiusDist(rd) * std::cos(thetaDist(rd)),
                         y = centerY + radiusDist(rd) * std::sin(thetaDist(rd));
            points.emplace_back(x, y);
        }
    }

    for (int iter = 0; iter < 50; ++iter)
    {
        std::array<std::pair<double, double>, K> newCentroids{};
        std::array<int, K> clusterPointCounts{};

        resetGrid();
        for (std::size_t pointIndex = 0; pointIndex < points.size(); ++pointIndex)
        {
            const auto [x, y] = points[pointIndex];

            int closestCentroidIndex = 0;
            double closestDistance = std::numeric_limits<double>::max();

            for (int centroidIndex = 0; centroidIndex < K; ++centroidIndex)
            {
                const auto [centroidX, centroidY] = centroids[centroidIndex];
                const double squaredDistance = (centroidX - x) * (centroidX - x) + (centroidY - y) * (centroidY - y);

                if (squaredDistance < closestDistance)
                {
                    closestCentroidIndex = centroidIndex;
                    closestDistance = squaredDistance;
                }
            }

            newCentroids[closestCentroidIndex].first += x;
            newCentroids[closestCentroidIndex].second += y;
            ++clusterPointCounts[closestCentroidIndex];

            const int row = rows - 1 - static_cast<int>(y),
                      column = static_cast<int>(x);

            if ((row >= 0) && (row < rows) && (column >= 0) && (column < columns))
                grid[row][column] = clusterCharacters[closestCentroidIndex];
        }

        for (int i = 0; i < K; ++i)
        {
            newCentroids[i].first /= clusterPointCounts[i];
            newCentroids[i].second /= clusterPointCounts[i];
        }

        moveCursor(0, 0);
        for (int row = 0; row < rows; ++row)
        {
            for (int column = 0; column < columns; ++column)
            {
                if (grid[row][column] != ' ')
                    for (int i = 0; i < K; ++i)
                        if (grid[row][column] == clusterCharacters[i])
                            std::cout << clusterColors[i];

                std::cout << grid[row][column];
                std::cout << "\033[0m" << std::flush;
            }

            std::cout << '\n' << std::flush;
        }

        std::swap(centroids, newCentroids);
        std::this_thread::sleep_for(std::chrono::milliseconds(80));
    }

    std::cout << "\033[0m";
    std::cout << "\033[?25h" << std::flush;
}