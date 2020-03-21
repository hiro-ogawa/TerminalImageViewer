#include "libtiv.hpp"

const unsigned int BITMAPS[] = {
    0x00000000, 0x00a0,

    // Block graphics
    // 0xffff0000, 0x2580,  // upper 1/2; redundant with inverse lower 1/2

    0x0000000f, 0x2581, // lower 1/8
    0x000000ff, 0x2582, // lower 1/4
    0x00000fff, 0x2583,
    0x0000ffff, 0x2584, // lower 1/2
    0x000fffff, 0x2585,
    0x00ffffff, 0x2586, // lower 3/4
    0x0fffffff, 0x2587,
    //0xffffffff, 0x2588,  // full; redundant with inverse space

    0xeeeeeeee, 0x258a, // left 3/4
    0xcccccccc, 0x258c, // left 1/2
    0x88888888, 0x258e, // left 1/4

    0x0000cccc, 0x2596, // quadrant lower left
    0x00003333, 0x2597, // quadrant lower right
    0xcccc0000, 0x2598, // quadrant upper left
                        //0xccccffff, 0x2599,  // 3/4 redundant with inverse 1/4
    0xcccc3333, 0x259a, // diagonal 1/2
                        //0xffffcccc, 0x259b,  // 3/4 redundant
                        //0xffff3333, 0x259c,  // 3/4 redundant
    0x33330000, 0x259d, // quadrant upper right
                        //0x3333cccc, 0x259e,  // 3/4 redundant
                        //0x3333ffff, 0x259f,  // 3/4 redundant

    // Line drawing subset: no double lines, no complex light lines

    0x000ff000, 0x2501, // Heavy horizontal
    0x66666666, 0x2503, // Heavy vertical

    0x00077666, 0x250f, // Heavy down and right
    0x000ee666, 0x2513, // Heavy down and left
    0x66677000, 0x2517, // Heavy up and right
    0x666ee000, 0x251b, // Heavy up and left

    0x66677666, 0x2523, // Heavy vertical and right
    0x666ee666, 0x252b, // Heavy vertical and left
    0x000ff666, 0x2533, // Heavy down and horizontal
    0x666ff000, 0x253b, // Heavy up and horizontal
    0x666ff666, 0x254b, // Heavy cross

    0x000cc000, 0x2578, // Bold horizontal left
    0x00066000, 0x2579, // Bold horizontal up
    0x00033000, 0x257a, // Bold horizontal right
    0x00066000, 0x257b, // Bold horizontal down

    0x06600660, 0x254f, // Heavy double dash vertical

    0x000f0000, 0x2500, // Light horizontal
    0x0000f000, 0x2500, //
    0x44444444, 0x2502, // Light vertical
    0x22222222, 0x2502,

    0x000e0000, 0x2574, // light left
    0x0000e000, 0x2574, // light left
    0x44440000, 0x2575, // light up
    0x22220000, 0x2575, // light up
    0x00030000, 0x2576, // light right
    0x00003000, 0x2576, // light right
    0x00004444, 0x2577, // light down
    0x00002222, 0x2577, // light down

    // Misc technical

    0x44444444, 0x23a2, // [ extension
    0x22222222, 0x23a5, // ] extension

    0x0f000000, 0x23ba, // Horizontal scanline 1
    0x00f00000, 0x23bb, // Horizontal scanline 3
    0x00000f00, 0x23bc, // Horizontal scanline 7
    0x000000f0, 0x23bd, // Horizontal scanline 9

    // Geometrical shapes. Tricky because some of them are too wide.

    //0x00ffff00, 0x25fe,  // Black medium small square
    0x00066000, 0x25aa, // Black small square

    //0x11224488, 0x2571,  // diagonals
    //0x88442211, 0x2572,
    //0x99666699, 0x2573,
    //0x000137f0, 0x25e2,  // Triangles
    //0x0008cef0, 0x25e3,
    //0x000fec80, 0x25e4,
    //0x000f7310, 0x25e5,

    0, 0 // End marker
};

