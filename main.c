#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <math.h>

#define OUTPUT_FILE_PATH "output.ppm"

#define WIDTH  1000
#define HEIGHT 1000
#define SEEDS_COUNT 50

#define COLOR_WHITE 0xFFFFFFFF
#define COLOR_RED   0xFF0000FF
#define COLOR_GREEN 0xFF00FF00
#define COLOR_BLUE  0xFFFF0000
#define COLOR_BLACK 0xFF000000
#define COLOR_BACKGROUND 0xFF201717

#define SEED_MARKER_RADIUS 4
#define SEED_MARKER_COLOR COLOR_BLACK

typedef uint32_t Color;
typedef struct {
    int x, y;
} Vec2;

static Color image[HEIGHT][WIDTH];
static Vec2 seeds[SEEDS_COUNT];


/**
 * @brief Fill the image with a specified color
 * 
 * @param color 
 * @return * Fill 
 */
void FillImage(Color color) 
{
    for (size_t y = 0; y < HEIGHT; ++y) {
        for (size_t x = 0; x < WIDTH; ++x) {
            image[x][y] = color;
        }
    }
}

/**
 * @brief Save image at specified path
 * 
 * @param filePath 
 * @return * Save 
 */
void SaveImageAsPPM(const char *filePath) 
{
    FILE *file = fopen(filePath, "wb");
    fprintf(file, "P6\n");
    fprintf(file, "%d %d 255\n", WIDTH, HEIGHT);

    if (file == NULL) {
        fprintf(stderr, "ERROR: cannot write into file %s: %s\n", filePath, strerror(errno));
        exit(1);
    }

    for (size_t y = 0; y < HEIGHT; ++y) {
        for (size_t x = 0; x < WIDTH; ++x) {
            Color pixel = image[y][x];

            uint8_t bytes[3] = {
                (uint8_t)((pixel&0x0000FF) >> 8 * 0),
                (uint8_t)((pixel&0x00FF00) >> 8 * 1),
                (uint8_t)((pixel&0xFF0000) >> 8 * 2)
            };
            
            fwrite(bytes, sizeof(bytes), 1, file);
            assert(!ferror(file));
        }
    }

    int err = fclose(file);
    assert(err == 0);
}

/**
 * @brief Get the square of the euclidean distance between two points
 * 
 * @param a 
 * @param b 
 * @return * Get 
 */
int EuclideanDistance(Vec2 pointA, Vec2 pointB) 
{
    int dx = pointA.x - pointB.x;
    int dy = pointA.y - pointB.y;

    return dx * dx + dy * dy;
}

/**
 * @brief Get the manhattan distance between two points
 * 
 * @param pointA 
 * @param pointB 
 * @return int 
 */
int ManhattanDistance(Vec2 pointA, Vec2 pointB) 
{
    int dx = abs(pointA.x - pointB.x);
    int dy = abs(pointA.y - pointB.y);

    return dx + dy;
}

/**
 * @brief Fill a circle with a specified radius at the specified origin with a specified color
 * 
 * @param origin 
 * @param radius 
 * @param color 
 * @return * Fill 
 */
void FillCircle(Vec2 origin, int radius, Color color) 
{
    Vec2 beginCorner = {origin.x - radius, origin.y - radius};
    Vec2 endCorner = {origin.x + radius, origin.y + radius};

    for (int x = beginCorner.x; x < endCorner.x; ++x) {
        if (!(0 <= x && x < WIDTH)) {
            continue;
        }
        for (int y = beginCorner.y; y < endCorner.y; ++y) {
            if (!(0 <= y && y < HEIGHT)) {
                continue;
            }

            Vec2 point = {x, y};
            if (SquareDistance(origin, point) <= radius * radius) {
                image[y][x] = color;
            }
        }
    }
}

/**
 * @brief Generate random seeds for Voronoi
 * 
 * @return * Generate 
 */
void GenerateRandomSeeds() 
{
    for (size_t i = 0; i < SEEDS_COUNT; ++i) {
        seeds[i].x = rand() % WIDTH;
        seeds[i].y = rand() % HEIGHT;
    }
}

/**
 * @brief Render the seed markers in the image
 * 
 * @return * Render 
 */
void  RenderSeedMarkers() 
{
    for (size_t i = 0; i < SEEDS_COUNT; ++i) {
        FillCircle(seeds[i], SEED_MARKER_RADIUS, SEED_MARKER_COLOR);
    }
}

/**
 * @brief Generate a color based on the position of a specified point
 * 
 * @param point 
 * @return * Generate 
 */
Color SeedToColor(Vec2 point) 
{
    assert(point.x >= 0);
    assert(point.y >= 0);
    assert(point.x < (1 << 16));
    assert(point.y < (1 << 16));

    uint16_t lf = point.x;
    uint16_t rg = point.y;

    return ((lf << 16) ^ rg);
}

/**
 * @brief Generate the Voronoi algorithm and render it
 * 
 * @return * Generate 
 */
void RenderVoronoi()
{
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            int closestSeedIdx = 0;
            Vec2 point = {x, y};

            for (size_t i = 1; i < SEEDS_COUNT; ++i) {
                int currDist = SquareDistance(seeds[i], point);
                int closestDist = SquareDistance(seeds[closestSeedIdx], point);

                if (currDist < closestDist) {
                    closestSeedIdx = i;
                }
            }

            Vec2 seedPos = {seeds[closestSeedIdx].x, seeds[closestSeedIdx].y};
            image[y][x] = SeedToColor(seedPos);
        }
    }
}

int main(void) 
{
    srand(time(0));
    FillImage(COLOR_BACKGROUND);
    GenerateRandomSeeds();
    RenderVoronoi();
    RenderSeedMarkers();
    SaveImageAsPPM(OUTPUT_FILE_PATH);
    return 0;
} 