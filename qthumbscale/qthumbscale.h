#ifndef QTHUMB_SCALE_H
#define QTHUMB_SCALE_H

#include <stdlib.h>
#include <string>

namespace cv {
    class Mat;
    class CascadeClassifier;
}

namespace qcontent
{


class QThumbScale
{
public:
    QThumbScale();
    ~QThumbScale();

    bool init(const std::string &cascade_cfg, double scale);
    void enable_debug() {
        debug = true;
    }

    bool scale_to_thumb(const unsigned char *data, size_t size, std::string &thumb_base64);
    bool scale_to_thumb(cv::Mat &mat_img, std::string &thumb_base64);

    bool face_detect(cv::Mat &img, int &face_center_x, int &face_center_y, double &face_radius);

private:
    cv::CascadeClassifier *cascade;
    std::string cascade_config;
    double cascade_scale;
    int scaled_w;
    int scaled_h;
    bool debug;
};


} // end namespace qcontent

#endif