struct CharData
{
  std::array<int, 3> fgColor = {0};
  std::array<int, 3> bgColor = {0};
  int codePoint;
};

// Return a CharData struct with the given code point and corresponding averag fg and bg colors.
CharData getCharData(const cv::Mat &image, int x0, int y0, int codepoint, int pattern)
{
  CharData result;
  result.codePoint = codepoint;
  int fg_count = 0;
  int bg_count = 0;
  unsigned int mask = 0x80000000;

  for (int y = 0; y < 8; y++)
  {
    for (int x = 0; x < 4; x++)
    {
      int *avg;
      if (pattern & mask)
      {
        avg = result.fgColor.data();
        fg_count++;
      }
      else
      {
        avg = result.bgColor.data();
        bg_count++;
      }
      for (int i = 0; i < 3; i++)
      {
        // avg[i] += image(x0 + x, y0 + y, 0, i);
        avg[i] += image.at<cv::Vec3b>(y0 + y, x0 + x)[i];
      }
      mask = mask >> 1;
    }
  }

  // Calculate the average color value for each bucket
  for (int i = 0; i < 3; i++)
  {
    if (bg_count != 0)
    {
      result.bgColor[i] /= bg_count;
    }
    if (fg_count != 0)
    {
      result.fgColor[i] /= fg_count;
    }
  }
  return result;
}

// Find the best character and colors for a 4x8 part of the image at the given position
CharData getCharData(const cv::Mat &image, int x0, int y0)
{
  int min[3] = {255, 255, 255};
  int max[3] = {0};
  std::map<long, int> count_per_color;

  // Determine the minimum and maximum value for each color channel
  for (int y = 0; y < 8; y++)
  {
    for (int x = 0; x < 4; x++)
    {
      long color = 0;
      for (int i = 0; i < 3; i++)
      {
        // int d = image(x0 + x, y0 + y, 0, i);
        int d = image.at<cv::Vec3b>(y0 + y, x0 + x)[i];
        min[i] = std::min(min[i], d);
        max[i] = std::max(max[i], d);
        color = (color << 8) | d;
      }
      count_per_color[color]++;
    }
  }

  std::multimap<int, long> color_per_count;
  for (auto i = count_per_color.begin(); i != count_per_color.end(); ++i)
  {
    color_per_count.insert(std::pair<int, long>(i->second, i->first));
  }

  auto iter = color_per_count.rbegin();
  int count2 = iter->first;
  long max_count_color_1 = iter->second;
  long max_count_color_2 = max_count_color_1;
  if ((++iter) != color_per_count.rend())
  {
    count2 += iter->first;
    max_count_color_2 = iter->second;
  }

  unsigned int bits = 0;
  bool direct = count2 > (8 * 4) / 2;

  if (direct)
  {
    for (int y = 0; y < 8; y++)
    {
      for (int x = 0; x < 4; x++)
      {
        bits = bits << 1;
        int d1 = 0;
        int d2 = 0;
        for (int i = 0; i < 3; i++)
        {
          int shift = 16 - 8 * i;
          int c1 = (max_count_color_1 >> shift) & 255;
          int c2 = (max_count_color_2 >> shift) & 255;
          // int c = image(x0 + x, y0 + y, 0, i);
          int c = image.at<cv::Vec3b>(y0 + y, x0 + x)[i];
          d1 += (c1 - c) * (c1 - c);
          d2 += (c2 - c) * (c2 - c);
        }
        if (d1 > d2)
        {
          bits |= 1;
        }
      }
    }
  }
  else
  {
    // Determine the color channel with the greatest range.
    int splitIndex = 0;
    int bestSplit = 0;
    for (int i = 0; i < 3; i++)
    {
      if (max[i] - min[i] > bestSplit)
      {
        bestSplit = max[i] - min[i];
        splitIndex = i;
      }
    }

    // We just split at the middle of the interval instead of computing the median.
    int splitValue = min[splitIndex] + bestSplit / 2;

    // Compute a bitmap using the given split and sum the color values for both buckets.
    for (int y = 0; y < 8; y++)
    {
      for (int x = 0; x < 4; x++)
      {
        bits = bits << 1;
        // if (image(x0 + x, y0 + y, 0, splitIndex) > splitValue)
        if (image.at<cv::Vec3b>(y0 + y, x0 + x)[splitIndex] > splitValue)
        {
          bits |= 1;
        }
      }
    }
  }

  // Find the best bitmap match by counting the bits that don't match,
  // including the inverted bitmaps.
  int best_diff = 8;
  unsigned int best_pattern = 0x0000ffff;
  int codepoint = 0x2584;
  bool inverted = false;
  for (int i = 0; BITMAPS[i + 1] != 0; i += 2)
  {
    unsigned int pattern = BITMAPS[i];
    for (int j = 0; j < 2; j++)
    {
      int diff = (std::bitset<32>(pattern ^ bits)).count();
      if (diff < best_diff)
      {
        best_pattern = BITMAPS[i]; // pattern might be inverted.
        codepoint = BITMAPS[i + 1];
        best_diff = diff;
        inverted = best_pattern != pattern;
      }
      pattern = ~pattern;
    }
  }

  if (direct)
  {
    CharData result;
    if (inverted)
    {
      long tmp = max_count_color_1;
      max_count_color_1 = max_count_color_2;
      max_count_color_2 = tmp;
    }
    for (int i = 0; i < 3; i++)
    {
      int shift = 16 - 8 * i;
      result.fgColor[i] = (max_count_color_2 >> shift) & 255;
      result.bgColor[i] = (max_count_color_1 >> shift) & 255;
      result.codePoint = codepoint;
    }
    return result;
  }
  return getCharData(image, x0, y0, codepoint, best_pattern);
}

