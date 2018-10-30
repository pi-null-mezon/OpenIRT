FROM alextaranov/ubuntu18

COPY /Sources /Sources 
COPY /Shared /Shared 
COPY /Apps /Apps

# Install download tool
RUN apt-get install -y wget

# Download resources 
RUN wget https://github.com/davisking/dlib-models/blob/master/dlib_face_recognition_resnet_model_v1.dat.bz2?raw=true && \
    mv dlib_face_recognition_resnet_model_v1.dat.bz2?raw=true dlib_face_recognition_resnet_model_v1.dat.bz2 && \
    bzip2 -d dlib_face_recognition_resnet_model_v1.dat.bz2 && \
    mv dlib_face_recognition_resnet_model_v1.dat /usr/local/bin && \
    wget https://github.com/davisking/dlib-models/blob/master/shape_predictor_5_face_landmarks.dat.bz2?raw=true && \
    mv shape_predictor_5_face_landmarks.dat.bz2?raw=true shape_predictor_5_face_landmarks.dat.bz2 && \
    bzip2 -d shape_predictor_5_face_landmarks.dat.bz2 && \
    mv shape_predictor_5_face_landmarks.dat /usr/local/bin

# Build oirtcli 
RUN cd Apps/oirtcli && \
    mkdir build && cd build && \
    qmake ../oirtcli.pro && \
    make && \
    make install && \
    cd ../ && rm -rf build && cd ~
	
# Build oirtsrv 
RUN cd Apps/Face/oirtsrv && \
    mkdir build && cd build && \
    qmake ../oirtsrv.pro && \
    make -j2 && \
    make install && \
    cd ../ && rm -rf build && cd ~ && \
	mkdir -p /var/facerec
	
# Prepare web server
RUN mkdir /home/Testdata && \
	echo "python3 /Apps/Face/oirtweb/oirtwebsrv.py &" > startwebsrv && \
	chmod +x startwebsrv

# Port forwarding
EXPOSE 5000

# Enable unicode support
ENV LANG en_US.UTF-8
	
# Run server	
CMD ./startwebsrv ; oirtsrv -a127.0.0.1 -l/var/facerec/labels.yml
    
