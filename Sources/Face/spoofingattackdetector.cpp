#include "spoofingattackdetector.h"

namespace dlib {

template<typename image_type>
rectangle make_random_cropping_rect(const matrix<image_type> &img, dlib::rand &rnd, float _mins, float _maxs)
{
    auto _scale = _mins + rnd.get_random_double()*(_maxs-_mins);
    rectangle rect(_scale*img.nc(),_scale*img.nr());
    // randomly shift the box around
    point offset(rnd.get_random_32bit_number()%(img.nc()-rect.width()),
                 rnd.get_random_32bit_number()%(img.nr()-rect.height()));
    return move_rect(rect, offset);
}

template<typename image_type>
void randomly_crop_image(const matrix<image_type>& img,
                         dlib::array<matrix<image_type>>& crops,
                         dlib::rand& rnd, long num_crops,
                         float _mins=0.9f, float _maxs=1.0f,
                         unsigned long _tcols=0, unsigned long _trows=0, bool _rndfliplr=true)
{
    if(_tcols == 0)
        _tcols = num_columns(img);
    if(_trows == 0)
        _trows = num_rows(img);

    std::vector<chip_details> dets;
    for (long i = 0; i < num_crops; ++i) {
        auto rect = make_random_cropping_rect(img, rnd, _mins, _maxs);
        dets.push_back(chip_details(rect, chip_dims(_trows,_tcols)));
    }
    extract_image_chips(img, dets, crops);

    if(_rndfliplr)
        for(auto&& _tmpimg : crops)
            if (rnd.get_random_double() > 0.5)
                _tmpimg = fliplr(_tmpimg);

}
}

namespace cv { namespace oirt {

SpoofingAttackDetector::SpoofingAttackDetector(const cv::String &_attack_modelname, const cv::String &_dlibshapepredictor) :
    CNNImageClassifier(Size(0,0),ColorOrder::RGB,CropMethod::NoCrop)
{
    nets = std::vector<dlib::softmax<dlib::densenet::subnet_type>>(5);
    for(size_t i = 0; i < nets.size(); ++i) {
        try {
            dlib::densenet _tmpnet;
            dlib::deserialize(_attack_modelname + "net_0_split_" + std::to_string(i) + ".dat") >> _tmpnet;
            nets[i].subnet() = _tmpnet.subnet();
        } catch(const std::exception& e) {
            std::cout << e.what() << std::endl;
        }
    }
    try {
        dlibfacedet = dlib::get_frontal_face_detector();
    } catch(const std::exception& e) {
        std::cout << e.what() << std::endl;
    }
    try {
        dlib::deserialize(_dlibshapepredictor.c_str()) >> dlibshapepredictor;
    } catch(const std::exception& e) {
        std::cout << e.what() << std::endl;
    }
    // Labels known to networks
    setLabelInfo(0,"Live");
    setLabelInfo(1,"Mask");
    setLabelInfo(2,"Print");
    setLabelInfo(3,"Replay");
    // Possible errors (0 - no error)
    errorsInfo[1] = "Can not find face!";
}



void SpoofingAttackDetector::predict(InputArray src, int &label, float &conf, int *_error) const
{
    cv::Mat _preprocessedmat = preprocessImageForCNN(src.getMat(),getInputSize(),getColorOrder(),getCropInput());
    auto _facerect = __detectbiggestface(_preprocessedmat);
    if(_facerect.area() != 0) {
        auto _facechip = __extractface(_preprocessedmat,_facerect,150,0.25);
        double _tm1 = cv::getTickCount();
        dlib::rand rnd(7);
        const long cropsnum = 16;
        dlib::array<dlib::matrix<dlib::rgb_pixel>> crops;
        dlib::matrix<float,1,4> vc;
        for(long k = 0; k < dlib::num_columns(vc); ++k)
            vc(k) = 0;
        for(size_t i = 0; i < nets.size(); ++i) {
            /*dlib::randomly_crop_image(_facechip,crops,rnd,cropsnum);
            dlib::matrix<float,1,4> p = dlib::sum_rows(dlib::mat(nets[i](crops.begin(),crops.end())))/crops.size();*/
            dlib::matrix<float,1,4> p = dlib::mat(nets[i](_facechip));
            for(long k = 0; k < dlib::num_columns(vc); ++k)
                vc(k) += p(k);
        }
        for(long k = 0; k < dlib::num_columns(vc); ++k)
            vc(k) /= nets.size();

        std::cout << 1000.0 * (cv::getTickCount() - _tm1) / cv::getTickFrequency() << " ms" << std::endl;
        label = dlib::index_of_max(vc);
        conf = vc(label);
        if(_error)
            *_error = 0;
    } else if(_error) {
        *_error = 1;
    }
}

void SpoofingAttackDetector::predict(InputArray src, std::vector<float> &conf, int *_error) const
{
    cv::Mat _preprocessedmat = preprocessImageForCNN(src.getMat(),getInputSize(),getColorOrder(),getCropInput());
    auto _facerect = __detectbiggestface(_preprocessedmat);
    if(_facerect.area() != 0) {
        auto _facechip = __extractface(_preprocessedmat,_facerect,150,0.25);
        double _tm1 = cv::getTickCount();
        conf = std::vector<float>(4,0);
        for(size_t i = 0; i < nets.size(); ++i) {
            dlib::matrix<float,1,4> p = dlib::mat(nets[i](_facechip));
            for(size_t k = 0; k < conf.size(); ++k)
                conf[k] += p(k);
        }
        for(size_t k = 0; k < conf.size(); ++k)
            conf[k] /= nets.size();
        std::cout << 1000.0 * (cv::getTickCount() - _tm1) / cv::getTickFrequency() << " ms" << std::endl;
        if(_error)
            *_error = 0;
    } else if(_error) {
        *_error = 1;
    }
}

Ptr<CNNImageClassifier> SpoofingAttackDetector::createSpoofingAttackDetector(const cv::String &_attack_modelname, const String &_dlibshapepredictor)
{
    return makePtr<SpoofingAttackDetector>(_attack_modelname,_dlibshapepredictor);
}

dlib::matrix<dlib::rgb_pixel> SpoofingAttackDetector::__extractface(const Mat &_inmat, const dlib::rectangle &_facerect,  unsigned long _targetsize, double _padding) const
{
    dlib::cv_image<dlib::rgb_pixel> _rgbcv_image(_inmat);
    auto _shape = dlibshapepredictor(_rgbcv_image, _facerect);
    dlib::matrix<dlib::rgb_pixel> _facechip;
    dlib::extract_image_chip(_rgbcv_image, dlib::get_face_chip_details(_shape,_targetsize,_padding), _facechip);
    return _facechip;
}

dlib::rectangle SpoofingAttackDetector::__detectbiggestface(const Mat &_inmat) const
{
    dlib::rectangle _facerect;
    cv::Mat _graymat;
    cv::cvtColor(_inmat, _graymat, cv::COLOR_RGB2GRAY);
    std::vector<dlib::rectangle> _facerects = dlibfacedet(dlib::cv_image<unsigned char>(_graymat));
    if(_facerects.size() > 0) {
        if(_facerects.size() > 1) {
            std::sort(_facerects.begin(),_facerects.end(),[](const dlib::rectangle &lhs, const dlib::rectangle &rhs) {
                return lhs.area() > rhs.area();
            });
        }
        _facerect = _facerects[0];
    }
    return _facerect;
}

}}
