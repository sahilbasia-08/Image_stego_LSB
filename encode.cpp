// encode.cpp  — hide text inside images using LSB steganography
// Build:  g++ -std=c++17 encode.cpp `pkg-config --cflags --libs opencv4` -o encode

#include <opencv2/opencv.hpp>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
#include <termios.h>
#include <unistd.h>

namespace fs = std::filesystem;

std::string toBinary(uint8_t val)
{
    std::string out(8, '0');
    for (int i = 7; i >= 0; --i)
        out[7 - i] = ((val >> i) & 1) ? '1' : '0';
    return out;
}
std::string toBinary(const std::string &s)
{
    std::string bits;
    bits.reserve(s.size() * 8);
    for (unsigned char c : s)
        bits += toBinary(c);
    return bits;
}

std::string getPassword(const std::string &prompt)
{
    std::cout << prompt;
    termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    std::string pwd;
    std::getline(std::cin, pwd);
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    std::cout << '\n';
    return pwd;
}

void embedBits(cv::Mat &img, const std::string &bits)
{
    const size_t len = bits.size();
    size_t idx = 0;

    for (int r = 0; r < img.rows && idx < len; ++r)
        for (int c = 0; c < img.cols && idx < len; ++c)
        {
            cv::Vec3b &pix = img.at<cv::Vec3b>(r, c);
            for (int ch = 0; ch < 3 && idx < len; ++ch)
            {
                pix[ch] = (pix[ch] & 0xFE) | (bits[idx++] - '0');
            }
        }
}

/*  splitAndTag  ▸ identical logic to Python version but with safe slicing  */
std::vector<std::string> splitAndTag(const std::string &payload,
                                     int parts,
                                     const std::string &pwd)
{
    const int chunk = std::ceil(static_cast<double>(payload.size()) / parts);
    std::vector<std::string> tagged(parts);
    size_t pos = 0;

    for (int i = 0; i < parts; ++i)
    {
        /* safe slice: if pos ≥ payload.size() return empty string */
        std::string slice;
        if (pos < payload.size())
        {
            size_t remaining = payload.size() - pos;
            slice = payload.substr(pos, std::min(static_cast<size_t>(chunk), remaining));
        }
        pos += chunk;

        if (i == 0)
            slice += "@%#/" + pwd + "@%#/" + std::to_string(i) + "seq###";
        else
            slice += std::to_string(i) + "seq###";

        tagged[i] = toBinary(slice);
    }
    return tagged;
}

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        std::cerr << "Usage: ./encode <text_file> --path <images_directory>\n";
        return 1;
    }

    std::string textFile = argv[1];
    std::string dir = argv[3];

    /* read secret text */
    std::ifstream fin(textFile);
    if (!fin)
    {
        std::cerr << "Cannot open text file.\n";
        return 1;
    }
    std::string payload((std::istreambuf_iterator<char>(fin)),
                        std::istreambuf_iterator<char>());

    /* enumerate images */
    std::vector<fs::path> images;
    for (const auto &entry : fs::directory_iterator(dir))
        if (entry.is_regular_file())
            images.push_back(entry.path());

    if (images.empty())
    {
        std::cerr << "Directory has no files.\n";
        return 1;
    }
    if (images.size() > 9)
    {
        std::cerr << "Only 9 files allowed maximum.\n";
        return 1;
    }

    /* password */
    std::string pwd = getPassword("Set a password for encryption: ");
    std::string pwd2 = getPassword("Confirm your password: ");
    if (pwd != pwd2)
    {
        std::cerr << "Password does not match.\n";
        return 1;
    }

    /* split message */
    auto bitChunks = splitAndTag(payload, static_cast<int>(images.size()), pwd);

    /* make output folder */
    fs::path outDir = fs::path(dir) / "encodedImages";
    fs::create_directory(outDir);

    /* encode loop */
    for (size_t i = 0; i < images.size(); ++i)
    {
        std::cout << "Starting encoding in image " << images[i].filename() << "...\n";
        cv::Mat img = cv::imread(images[i].string(), cv::IMREAD_COLOR);
        if (img.empty())
        {
            std::cerr << "Cannot read " << images[i] << '\n';
            return 1;
        }

        const auto &bits = bitChunks[i];
        size_t maxBits = static_cast<size_t>(img.rows) * img.cols * 3;
        if (bits.size() > maxBits)
        {
            std::cerr << "Message chunk too big for " << images[i] << '\n';
            return 1;
        }

        embedBits(img, bits);

        fs::path outFile = outDir / (images[i].stem().string() + "_encoded" + std::to_string(i) + ".png");
        cv::imwrite(outFile.string(), img);
    }

    std::cout << ".\n.\n.\nEncoding completed!\n";
    return 0;
}
