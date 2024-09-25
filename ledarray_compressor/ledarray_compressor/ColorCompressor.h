#pragma once
#include <cmath>
#include <limits>
#include <cstdlib>
#include <vector>

class ColorCompressor
{
private:

    static double distance(const unsigned char* color, const unsigned char* other)  {
        return sqrt(pow(color[0] - other[0], 2) + pow(color[1] - other[1], 2) + pow(color[2] - other[2], 2));
    }

    static unsigned char getIndex(const unsigned char* color)
    {
        double min = DBL_MAX;
        unsigned char minIndex = 0;
        unsigned char i = 0;
        do {
            double curr = distance(color, palette[i]);
            if (curr < min) {
                minIndex = i;
                min = curr;
            }
            i++;
        } while (i != 255);
        return minIndex;
    }

public:

    static std::vector<char> convertColorsToIndices(const std::vector<char>& colors) {
        std::vector<char> indices;
        const unsigned char* colorsArr = (unsigned char*)colors.data();
        for (int i = 0; i < colors.size(); i+=3) {
            indices.push_back((char)getIndex(colorsArr + i));
        }
        return indices;
    }

    static unsigned char palette[256][3];
};