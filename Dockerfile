FROM ubuntu:18.04

RUN apt-get update && apt-get install -y locales && rm -rf /var/lib/apt/lists/* \
    && localedef -i en_US -c -f UTF-8 -A /usr/share/locale/locale.alias en_US.UTF-8

# Enable unicode support
ENV LANG en_US.UTF-8

# Copy repository directories into container
COPY /Sources /Sources 
COPY /Shared /Shared 
COPY /Apps /Apps

# Update OS and install build tools
RUN pt-get update && \
    apt-get upgrade -y && \
    apt-get install -y build-essential cmake git pkg-config wget 

# Build opencv
RUN apt-get install -y libavcodec-dev libavformat-dev libswscale-dev libtbb2 libtbb-dev libjpeg-dev libpng-dev libatlas-base-dev && \
    git clone https://github.com/opencv/opencv.git opencv && cd opencv && git checkout 3.4 && mkdir build && cd build && \
    cmake -DCMAKE_BUILD_TYPE=RELEASE \
          -D CMAKE_INSTALL_PREFIX=/usr/local \
          -D INSTALL_C_EXAMPLES=OFF \
          -D INSTALL_PYTHON_EXAMPLES=OFF \
          -D BUILD_EXAMPLES=OFF \
          -D BUILD_TESTS=OFF \
          -D BUILD_PERF_TESTS=OFF \
          -D BUILD_opencv_flann=OFF \
          -D BUILD_opencv_photo=OFF \
          -D BUILD_opencv_video=OFF \
          -D BUILD_opencv_ts=OFF \
          -D BUILD_opencv_java_bindings_generator=OFF \
          -D BUILD_opencv_python_bindings_generator=OFF \
          .. && \
    make -j$(nproc) && \
    make install && \
    ldconfig && \
    cd ../../ && rm -rf opencv

# Build Dlib
RUN apt-get install -y libopenblas-dev && \
    git clone https://github.com/davisking/dlib.git dlib && \
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
    pip3 install -U Flask && \
	pip3 install waitress	
	
# Build oirtcli 
RUN cd Apps/oirtcli && \
    mkdir build && cd build && \
    qmake ../oirtcli.pro && \
    make && \
    make install && \
    cd ../ && rm -rf build && cd ~/..
	
# Build oirtsrv 
RUN cd Apps/Face/oirtsrv && \
    mkdir build && cd build && \
    qmake ../oirtsrv.pro && \
    make -j2 && \
    make install && \
    cd ../ && rm -rf build && cd ~/.. && \
	mkdir -p /var/facerec
	
# Download resources 
RUN wget https://github.com/davisking/dlib-models/blob/master/dlib_face_recognition_resnet_model_v1.dat.bz2?raw=true && \
    mv dlib_face_recognition_resnet_model_v1.dat.bz2?raw=true dlib_face_recognition_resnet_model_v1.dat.bz2 && \
    bzip2 -d dlib_face_recognition_resnet_model_v1.dat.bz2 && \
    mv dlib_face_recognition_resnet_model_v1.dat /usr/local/bin && \
    wget https://github.com/davisking/dlib-models/blob/master/shape_predictor_5_face_landmarks.dat.bz2?raw=true && \
    mv shape_predictor_5_face_landmarks.dat.bz2?raw=true shape_predictor_5_face_landmarks.dat.bz2 && \
    bzip2 -d shape_predictor_5_face_landmarks.dat.bz2 && \
    mv shape_predictor_5_face_landmarks.dat /usr/local/bin
	
# Prepare web server
RUN mkdir -p /home/Testdata && \
	echo "python3 /Apps/Face/oirtweb/oirtwebsrv.py &" > startwebsrv && \
	chmod +x startwebsrv

# This port is listening by oirtweb server by default
EXPOSE 5000

CMD /bin/bash   