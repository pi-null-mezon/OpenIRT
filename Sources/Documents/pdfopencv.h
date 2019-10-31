#ifndef PDFOPENCV_H
#define PDFOPENCV_H

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

#include <poppler/cpp/poppler-document.h>
#include <poppler/cpp/poppler-page.h>
#include <poppler/cpp/poppler-page-renderer.h>
#include <poppler/cpp/poppler-image.h>

namespace cv {

std::vector<cv::Mat> pdfread(const std::string &_filename, int _dpi=100)
{
    std::vector<cv::Mat> _vmats;
    poppler::document* pdfdoc = poppler::document::load_from_file(_filename);
    if(pdfdoc != NULL) {
        _vmats.resize(pdfdoc->pages());
        for(size_t i = 0; i < _vmats.size(); ++i) {
            poppler::page* page = pdfdoc->create_page(i);
            poppler::page_renderer renderer;
            renderer.set_render_hint(poppler::page_renderer::text_antialiasing);
            poppler::image image = renderer.render_page(page,_dpi,_dpi);
            switch(image.format()) {
                case poppler::image::format_bgr24:
                    cv::Mat(image.height(),image.width(),CV_8UC3,image.data()).copyTo(_vmats[i]);
                    cv::cvtColor(_vmats[i],_vmats[i],cv::COLOR_BGR2RGB);
                    break;
                case poppler::image::format_rgb24:
                    cv::Mat(image.height(),image.width(),CV_8UC3,image.data()).copyTo(_vmats[i]);
                    break;
                case poppler::image::format_argb32:
                    cv::Mat(image.height(),image.width(),CV_8UC4,image.data()).copyTo(_vmats[i]);
                    break;
                case poppler::image::format_gray8:
                    cv::Mat(image.height(),image.width(),CV_8UC1,image.data()).copyTo(_vmats[i]);
                    break;
                default:
                    std::cout << "pdfread error - unsupported image format" << std::endl;
                    break;
            }
            delete page;
        }
    }
    delete pdfdoc;
    return _vmats;
}

std::vector<cv::Mat> pdfread(const char *file_data, int file_data_length, int _dpi=100)
{
    std::vector<cv::Mat> _vmats;
    poppler::document* pdfdoc = poppler::document::load_from_raw_data(file_data,file_data_length);
    if(pdfdoc != NULL) {
        _vmats.resize(pdfdoc->pages());
        for(size_t i = 0; i < _vmats.size(); ++i) {
            poppler::page* page = pdfdoc->create_page(i);
            poppler::page_renderer renderer;
            renderer.set_render_hint(poppler::page_renderer::text_antialiasing);
            poppler::image image = renderer.render_page(page,_dpi,_dpi);
            switch(image.format()) {
                case poppler::image::format_bgr24:
                    cv::Mat(image.height(),image.width(),CV_8UC3,image.data()).copyTo(_vmats[i]);
                    cv::cvtColor(_vmats[i],_vmats[i],cv::COLOR_BGR2RGB);
                    break;
                case poppler::image::format_rgb24:
                    cv::Mat(image.height(),image.width(),CV_8UC3,image.data()).copyTo(_vmats[i]);
                    break;
                case poppler::image::format_argb32:
                    cv::Mat(image.height(),image.width(),CV_8UC4,image.data()).copyTo(_vmats[i]);
                    break;
                case poppler::image::format_gray8:
                    cv::Mat(image.height(),image.width(),CV_8UC1,image.data()).copyTo(_vmats[i]);
                    break;
                default:
                    std::cout << "pdfread error - unsupported image format" << std::endl;
                    break;
            }
            delete page;
        }
    }
    delete pdfdoc;
    return _vmats;
}
}
#endif // PDFOPENCV_H
