#include "googlenetrecognizer.h"

#include <fstream>

using namespace std;
using namespace cv;
using namespace imgrec;

#define RES_PATH "C:/Programming/3rdParties/Caffe/models/bvlc_googlenet"

#define PICTURE_FILE "C:/Programming/3rdParties/opencv321/opencv_contrib/modules/dnn/samples/space_shuttle.jpg"
#define LABELINFO_FILE "C:/Programming/3rdParties/opencv321/opencv_contrib/modules/dnn/samples/synset_words.txt"

std::vector<String> readClassNames(const char *filename = LABELINFO_FILE)
{
    std::vector<String> classNames;

    std::ifstream fp(filename);
    if (!fp.is_open())
    {
        std::cerr << "File with classes labels not found: " << filename << std::endl;
        exit(-1);
    }

    std::string name;
    while (!fp.eof())
    {
        std::getline(fp, name);
        if (name.length())
            classNames.push_back( name.substr(name.find(' ')+1) );
    }

    fp.close();
    return classNames;
}

int main(int argc, char *argv[])
{

    Ptr<ImageRecognizer> _ptr = createGoogleNetRecognizer( String(RES_PATH)+"/bvlc_googlenet.prototxt",
                                                           String(RES_PATH)+"/bvlc_googlenet.caffemodel",
                                                           DistanceType::Euclidean,
                                                           DBL_MAX );

    Mat _img = imread(PICTURE_FILE, cv::IMREAD_UNCHANGED);
    Mat _dscr = _ptr->getImageDescription(_img);

    float *_data = _dscr.ptr<float>(0);
    /*for(int i = 0; i < _dscr.total(); ++i) {
        std::cout << i << ") " << _data[i] << std::endl;
    }*/
    int _label = (std::max_element(_data, _data+_dscr.total()) - _data);
    std::string _labelinfo = readClassNames()[_label];
    std::cout << "Predicted label: " << _labelinfo << " (prob: " << _data[_label]*100.0 << " %)"  << std::endl;

    return 0;
}
