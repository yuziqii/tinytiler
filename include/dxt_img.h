#ifndef DXT_IMG_H
#define DXT_IMG_H
#include <vector>
#include <osg/Image>

void fill_4BitImage(std::vector<unsigned char>& jpeg_buf, osg::Image* img, int& width, int& height);

#endif