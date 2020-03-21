#include "libtiv.hpp"

#include <vector>
#include <experimental/filesystem>

using namespace std;

/* Wrapper around CImg<T>(const char*) to ensure the result has 3 channels as RGB
 */
cv::Mat load_rgb_CImg(const char *const filename)
{
  cv::Mat image = cv::imread(filename, cv::IMREAD_COLOR);
  cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
  return image;
}

int main(int argc, char *argv[])
{
  std::vector<std::string> file_names;
  int error = 0;

  for (int i = 1; i < argc; i++)
  {
    std::string arg(argv[i]);
    file_names.push_back(arg);
  }

  for (unsigned int i = 0; i < file_names.size(); i++)
  {
    try
    {
      cv::Mat image = load_rgb_CImg(file_names[i].c_str());
      cout << image.size() << endl;

      auto image_str = emit_image(image);
      for (auto line : image_str)
        cout << line << endl;
    }
    catch (...)
    {
      error = 1;
      std::cerr << "File format is not recognized for '" << file_names[i] << "'" << std::endl;
    }
  }

  return error;
}