int clamp_byte(int value)
{
  return value < 0 ? 0 : (value > 255 ? 255 : value);
}

std::string emit_color(bool bg, int r, int g, int b)
{
  std::stringstream ss;
  r = clamp_byte(r);
  g = clamp_byte(g);
  b = clamp_byte(b);

  ss << (bg ? "\x1b[48;2;" : "\x1b[38;2;") << r << ';' << g << ';' << b << 'm';
  return ss.str();
}

std::string emitCodepoint(int codepoint)
{
  std::stringstream ss;
  if (codepoint < 128)
  {
    ss << (char)codepoint;
  }
  else if (codepoint < 0x7ff)
  {
    ss << (char)(0xc0 | (codepoint >> 6));
    ss << (char)(0x80 | (codepoint & 0x3f));
  }
  else if (codepoint < 0xffff)
  {
    ss << (char)(0xe0 | (codepoint >> 12));
    ss << (char)(0x80 | ((codepoint >> 6) & 0x3f));
    ss << (char)(0x80 | (codepoint & 0x3f));
  }
  else if (codepoint < 0x10ffff)
  {
    ss << (char)(0xf0 | (codepoint >> 18));
    ss << (char)(0x80 | ((codepoint >> 12) & 0x3f));
    ss << (char)(0x80 | ((codepoint >> 6) & 0x3f));
    ss << (char)(0x80 | (codepoint & 0x3f));
  }
  else
  {
    std::cerr << "ERROR";
  }
  return ss.str();
}

std::vector<std::string> emit_image(const cv::Mat &image)
{
  std::vector<std::string> image_string;
  CharData lastCharData;
  for (int y = 0; y <= image.size().height - 8; y += 8)
  {
    std::stringstream ss;
    for (int x = 0; x <= image.size().width - 4; x += 4)
    {
      CharData charData = getCharData(image, x, y);
      if (x == 0 || charData.bgColor != lastCharData.bgColor)
        ss << emit_color(true, charData.bgColor[0], charData.bgColor[1], charData.bgColor[2]);
      if (x == 0 || charData.fgColor != lastCharData.fgColor)
        ss << emit_color(false, charData.fgColor[0], charData.fgColor[1], charData.fgColor[2]);
      ss << emitCodepoint(charData.codePoint);
      lastCharData = charData;
    }
    ss << "\x1b[0m";
    image_string.push_back(ss.str());
  }
  return image_string;
}
