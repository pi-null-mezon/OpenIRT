FROM ubuntu:18.04

ARG CORES=2

RUN apt-get update && apt-get install -y locales && rm -rf /var/lib/apt/lists/* \
    && localedef -i en_US -c -f UTF-8 -A /usr/share/locale/locale.alias en_US.UTF-8

# Enable unicode support
ENV LANG en_US.UTF-8

# Copy repository directories into container
COPY /Sources /Programming/OpenIRT/Sources 
COPY /Shared /Programming/OpenIRT/Shared 
COPY /Apps/Face /Programming/OpenIRT/Apps/Face
COPY /Apps/oirtcli /Programming/OpenIRT/Apps/oirtcli
COPY /Apps/Shared /Programming/OpenIRT/Apps/Shared

# Update OS and install build tools
RUN apt-get update && \
    apt-get upgrade -y && \
    apt-get install -y build-essential cmake git pkg-config wget 

# Build opencv
RUN apt-get install -y libtbb2 libtbb-dev libjpeg-dev libpng-dev libopenblas-dev && \
    git clone https://github.com/opencv/opencv.git opencv && cd opencv && mkdir build && cd build && \
    cmake -D CMAKE_BUILD_TYPE=RELEASE \
          -D CMAKE_INSTALL_PREFIX=/usr/local \
          -D INSTALL_C_EXAMPLES=OFF \
          -D INSTALL_PYTHON_EXAMPLES=OFF \
          -D BUILD_EXAMPLES=OFF \
          -D BUILD_TESTS=OFF \
          -D BUILD_PERF_TESTS=OFF \
          -D BUILD_opencv_apps=OFF \
          -D BUILD_opencv_photo=OFF \
          -D BUILD_opencv_video=OFF \
          -D BUILD_opencv_videoio=OFF \
          -D BUILD_opencv_objdetect=OFF \
          -D BUILD_opencv_flann=OFF \
          -D BUILD_opencv_highgui=OFF \
          -D BUILD_opencv_dnn=ON \
          -D BUILD_opencv_python3=OFF \
          -D BUILD_opencv_ml=OFF \
          -D BUILD_opencv_gapi=OFF \
          -D BUILD_opencv_features2d=OFF \
          -D BUILD_opencv_ts=OFF \
          -D BUILD_opencv_java_bindings_generator=OFF \
          -D BUILD_opencv_python_bindings_generator=OFF \
          .. && \
    make -j${CORES} && \
    make install && \
    ldconfig && \
    cd ../../ && rm -rf opencv

# Build Dlib
RUN git clone https://github.com/davisking/dlib.git dlib && \
    cd dlib && mkdir build && cd build && \
    cmake -D DLIB_NO_GUI_SUPPORT=ON \
    .. && \
    cmake --build . --config Release && \
    make install && \
    ldconfig && \
    cd ../../ && rm -rf dlib

# Install Qt5
RUN apt-get install -y qt5-default

# Install Python3 with webserver packages
RUN apt-get install -y python3 python3-pip && \
    pip3 install --no-cache-dir Flask waitress requests	
	
# Build oirtsrv 
RUN cd Programming && git clone https://github.com/pi-null-mezon/OpenFRT.git && \
    cd OpenIRT/Apps/Face/oirtsrv && \
    rm -rf build && mkdir build && cd build && \
    qmake ../oirtsrv.pro && \
    make -j${CORES} && \
    make install && \
    cp ../../httpsrv/httpsrv.py /usr/local/bin && \
    cd / && rm -rf Programming && mkdir -p /var/iface
	
# Download resources 
RUN wget https://github.com/davisking/dlib-models/raw/master/dlib_face_recognition_resnet_model_v1.dat.bz2 && \
    bzip2 -d dlib_face_recognition_resnet_model_v1.dat.bz2 && \
    wget https://github.com/davisking/dlib-models/raw/master/shape_predictor_5_face_landmarks.dat.bz2 && \   
    bzip2 -d shape_predictor_5_face_landmarks.dat.bz2 && \
    wget https://github.com/pi-null-mezon/FaceAntiSpoofing/raw/master/AnyAttacks/Models/nets_v0.tar.gz && \
    tar -xzvf nets_v0.tar.gz && rm nets_v0.tar.gz && \
    mv *.dat /usr/local/bin && \
    wget https://raw.githubusercontent.com/opencv/opencv_3rdparty/dnn_samples_face_detector_20180205_fp16/res10_300x300_ssd_iter_140000_fp16.caffemodel && \
    mv res10_300x300_ssd_iter_140000_fp16.caffemodel /usr/local/bin
	
# Prepare web server
RUN echo "python3 /usr/local/bin/httpsrv.py & oirtsrv -l/var/iface/iface_biometric_templates.yml" > serve && \
	chmod +x serve

# This port is listening by oirtweb server by default
EXPOSE 5000

CMD ./serve   
