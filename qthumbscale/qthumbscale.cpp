#include "qthumbscale.h"

#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <vector>
#include <iostream>

#include <cassert>
#include <glog/logging.h>

#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

namespace qcontent
{

static std::string base64_encode(unsigned char const* , unsigned int len);
//static std::string base64_decode(std::string const& s);


QThumbScale::QThumbScale()
{
    cascade = new cv::CascadeClassifier();
    debug = false;
}

QThumbScale::~QThumbScale()
{
    delete cascade;
}

bool QThumbScale::init(const std::string &cascade_cfg, double scale)
{
    scaled_w = 120;
    scaled_h = 80;

    cascade_config = cascade_cfg;
    cascade_scale = scale;

    if(!cascade_config.empty()
            && !cascade->load(cascade_config)) {
        return false;
    }

    return true;
}

bool QThumbScale::scale_to_thumb(const unsigned char *data, size_t size, std::string &thumb_base64)
{
    std::vector<unsigned char> buf(data, data + size);

    try {
        cv::Mat mat_img = cv::imdecode(cv::Mat(buf), 1);

        return scale_to_thumb(mat_img, thumb_base64);
    } catch (cv::Exception &e) {
        LOG(ERROR) << "scale _thumb excepition " << e.what();
        return false;
    }
}

bool QThumbScale::scale_to_thumb(cv::Mat &mat_img, std::string &thumb_base64)
{
    int h = mat_img.rows;
    int w = mat_img.cols;

    if (mat_img.empty() || h <= 0 || w <= 0) {
        LOG(INFO) << "img is empty";
        return false;
    }

    if (!debug) {
        if (h * w < 90000 || w  > 3 * h || 2 * w < h) {
            LOG(INFO) << "img is not good";
            return false;
        }
    }

    bool has_face = false;
    int face_center_x = 0;
    int face_center_y = 0;
    double face_radius = 0.0;

    DLOG(INFO) << "face detect";
    has_face = face_detect(mat_img, face_center_x, face_center_y, face_radius);
    DLOG(INFO) << "after face detect";

    cv::Mat resize_img;
    //cv::Size(scaled_w, scaled_h);
    double w_ratio =  static_cast<double>(scaled_w) / w;
    double h_ratio =  static_cast<double>(scaled_h) / h;

    int resize_face_radius = 0;
    //int resize_face_area_ratio = 0;

    int resize_w = 0, resize_h = 0;
    int thumb_x0 = 0, thumb_y0 = 0;

    if (w_ratio < h_ratio) {
        resize_h = scaled_h;
        resize_w = w * h_ratio;
        if (resize_w < scaled_w) {
            resize_w = scaled_w;
        }

        if (has_face) {
            int resize_fx = face_center_x * h_ratio;
            int half_scaled_w = scaled_w / 2;
            if (resize_fx >= resize_w - half_scaled_w) {
                thumb_x0 = resize_w - scaled_w;
            } else if (resize_fx <= half_scaled_w) {
                thumb_x0 = 0;
            } else {
                thumb_x0 = resize_fx - half_scaled_w;
            }
            resize_face_radius = face_radius * h_ratio;
        } else {
            thumb_x0 = (resize_w - scaled_w)/2;
        }

    } else {
        resize_w = scaled_w;
        resize_h = h * w_ratio;
        if (resize_h < scaled_h) {
            resize_h = scaled_h;
        }

        if (has_face) {
            int resize_fy = face_center_y * w_ratio;
            int half_scaled_h = scaled_h / 2;
            if (resize_fy >= resize_h - half_scaled_h) {
                thumb_y0 = resize_h - scaled_h;
            } else if (resize_fy <= half_scaled_h) {
                thumb_y0 = 0;
            } else {
                thumb_y0 = resize_fy - half_scaled_h;
            }
            resize_face_radius = face_radius * w_ratio;
        } else {
            thumb_y0 = (resize_h - scaled_h)/2;
        }
    }

    if (!debug && has_face
            && (double)resize_face_radius * 2 /  scaled_h  > 0.7
            && 3.1415926 * (double)resize_face_radius * (double)resize_face_radius / (scaled_w * scaled_h) > 0.33) {
        LOG(INFO) << "ignore big face img";
        return false;
    }

    assert(resize_w >= scaled_w && resize_h >= scaled_h);
    assert(thumb_x0 + scaled_w <= resize_w);
    assert(thumb_y0 + scaled_h <= resize_h);

    DLOG(INFO) << "resize_w: " << resize_w;
    DLOG(INFO) << "resize_h: " << resize_h;
    DLOG(INFO) << "thumb_x0: " << thumb_x0;
    DLOG(INFO) << "thumb_y0: " << thumb_y0;
    DLOG(INFO) << "scaled_w: " << scaled_w;
    DLOG(INFO) << "scaled_h: " << scaled_h;

    cv::resize(mat_img, resize_img, cv::Size(resize_w, resize_h), 0, 0);
    DLOG(INFO) << "after resize";
    cv::Mat thumb_img(resize_img, cv::Rect(thumb_x0, thumb_y0, scaled_w, scaled_h));
    DLOG(INFO) << "after thumb";

    if (debug) {
        std::cout << "w " << w << std::endl;
        std::cout << "h " << h << std::endl;
        std::cout << "has_face " << has_face << std::endl;
        std::cout << "fx " << face_center_x << std::endl;
        std::cout << "fy " << face_center_y << std::endl;
        std::cout << "fr " << face_radius << std::endl;
        std::cout << "resize_fr " << resize_face_radius << std::endl;
        std::cout << "resize_fr_ratio " << 2 * resize_face_radius * 100 / 80 << std::endl;
        std::cout << "resize_fr_area_ratio " << 3.1415926 * resize_face_radius * resize_face_radius * 100 / (120 * 80) << std::endl;
        cvNamedWindow( "result", 1 );
        cv::Scalar color = CV_RGB(255,0,255);
        cv::rectangle(resize_img, cv::Point(thumb_x0, thumb_y0), cv::Point(thumb_x0 + scaled_w, thumb_y0 + scaled_h), color);
        cv::imshow("result", resize_img);
        cv::waitKey(0);
        cvDestroyWindow("result");
    }

    std::vector<unsigned char> write_buf;
    std::vector<int> params;
    //params.push_back(100);
    params.push_back(CV_IMWRITE_JPEG_QUALITY);
    params.push_back(85);
    if (!imencode(".jpg", thumb_img, write_buf, params)) {
        LOG(ERROR) << "encode jpg error";
        return false;
    }
    int write_buf_size = write_buf.size();

    std::string data;
    data.reserve(write_buf_size);
    for (int i = 0; i < write_buf_size; ++i) {
        data.append(1, write_buf[i]);
    }
    // add raw width and raw height;
    char header_buf[128];
    sprintf(header_buf, "1 %d %d ", w, h);
    thumb_base64.append(header_buf);

    //thumb_base64.append("data:image/jpeg;base64,");
    thumb_base64.append(base64_encode((const unsigned char *)data.c_str(), data.size()));

    return true;
}

bool QThumbScale::face_detect(cv::Mat &mat_img, int &face_center_x, int &face_center_y, double &face_radius)
{
    std::vector<cv::Rect> faces;

    cv::Mat gray, smallImg(cvRound (mat_img.rows/cascade_scale), cvRound(mat_img.cols/cascade_scale), CV_8UC1 );
    cv::cvtColor(mat_img, gray, CV_BGR2GRAY );
    cv::resize( gray, smallImg, smallImg.size(), 0, 0, cv::INTER_LINEAR );
    cv::equalizeHist( smallImg, smallImg );

    cascade->detectMultiScale(smallImg, faces,
        1.1, 2, 0
        //|CV_HAAR_FIND_BIGGEST_OBJECT
        //|CV_HAAR_DO_ROUGH_SEARCH
        |CV_HAAR_SCALE_IMAGE
        ,
        cv::Size(30, 30) );

    double max_radius = 0.0;

    for (std::vector<cv::Rect>::iterator r = faces.begin(); r != faces.end(); ++r) {
        int cx = cvRound((r->x + r->width * 0.5) * cascade_scale);
        int cy = cvRound((r->y + r->height * 0.5) * cascade_scale);
        int radius = cvRound((r->width + r->height) * 0.25 * cascade_scale);

        if (radius > max_radius) {
            max_radius = radius;
            face_radius = radius;
            face_center_x = cx;
            face_center_y = cy;
        }
    }

    if (max_radius > 1) {
        return true;
    }

    return false;
}

/*
   base64.cpp and base64.h

   Copyright (C) 2004-2008 René Nyffenegger

   This source code is provided 'as-is', without any express or implied
   warranty. In no event will the author be held liable for any damages
   arising from the use of this software.

   Permission is granted to anyone to use this software for any purpose,
   including commercial applications, and to alter it and redistribute it
   freely, subject to the following restrictions:

   1. The origin of this source code must not be misrepresented; you must not
      claim that you wrote the original source code. If you use this source code
      in a product, an acknowledgment in the product documentation would be
      appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
      misrepresented as being the original source code.

   3. This notice may not be removed or altered from any source distribution.

   René Nyffenegger rene.nyffenegger@adp-gmbh.ch

*/


static const std::string base64_chars = 
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";


static inline bool is_base64(unsigned char c) {
  return (isalnum(c) || (c == '+') || (c == '/'));
}

static std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len) {
  std::string ret;
  int i = 0;
  int j = 0;
  unsigned char char_array_3[3];
  unsigned char char_array_4[4];

  while (in_len--) {
    char_array_3[i++] = *(bytes_to_encode++);
    if (i == 3) {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for(i = 0; (i <4) ; i++)
        ret += base64_chars[char_array_4[i]];
      i = 0;
    }
  }

  if (i)
  {
    for(j = i; j < 3; j++)
      char_array_3[j] = '\0';

    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
    char_array_4[3] = char_array_3[2] & 0x3f;

    for (j = 0; (j < i + 1); j++)
      ret += base64_chars[char_array_4[j]];

    while((i++ < 3))
      ret += '=';

  }

  return ret;

}

/*
static std::string base64_decode(std::string const& encoded_string) {
  int in_len = encoded_string.size();
  int i = 0;
  int j = 0;
  int in_ = 0;
  unsigned char char_array_4[4], char_array_3[3];
  std::string ret;

  while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
    char_array_4[i++] = encoded_string[in_]; in_++;
    if (i ==4) {
      for (i = 0; i <4; i++)
        char_array_4[i] = base64_chars.find(char_array_4[i]);

      char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

      for (i = 0; (i < 3); i++)
        ret += char_array_3[i];
      i = 0;
    }
  }

  if (i) {
    for (j = i; j <4; j++)
      char_array_4[j] = 0;

    for (j = 0; j <4; j++)
      char_array_4[j] = base64_chars.find(char_array_4[j]);

    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

    for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
  }

  return ret;
}

*/

} // end namespace qcontent
