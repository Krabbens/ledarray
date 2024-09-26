#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>
#include "lz4.h"
#include "ColorCompressor.h"

#define FRAMES_ONE_SPIN 30
#define DEFAULT_BLOCK_SIZE 400; // one frame
#define EXTENSION ".lz4"
#define BMP_HEADER_SIZE 54

int BLOCK_SIZE = DEFAULT_BLOCK_SIZE;
bool debug = false;

std::vector<char> merge_bmp_files(const std::string& folder_path, int spins) {
    std::vector<char> merged_data;

    for (int j = 1; j <= spins; ++j) {
        for (int i = 0; i < FRAMES_ONE_SPIN; ++i) {
            std::ostringstream filename_stream;
            filename_stream << std::setw(3) << std::setfill('0') << j << "_"
                << std::setw(2) << std::setfill('0') << i << ".bmp";

            std::string subfolder_path = folder_path + "/" + std::to_string(i);
            std::string bmp_file_path = subfolder_path + "/" + filename_stream.str();
            std::ifstream bmp_file(bmp_file_path, std::ios::binary);
            if (bmp_file) {
                // skip bmp header
                bmp_file.seekg(BMP_HEADER_SIZE);
                std::vector<char> file_data((std::istreambuf_iterator<char>(bmp_file)), std::istreambuf_iterator<char>());
                merged_data.insert(merged_data.end(), file_data.begin(), file_data.end());
            }
            else {
                std::cout << "Cannot open input file: " << bmp_file_path << std::endl;
                exit(1);
            }
        }
    }

    if (debug) {
        int pixel_count = 0; 
        for (int i = 0; i < 20; ++i) {
            for (int j = 0; j < 20; ++j) {
                int index = (i * 20 + j) * 3; 
                if (index + 2 < merged_data.size()) {
                    unsigned char blue = static_cast<unsigned char>(merged_data[index]);
                    unsigned char green = static_cast<unsigned char>(merged_data[index + 1]);
                    unsigned char red = static_cast<unsigned char>(merged_data[index + 2]);
                    std::cout << "(" << std::setw(3) << (int)red << ", "
                        << std::setw(3) << (int)green << ", "
                        << std::setw(3) << (int)blue << ") ";
                }
                else {
                    std::cout << "(---, ---, ---) ";
                }
                pixel_count++;
            }
            std::cout << std::endl;
        }
    }

    return merged_data;
}

std::vector<char> rotate(std::vector<char> vec) {
    const int block_size = 20;
    const int frame_size = block_size * block_size;

    std::vector<char> rotated(vec.size());

    for (int frame_i = 0; frame_i * frame_size < vec.size(); frame_i++) {
        int frame_n = frame_i * frame_size;

        for (int row_i = 0; row_i < block_size; row_i++) {
            for (int column_i = 0; column_i < block_size; column_i++) {
                // Calculate rotated positions
                int new_row = block_size - 1 - column_i;  // Flip the row to correct the upside-down issue
                int new_column = row_i;

                // Map the original index to the rotated index
                rotated[frame_n + new_row * block_size + new_column] = vec[frame_n + row_i * block_size + column_i];
            }
        }
    }

    return rotated;
}

void lz4Compress(const char* input, std::ofstream& outputFile) {

    char* compressed = new char[LZ4_compressBound(BLOCK_SIZE)];
    if (!compressed) {
        std::cout << "Memory allocation error" << std::endl;
        return;
    }

    short compressedSize = LZ4_compress_default(input, compressed, BLOCK_SIZE, LZ4_compressBound(BLOCK_SIZE));

    if (debug) {
        static int blockNum = 0; static int totalSize = 0; totalSize += compressedSize;
        std::cout << "[ " << blockNum++ << " ] " << "Compressed size: " << compressedSize << "\tTotal size: " << totalSize << std::endl;
    }

    if (compressedSize <= 0) {
        std::cout << "Compression error" << std::endl;
        delete[] compressed;
        return;
    }

    uint16_t size = static_cast<uint16_t>(compressedSize);
    outputFile.write(reinterpret_cast<const char*>(&size), sizeof(size));
    outputFile.write(compressed, compressedSize);
    if (debug) std::cout << "Current pos: " << outputFile.tellp() << std::endl;
    delete[] compressed;
}

void compressFile(const char* inputFilename, const char* outputFilename, int spins) {

    std::cout << "Merging files..." << std::endl;
    std::vector<char> inputVec = merge_bmp_files(inputFilename, spins);

    if (debug) {
        std::ofstream mergedFile(std::string(outputFilename) + "_merged_data.raw", std::ios::binary);
        if (!mergedFile) {
            std::cout << "Cannot open merged file for writing." << std::endl;
            exit(1);
        }
        mergedFile.write(inputVec.data(), inputVec.size());
        mergedFile.close();
    }

    std::cout << "Compressing... 1/2" << std::endl;
    std::vector<char> colorsVec = ColorCompressor::convertColorsToIndices(inputVec);
    inputVec.clear();

    if (debug) {
        std::ofstream colorCompressedFile(std::string(outputFilename) + "_color_compressed.raw", std::ios::binary);
        if (!colorCompressedFile) {
            std::cout << "Cannot open color-compressed file for writing." << std::endl;
            exit(1);
        }
        colorCompressedFile.write(colorsVec.data(), colorsVec.size());
        colorCompressedFile.close();
    }

    std::cout << "Rotating... " << std::endl;
    std::vector<char> rotated = rotate(colorsVec);
    colorsVec.clear();

    size_t totalBytes = rotated.size();
    char* input = rotated.data();
    size_t offset = 0;

    std::ofstream outputFile(outputFilename, std::ios::binary);
    if (!outputFile) {
        std::cout << "Cannot open output file" << std::endl;
        exit(1);
    }

    std::cout << "Compressing... 2/2" << std::endl;

    while (offset + BLOCK_SIZE <= totalBytes) {
        lz4Compress(input + offset, outputFile);
        offset += BLOCK_SIZE;
    }

    if (offset != totalBytes) {
        std::cout << "Error: Not all data has been processed. Please check if the block size is appropriate." << std::endl;
        exit(1);
    }

    if (debug) std::cout << "Final pos: " << outputFile.tellp() << std::endl;
    outputFile.close();

}

std::string changeExtension(const std::string& filename, const std::string& newExt) {
    size_t dotPosition = filename.find_last_of('.');
    if (dotPosition != std::string::npos) {
        return filename.substr(0, dotPosition) + newExt;
    }
    return filename + newExt;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " <dir name> <number of spins>" << std::endl;
        return 1;
    }
    if (argc == 4) {
        debug = true;
    }

    const char* inputDirectory = argv[1];
    int spins = std::atoi(argv[2]);

    size_t dirNameLen = strlen(inputDirectory);
    size_t extLen = strlen(EXTENSION);
    size_t outputFileNameLen = dirNameLen + extLen + 1;
    char* outputFilename = new char[outputFileNameLen];
    strcpy_s(outputFilename, outputFileNameLen, inputDirectory);
    strcat_s(outputFilename, outputFileNameLen, EXTENSION);

    compressFile(inputDirectory, outputFilename, spins);

    delete[] outputFilename;
    return 0;
}