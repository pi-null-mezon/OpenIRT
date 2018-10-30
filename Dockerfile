FROM alextaranov/ubuntu18

COPY /Sources /Sources 
COPY /Shared /Shared 
COPY /Apps /Apps

# Download resources 
RUN wget https://github.com/davisking/dlib-models/blob/master/dlib_face_recognition_resnet_model_v1.dat.bz2?raw=true && \
    mv dlib_face_recognition_resnet_model_v1.dat.bz2?raw=true dlib_face_recognition_resnet_model_v1.dat.bz2 && \
    bzip2 -d dlib_face_recognition_resnet_model_v1.dat.bz2 && \
    mv dlib_face_recognition_resnet_model_v1.dat /usr/local/bin && \
    wget https://github.com/davisking/dlib-models/blob/master/shape_predictor_5_face_landmarks.dat.bz2?raw=true && \
    mv shape_predictor_5_face_landmarks.dat.bz2?raw=true shape_predictor_5_face_landmarks.dat.bz2 && \
    bzip2 -d shape_predictor_5_face_landmarks.dat.bz2 && \
    mv shape_predictor_5_face_landmarks.dat /usr/local/bin

# Build sources 
RUN cd Apps/oirtcli && \
    mkdir build && cd build && \
    qmake ../oirtcli.pro && \
    make && \
    make install && \
    cd ../ && rm -rf build && \
    oirtcli -h
    
