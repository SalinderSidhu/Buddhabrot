#include "Buddhabrot.hpp"

// Initialize Buddhabrot class variables and allocate heatmaps
Buddhabrot::Buddhabrot(int width, int height, int samples, double minR,
    double maxR, double minI, double maxI, int redIters, int blueIters,
    int greenIters) {
    // Initialize instance variables
    imageWidth = width;
    imageHeight = height;
    redIterations = redIters;
    blueIterations = blueIters;
    greenIterations = greenIters;
    min = ComplexNumber(minR, minI);
    max = ComplexNumber(maxR, maxI);
    sampleCount = width * height * samples;
    maxHeatmapValue = 0;
    // Allocate heatmaps for each colour channel
    allocHeatmap(redHeatmap, imageWidth, imageHeight);
    allocHeatmap(blueHeatmap, imageWidth, imageHeight);
    allocHeatmap(greenHeatmap, imageWidth, imageHeight);
}

// Generate the Buddhabrot fracal data
void Buddhabrot::generate() {
    // Variables to keep track of completition
    ULLI iterCount = 0;
    ULLI totalIters = sampleCount * 3;
    // Generate heatmaps for each colour channel
    generateHeatmap(redHeatmap, imageWidth, imageHeight, min, max,
        redIterations, sampleCount, maxHeatmapValue, iterCount, totalIters);
    generateHeatmap(greenHeatmap, imageWidth, imageHeight, min, max,
        greenIterations, sampleCount, maxHeatmapValue, iterCount, totalIters);
    generateHeatmap(blueHeatmap, imageWidth, imageHeight, min, max,
        blueIterations, sampleCount, maxHeatmapValue, iterCount, totalIters);
    // Scale the heatmap for each colour channel
    for (int row = 0; row < imageHeight; ++row) {
        for (int col = 0; col < imageWidth; ++col) {
            redHeatmap[row][col] = colourFromHeatmap(redHeatmap[row][col],
                maxHeatmapValue, 255);
            greenHeatmap[row][col] = colourFromHeatmap(greenHeatmap[row][col],
                maxHeatmapValue, 255);
            blueHeatmap[row][col] = colourFromHeatmap(blueHeatmap[row][col],
                maxHeatmapValue, 255);
        }
    }
}

// Write the Buddhabrot fracal data to a file in PPM format
void Buddhabrot::flushToPPMFile(ofstream& outputFile) {
    // Write the PPM file header
    outputFile << "P3" << endl;
    outputFile << imageWidth << " " << imageHeight << endl;
    outputFile << 255 << endl;
    // Write the heatmaps to the PPM file and close the file
    for (int row = 0; row < imageHeight; ++row) {
        for (int col = 0; col < imageWidth; ++col) {
            outputFile << redHeatmap[row][col] << " ";
            outputFile << greenHeatmap[row][col] << " ";
            outputFile << blueHeatmap[row][col] << "   ";
        }
        outputFile << endl;
    }
    // Free memory used by the heatmaps
    freeHeatmap(redHeatmap, imageHeight);
    freeHeatmap(blueHeatmap, imageHeight);
    freeHeatmap(greenHeatmap, imageHeight);
}

// Free memory used by a heatmap with a specific height
void Buddhabrot::freeHeatmap(UINT**& heatmap, int height) {
    // Delete and free memory used by the nested arrays of the heatmap pointer
    for (int i = 0; i < height; ++i) {
        delete[] heatmap[i];
        heatmap[i] = nullptr;
    }
    // Delete and free memory used by the heatmap pointer itself
    delete heatmap;
    heatmap = nullptr;
}

// Allocate a new heatmap of a specific width and height with default 0 value
void Buddhabrot::allocHeatmap(UINT**& heatmap, int width, int height) {
    heatmap = new UINT*[height];
    for (int i = 0; i < height; ++i) {
        heatmap[i] = new UINT[width];
        // Allocate each point in the heatmap with a default value of 0
        for (int j = 0; j < width; ++j) {
            heatmap[i][j] = 0;
        }
    }
}

// Return the scaled colour from heatmap using the maxHeatmapValue
int Buddhabrot::colourFromHeatmap(UINT heatmapValue, UINT maxHeatmapValue,
    int maxColour) {
    double scale = static_cast<double>(maxColour) / maxHeatmapValue;
    return static_cast<int>(heatmapValue * scale);
}

// Return the row value from the real component of a complex number
int Buddhabrot::rowFromR(double r, double minR, double maxR, int imageHeight) {
    return static_cast<int>((r - minR) * (imageHeight / (maxR - minR)));
}

// Return the column value from the imaginary component of a complex number
int Buddhabrot::colFromI(double i, double minI, double maxI, int imageWidth) {
    return static_cast<int>((i - minI) * (imageWidth / (maxI - minI)));
}

// Return a list of Buddhabrot points consisting of complex numbers
vector<ComplexNumber> Buddhabrot::buddhabrotPoints(const ComplexNumber& c,
    int numIters) {
    int n = 0;
    ComplexNumber z;
    // Generate a vector of complex numbers and reserve required space
    vector<ComplexNumber> outputPoints;
    outputPoints.reserve(numIters);
    // The iteration algorithm that generates the points to be counted
    while (n < numIters && z.squareMagnitude() <= 2.0) {
        z = z * z + c;
        ++n;
        outputPoints.push_back(z);
    }
    /* If the point remains bounded through numIters then the point is bounded
    in the Mandelbrot set and can be ignored */
    if (n == numIters) {
        return vector<ComplexNumber>();
    } else {
        return outputPoints;
    }
}

// Generate the Buddhabrot fractal heatmap
void Buddhabrot::generateHeatmap(UINT** heatmap, int imageWidth,
    int imageHeight, const ComplexNumber& min, const ComplexNumber& max,
    int numIters, ULLI numSamples, UINT& maxHeatmapValue, ULLI& iterCount,
    ULLI totalIters) {
    // Configure the random number generator, seed and uniform distributions
    mt19937 rng;
    uniform_real_distribution<double> rDistribution(min.r(), max.r());
    uniform_real_distribution<double> iDistribution(min.i(), max.i());
    rng.seed(static_cast<UINT>(high_resolution_clock::now().time_since_epoch()
		.count()));
    auto next = high_resolution_clock::now() + seconds(5);
    // Collect numSamples samples where sample is just a random Complex number
    for (ULLI sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex) {
        if (high_resolution_clock::now() > next) {
            next = high_resolution_clock::now() + seconds(30);
            // Display the estimated completition
            cout << "Estimated Completion: " << static_cast<float>(iterCount) /
                static_cast<float>(totalIters) * 100 << "%" << endl;
        }
        ++iterCount;
        // Each sample gets a list of points if it escapes to infinity
        ComplexNumber sample(rDistribution(rng), iDistribution(rng));
        vector<ComplexNumber> points = buddhabrotPoints(sample, numIters);
        for (ComplexNumber& point : points) {
            if (point.r() <= max.r() && point.r() >= min.r() &&
                point.i() <= max.i() && point.i() >= min.i()) {
                // Obtain the row and column and increment heatmap position
                int row = rowFromR(point.r(), min.r(), max.r(), imageHeight);
                int col = colFromI(point.i(), min.i(), max.i(), imageWidth);
                ++heatmap[row][col];
                // Record new maximum heatmap value
                if (heatmap[row][col] > maxHeatmapValue) {
                    maxHeatmapValue = heatmap[row][col];
                }
            }
        }
    }
}
