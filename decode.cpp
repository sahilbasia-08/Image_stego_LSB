// decode.cpp  — extract hidden text from encoded images
// g++ -std=c++17 decode.cpp `pkg-config --cflags --libs opencv4` -o decode

#include <opencv2/opencv.hpp>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
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

std::string extractChunk(const cv::Mat &img)
{
    std::string bits;
    bits.reserve(static_cast<size_t>(img.rows) * img.cols * 3);

    for (int r = 0; r < img.rows; ++r)
        for (int c = 0; c < img.cols; ++c)
        {
            const cv::Vec3b &pix = img.at<cv::Vec3b>(r, c);
            for (int ch = 0; ch < 3; ++ch)
                bits.push_back((pix[ch] & 1) ? '1' : '0');
        }

    std::string out, byte;
    for (size_t i = 0; i + 7 < bits.size(); i += 8)
    {
        byte = bits.substr(i, 8);
        char c = static_cast<char>(std::stoi(byte, nullptr, 2));
        out.push_back(c);
        if (out.size() >= 3 && out.substr(out.size() - 3) == "###")
            break;
    }
    return out;
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        std::cerr << "Usage: ./decode --path <directory_with_encoded_images>\n";
        return 1;
    }
    std::string dir = argv[2];

    std::vector<fs::path> imgs;
    for (auto const &entry : fs::directory_iterator(dir))
        if (entry.is_regular_file())
            imgs.push_back(entry.path());

    if (imgs.empty())
    {
        std::cerr << "Directory has no files.\n";
        return 1;
    }

    std::string pwd = getPassword("Enter password for decryption: ");

    std::vector<std::string> pieces(imgs.size());

    for (const auto &p : imgs)
    {
        cv::Mat img = cv::imread(p.string(), cv::IMREAD_COLOR);
        if (img.empty())
        {
            std::cerr << "Cannot read " << p << '\n';
            return 1;
        }

        std::string chunk = extractChunk(img);
        if (chunk.size() < 7)
            continue;
        char seqChar = chunk[chunk.size() - 7];
        int seq = seqChar - '0';
        if (seq == 0)
        {
            /* verify password */
            std::string withPwd = chunk.substr(0, chunk.size() - 7);
            size_t first = withPwd.find("@%#/");
            size_t second = withPwd.find("@%#/", first + 4);
            if (first == std::string::npos || second == std::string::npos)
            {
                std::cerr << "Malformed header.\n";
                return 1;
            }
            std::string pwdInside = withPwd.substr(first + 4, second - (first + 4));
            if (pwdInside != pwd)
            {
                std::cerr << "Password is not valid.\n";
                return 1;
            }
            std::cout << "Initializing extraction....\n";
            std::string content = withPwd.substr(0, first);
            pieces[seq] = content;
        }
        else
        {
            pieces[seq] = chunk.substr(0, chunk.size() - 7);
        }
    }

    /* concatenate in order */
    std::string result;
    for (const auto &s : pieces)
        result += s;

    std::ofstream fout("Extracted_msg.txt");
    fout << result;
    fout.close();

    std::cout << ".\n.\n.\n>>> Extraction completed.\n";
    return 0;
}
