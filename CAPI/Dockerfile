FROM ubuntu:18.04
Maintainer eesast.com
WORKDIR /usr/local
RUN mkdir /usr/local/mnt
RUN apt-get update && apt-get install --no-install-recommends -y gcc g++ make libprotobuf-dev wget curl unzip ca-certificates
RUN wget https://cloud.tsinghua.edu.cn/f/cf904973cfc44862a937/?dl=1
RUN unzip index.html\?dl=1 && rm index.html\?dl=1
RUN ls && pwd && tar -zxvf cmake.tar.gz && ls
ENV PATH=$PATH:/usr/local/cmake-3.15.7-Linux-x86_64/bin
COPY compile.sh /usr/local
RUN ls\
	&& tar -zxvf protobuf.tar.gz && rm protobuf.tar.gz && cd protobuf-3.8.0-rc-1 && ./configure --prefix=/usr/local/protobuf && make && make install \
	&& ls && cp /usr/local/protobuf/bin/protoc /usr/bin/protoc
ENV LD_LIBRARY_PATH=/usr/local/protobuf/lib
ENV Protobuf_INCLUDE_DIR=/usr/local/protobuf/include
ADD CAPI /usr/local/CAPI
RUN ls && cd CAPI/CAPI
RUN wget https://cloud.tsinghua.edu.cn/f/92af608253d146c5bec5/?dl=1 
RUN unzip index.html\?dl=1 && rm index.html\?dl=1 && cd ../../
RUN apt-get remove -y libprotobuf-dev && cp -r /usr/local/protobuf/include/* /usr/include/ && cp -r /usr/local/protobuf/lib/*.* /usr/lib/x86_64-linux-gnu/
RUN rm /usr/local/cmake.tar.gz
ENTRYPOINT ["bash", "compile.sh"]
