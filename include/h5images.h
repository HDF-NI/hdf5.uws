#pragma once
#include <exception>
#include <fstream>
#include <sstream>
#include <string_view>
#include <memory>

#include "hdf5.h"
#include "hdf5_hl.h"
#include "H5Lpublic.h"

#include "h5.h"

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h> 

#include "rapidjson/stringbuffer.h"
#include "rapidjson/reader.h"
#include "rapidjson/writer.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/filewritestream.h"

namespace pmc{
    
    struct image_meta : rapidjson::BaseReaderHandler<> {
        unsigned int width = 0;
        unsigned int height = 0;
        unsigned int planes = 0;
        unsigned int npals = 0;
        unsigned int size = 0;
        std::unique_ptr<hsize_t[]> start;
        std::unique_ptr<hsize_t[]> stride;
        std::unique_ptr<hsize_t[]> count;
        int boundary = 0;
        
        short state=-1;
        unsigned int index=0;

                bool Uint(unsigned int ui) {
                    switch(state)
                    {
                        case 0:
                            width = ui;
                            state=-1;
                            break;
                        case 1:
                            height = ui;
                            state=-1;
                            break;
                        case 2:
                            planes = ui;
                            state=-1;
                            break;
                        case 3:
                            npals = ui;
                            state=-1;
                            break;
                        case 4:
                            size = ui;
                            state=-1;
                            break;
                        case 5:
                            start[index++]=ui;
                            break;
                        case 6:
                            stride[index++]=ui;
                            break;
                        case 7:
                            count[index++]=ui;
                            break;
                        case 8:
                            boundary = ui;
                            state=-1;
                            break;
                    }
                    return true;
                }
                
                bool Int(int ui) {
                    switch(state)
                    {
                        case 0:
                            width = ui;
                            state=-1;
                            break;
                        case 1:
                            height = ui;
                            state=-1;
                            break;
                        case 2:
                            planes = ui;
                            state=-1;
                            break;
                        case 3:
                            npals = ui;
                            state=-1;
                            break;
                        case 4:
                            size = ui;
                            state=-1;
                            break;
                        case 5:
                            start[index++]=ui;
                            break;
                        case 6:
                            stride[index++]=ui;
                            break;
                        case 7:
                            count[index++]=ui;
                            break;
                        case 8:
                            boundary = ui;
                            state=-1;
                            break;
                    }
                    return true;
                }
                
        bool StartArray() {
            index=0;
            switch(state)
            {
                case 5:
                    start=std::make_unique<hsize_t[]>(9);
                    break;
                case 6:
                    stride=std::make_unique<hsize_t[]>(9);
                    break;
                case 7:
                    count=std::make_unique<hsize_t[]>(9);
                    break;
            }
            return true;
        }
        bool EndArray(rapidjson::SizeType elementCount) {
            switch(state)
            {
                case 5:
                {
                    auto startTmp=std::make_unique<hsize_t[]>(index);
                    start.swap(startTmp);
                    for(unsigned int i=0;i<index;i++)start[i]=startTmp[i];
                }
                    break;
                case 6:
                {
                    auto strideTmp=std::make_unique<hsize_t[]>(index);
                    stride.swap(strideTmp);
                    for(unsigned int i=0;i<index;i++)stride[i]=strideTmp[i];
                }
                    break;
                case 7:
                {
                    auto countTmp=std::make_unique<hsize_t[]>(index);
                    count.swap(countTmp);
                    for(unsigned int i=0;i<index;i++)count[i]=countTmp[i];
                }
                    break;
            }
                    state=-1;
            return true;
        }
    
        bool Key(const Ch* str, rapidjson::SizeType len, bool copy) {
            if (std::strcmp(str, "width") == 0) {
                state = 0;
            }
            else if (std::strcmp(str, "height") == 0) {
                state = 1;
            }
            else if (std::strcmp(str, "planes") == 0) {
                state = 2;
            }
            else if (std::strcmp(str, "npals") == 0) {
                state = 3;
            }
            else if (std::strcmp(str, "size") == 0) {
                state = 4;
            }
            else if (std::strcmp(str, "start") == 0) {
                state = 5;
            }
            else if (std::strcmp(str, "stride") == 0) {
                state = 6;
            }
            else if (std::strcmp(str, "count") == 0) {
                state = 7;
            }
            else if (std::strcmp(str, "boundary") == 0) {
                state = 8;
            }
            return true;
        }
        
    };

template <bool SSL>
struct MakeImage : public uWS::TemplatedApp<SSL>::WebSocketBehavior{
    using uWS::TemplatedApp<SSL>::WebSocketBehavior::compression;
    using uWS::TemplatedApp<SSL>::WebSocketBehavior::maxPayloadLength;
    using uWS::TemplatedApp<SSL>::WebSocketBehavior::idleTimeout;
    using uWS::TemplatedApp<SSL>::WebSocketBehavior::open;
    using uWS::TemplatedApp<SSL>::WebSocketBehavior::message;
    using uWS::TemplatedApp<SSL>::WebSocketBehavior::drain;
    using uWS::TemplatedApp<SSL>::WebSocketBehavior::ping;
    using uWS::TemplatedApp<SSL>::WebSocketBehavior::pong;
    using uWS::TemplatedApp<SSL>::WebSocketBehavior::close;
    
    MakeImage(){
        compression = uWS::SHARED_COMPRESSOR;
        maxPayloadLength = 1024 * 1024 * 1024;
        idleTimeout = 60;
        open = [](auto *ws, auto *req) {
            std::cout<<"mi open "<<req->getUrl()<<std::endl;
            static_cast<H5SocketData*>(ws->getUserData())->msgCount=0;
        };
        message = [&](auto *ws, std::string_view message, uWS::OpCode opCode) {
            switch(opCode){
                case uWS::TEXT:
                    if(static_cast<H5SocketData*>(ws->getUserData())->msgCount==0){
                        static_cast<H5SocketData*>(ws->getUserData())->path=std::string(message.begin(), message.end());
//                        std::cout<<"path "<<path<<std::endl;
                        static_cast<H5SocketData*>(ws->getUserData())->msgCount++;
                    }
                    else{
                        static_cast<H5SocketData*>(ws->getUserData())->metaData=std::string(message.begin(), message.end());
                    }
                    break;
                case uWS::BINARY:
                    std::cout<<"metaData "<<static_cast<H5SocketData*>(ws->getUserData())->metaData<<std::endl;
                    std::string_view stem, leaf;
                    std::tie (stem, leaf)=h5.toStemAndLeaf(static_cast<H5SocketData*>(ws->getUserData())->path);
                    std::cout<<"stem "<<stem<<" "<<leaf<<std::endl;
                    std::vector<hid_t>       hidPath;
                    hid_t       previous_hid=h5.openStem(stem, hidPath);
                    rapidjson::InsituStringStream s((char*)static_cast<H5SocketData*>(ws->getUserData())->metaData.c_str());
                    image_meta imeta;
                    rapidjson::Reader reader;
                    reader.Parse<rapidjson::kParseInsituFlag>(s, imeta);
                    std::string interlace="interlace";
                    herr_t            err;
                    hsize_t           dims[3];
                    dims[0]=imeta.height;
                    dims[1]=imeta.width;
                    dims[2]=imeta.planes;
                    std::cout<<"dims "<<dims[0]<<" "<<dims[1]<<" "<<dims[2]<<" "<<" "<<message.length()<<std::endl;
                    err           = H5LTmake_dataset(previous_hid, leaf.data(), 3, dims, H5T_NATIVE_UCHAR, (const char*)message.data());
                    if (err < 0) {
                      throw Exception("failed to make image dataset");
                    }
                    H5LTset_attribute_string(previous_hid, leaf.data(), "CLASS", "IMAGE");
                    H5LTset_attribute_string(previous_hid, leaf.data(), "IMAGE_SUBCLASS", "IMAGE_BITMAP");
                    H5LTset_attribute_string(previous_hid, leaf.data(), "IMAGE_SUBCLASS", "IMAGE_TRUECOLOR");
                    H5LTset_attribute_string(previous_hid, leaf.data(), "IMAGE_VERSION", "1.2");
                    H5LTset_attribute_string(previous_hid, leaf.data(), "INTERLACE_MODE", "INTERLACE_PIXEL");
                    std::unique_ptr<unsigned char[]> rangeBuffer(new unsigned char[(size_t)(2)]);
                    rangeBuffer.get()[0] = (unsigned char)0;
                    rangeBuffer.get()[1] = (unsigned char)255;
                    H5LTset_attribute_uchar(previous_hid, leaf.data(), "IMAGE_MINMAXRANGE", rangeBuffer.get(), 2);

                    h5.closeStem(hidPath);
                    break;
            }
        };
        drain = [](auto *ws) {
            /* Check getBufferedAmount here */
        };
        ping = [](auto *ws) {

        };
        pong = [](auto *ws) {

        };
        close = [](auto *ws, int code, std::string_view message) {
            static_cast<H5SocketData*>(ws->getUserData())->msgCount=0;
        };
    };
    
    pmc::h5 h5;
};
    
template <bool SSL>
struct ReadImage : public uWS::TemplatedApp<SSL>::WebSocketBehavior{
    using uWS::TemplatedApp<SSL>::WebSocketBehavior::compression;
    using uWS::TemplatedApp<SSL>::WebSocketBehavior::maxPayloadLength;
    using uWS::TemplatedApp<SSL>::WebSocketBehavior::idleTimeout;
    using uWS::TemplatedApp<SSL>::WebSocketBehavior::open;
    using uWS::TemplatedApp<SSL>::WebSocketBehavior::message;
    using uWS::TemplatedApp<SSL>::WebSocketBehavior::drain;
    using uWS::TemplatedApp<SSL>::WebSocketBehavior::ping;
    using uWS::TemplatedApp<SSL>::WebSocketBehavior::pong;
    using uWS::TemplatedApp<SSL>::WebSocketBehavior::close;
    
    
    ReadImage(){
        compression = uWS::SHARED_COMPRESSOR;
        maxPayloadLength = 1024 * 1024 * 1024;
        idleTimeout = 10;
        open = [](auto *ws, auto *req) {
            std::cout<<"mi open "<<req->getUrl()<<std::endl;
        };
        message = [&](auto *ws, std::string_view message, uWS::OpCode opCode) {
            switch(opCode){
                case uWS::TEXT:
                        static_cast<H5SocketData*>(ws->getUserData())->path=std::string(message.begin(), message.end());
                    //std::cout<<"metaData "<<static_cast<H5SocketData*>(ws->getUserData())->metaData<<std::endl;
                    std::string_view stem, leaf;
                    std::tie (stem, leaf)=h5.toStemAndLeaf(static_cast<H5SocketData*>(ws->getUserData())->path);
                    //std::cout<<"stem "<<stem<<" "<<leaf<<std::endl;
                    std::vector<hid_t>       hidPath;
                    hid_t previous_hid=h5.openStem(stem, hidPath);
                    hsize_t           width;
                    hsize_t           height;
                    hsize_t           planes;
                    char              interlace[255];
                    hssize_t          npals;
                    herr_t            err    = H5IMget_image_info(previous_hid, leaf.data(), &width, &height, &planes, interlace, &npals);
                    if (err < 0) {
                      throw Exception("failed to get image info");
                    }
                    rapidjson::StringBuffer s;
                    rapidjson::Writer<rapidjson::StringBuffer> writer(s);
                    writer.StartObject();
                    writer.String("width");
                    writer.Uint(width);
                    writer.String("height");
                    writer.Uint(height);
                    writer.String("planes");
                    writer.Uint(planes);
                    writer.String("interlace");
                    writer.String(interlace);
                    writer.String("npals");
                    writer.Uint(npals);
                    writer.EndObject();
                    ws->send(s.GetString(), opCode);
                    std::unique_ptr<unsigned char[]> contentBuffer(new unsigned char[(size_t)(planes * width * height)]);
                    err = H5LTread_dataset(previous_hid, leaf.data(), H5T_NATIVE_UCHAR, contentBuffer.get());
                    h5.closeStem(hidPath);
                    if (err < 0) {
                      throw Exception("failed to read image");
                    }
                    std::string buf((char*)contentBuffer.get(), (size_t)(planes * width * height));
                    ws->send(std::string_view(buf));                    
                    ws->end(uWS::CLOSE);
                    h5.closeStem(hidPath);
                    break;
            }
        };
        drain = [](auto *ws) {
            /* Check getBufferedAmount here */
        };
        ping = [](auto *ws) {

        };
        pong = [](auto *ws) {

        };
        close = [](auto *ws, int code, std::string_view message) {

        };
    };
    
    pmc::h5 h5;
};
    
template <bool SSL>
struct ReadImageRegion : public uWS::TemplatedApp<SSL>::WebSocketBehavior{
    using uWS::TemplatedApp<SSL>::WebSocketBehavior::compression;
    using uWS::TemplatedApp<SSL>::WebSocketBehavior::maxPayloadLength;
    using uWS::TemplatedApp<SSL>::WebSocketBehavior::idleTimeout;
    using uWS::TemplatedApp<SSL>::WebSocketBehavior::open;
    using uWS::TemplatedApp<SSL>::WebSocketBehavior::message;
    using uWS::TemplatedApp<SSL>::WebSocketBehavior::drain;
    using uWS::TemplatedApp<SSL>::WebSocketBehavior::ping;
    using uWS::TemplatedApp<SSL>::WebSocketBehavior::pong;
    using uWS::TemplatedApp<SSL>::WebSocketBehavior::close;
    
    
    ReadImageRegion(){
        compression = uWS::SHARED_COMPRESSOR;
        maxPayloadLength = 16 * 1024 * 1024;
        idleTimeout = 10;
        open = [](auto *ws, auto *req) {
            std::cout<<"mi open "<<req->getUrl()<<std::endl;
        };
        message = [&](auto *ws, std::string_view message, uWS::OpCode opCode) {
            switch(opCode){
                case uWS::TEXT:
                        static_cast<H5SocketData*>(ws->getUserData())->path=std::string(message.begin(), message.end());
                    //std::cout<<"metaData "<<static_cast<H5SocketData*>(ws->getUserData())->metaData<<std::endl;
                    std::string_view stem, leaf;
                    std::tie (stem, leaf)=h5.toStemAndLeaf(static_cast<H5SocketData*>(ws->getUserData())->path);
                    //std::cout<<"stem "<<stem<<" "<<leaf<<std::endl;
                    std::vector<hid_t>       hidPath;
                    hid_t previous_hid=h5.openStem(stem, hidPath);
                    hsize_t           width;
                    hsize_t           height;
                    hsize_t           planes;
                    char              interlace[255];
                    hssize_t          npals;
                    herr_t            err    = H5IMget_image_info(previous_hid, leaf.data(), &width, &height, &planes, interlace, &npals);
                    if (err < 0) {
                      throw Exception("failed to get image info");
                    }
                    rapidjson::StringBuffer s;
                    rapidjson::Writer<rapidjson::StringBuffer> writer(s);
                    writer.StartObject();
                    writer.String("width");
                    writer.Uint(width);
                    writer.String("height");
                    writer.Uint(height);
                    writer.String("planes");
                    writer.Uint(planes);
                    writer.String("interlace");
                    writer.String(interlace);
                    writer.String("npals");
                    writer.Uint(npals);
                    writer.EndObject();
                    ws->send(s.GetString(), opCode);
                    std::unique_ptr<unsigned char[]> contentBuffer(new unsigned char[(size_t)(planes * width * height)]);
                    err = H5LTread_dataset(previous_hid, leaf.data(), H5T_NATIVE_UCHAR, contentBuffer.get());
                    h5.closeStem(hidPath);
                    if (err < 0) {
                      throw Exception("failed to read image");
                    }
                    std::string buf((char*)contentBuffer.get(), (size_t)(planes * width * height));
                    ws->send(std::string_view(buf));                    
                    h5.closeStem(hidPath);
                    break;
            }
        };
        drain = [](auto *ws) {
            /* Check getBufferedAmount here */
        };
        ping = [](auto *ws) {

        };
        pong = [](auto *ws) {

        };
        close = [](auto *ws, int code, std::string_view message) {

        };
    };
    
    pmc::h5 h5;
};
    
template <bool SSL>
struct ReadImageMosaic : public uWS::TemplatedApp<SSL>::WebSocketBehavior{
    using uWS::TemplatedApp<SSL>::WebSocketBehavior::compression;
    using uWS::TemplatedApp<SSL>::WebSocketBehavior::maxPayloadLength;
    using uWS::TemplatedApp<SSL>::WebSocketBehavior::idleTimeout;
    using uWS::TemplatedApp<SSL>::WebSocketBehavior::open;
    using uWS::TemplatedApp<SSL>::WebSocketBehavior::message;
    using uWS::TemplatedApp<SSL>::WebSocketBehavior::drain;
    using uWS::TemplatedApp<SSL>::WebSocketBehavior::ping;
    using uWS::TemplatedApp<SSL>::WebSocketBehavior::pong;
    using uWS::TemplatedApp<SSL>::WebSocketBehavior::close;
    
    
    ReadImageMosaic(){
        compression = uWS::SHARED_COMPRESSOR;
        maxPayloadLength = 1024 * 1024 * 1024;
        idleTimeout = 120;
        open = [](auto *ws, auto *req) {
            std::cout<<"mi open "<<req->getUrl()<<std::endl;
            static_cast<H5SocketData*>(ws->getUserData())->msgCount=0;
        };
        message = [&](auto *ws, std::string_view message, uWS::OpCode opCode) {
            switch(opCode){
                case uWS::TEXT:
                    if(static_cast<H5SocketData*>(ws->getUserData())->msgCount==0){
                        static_cast<H5SocketData*>(ws->getUserData())->path=std::string(message.begin(), message.end());
//                        std::cout<<"path "<<path<<std::endl;
                        static_cast<H5SocketData*>(ws->getUserData())->msgCount++;
                    }
                    else{
                        static_cast<H5SocketData*>(ws->getUserData())->metaData=std::string(message.begin(), message.end());
                    std::cout<<"metaData "<<static_cast<H5SocketData*>(ws->getUserData())->metaData<<std::endl;
                    rapidjson::InsituStringStream s((char*)static_cast<H5SocketData*>(ws->getUserData())->metaData.c_str());
                    image_meta imeta;
                    rapidjson::Reader reader;
                    reader.Parse<rapidjson::kParseInsituFlag>(s, imeta);
                    std::string_view stem, leaf;
                    std::tie (stem, leaf)=h5.toStemAndLeaf(static_cast<H5SocketData*>(ws->getUserData())->path);
                    std::cout<<"stem "<<stem<<" "<<leaf<<std::endl;
                    std::vector<hid_t>       hidPath;
                    hid_t previous_hid=h5.openStem(stem, hidPath);
                    hsize_t           width;
                    hsize_t           height;
                    hsize_t           planes;
                    char              interlace[255];
                    hssize_t          npals;
                    herr_t            err    = H5IMget_image_info(previous_hid, leaf.data(), &width, &height, &planes, interlace, &npals);
                    if (err < 0) {
                      throw Exception("failed to get image info");
                    }
                    int rank = imeta.index;//std::size(imeta.start.get());
                    hsize_t theSize = 1;
                    for (int rankIndex = 0; rankIndex < rank; rankIndex++) {
                      theSize *= imeta.count.get()[rankIndex];
                    }
                    std::cout<<"rank "<<rank<<std::endl;
                    rapidjson::StringBuffer sb;
                    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
                    //var metaData={name: leaf, startX: 0, startY: 0, width: buffer.width, height: buffer.height, planes: buffer.planes, npals: buffer.planes, size: size}
                    writer.StartObject();
                    writer.String("name");
                    writer.String(leaf.data());
                    writer.String("startX");
                    writer.Uint(0);
                    writer.String("startY");
                    writer.Uint(0);
                    writer.String("width");
                    writer.Uint(imeta.count.get()[0]);
                    writer.String("height");
                    writer.Uint(imeta.count.get()[1]);
                    writer.String("planes");
                    writer.Uint(planes);
                    writer.String("interlace");
                    writer.String(interlace);
                    writer.String("npals");
                    writer.Uint(npals);
                    writer.String("size");
                    writer.Uint(theSize);
                    writer.EndObject();
                    ws->send(sb.GetString(), opCode);
      hid_t did          = H5Dopen(previous_hid, leaf.data(), H5P_DEFAULT);
      hid_t t            = H5Dget_type(did);
      hid_t type_id      = H5Tget_native_type(t, H5T_DIR_ASCEND);
      hid_t dataspace_id = H5Dget_space(did);
      hid_t memspace_id  = H5Screate_simple(rank, imeta.count.get(), NULL);
                    std::cout<<"H5Sselect_hyperslab "<<rank<<" "<<imeta.boundary<<std::endl;
      err                = H5Sselect_hyperslab(dataspace_id, H5S_SELECT_SET, imeta.start.get(), imeta.stride.get(), imeta.count.get(), NULL);
      if (err < 0) {
        H5Sclose(memspace_id);
        H5Sclose(dataspace_id);
        H5Dclose(did);
        throw Exception("failed to select hyperslab");
      }


      std::unique_ptr<unsigned char[]> contentBuffer(new unsigned char[(size_t)(theSize)]);
      err = H5Dread(did, type_id, memspace_id, dataspace_id, H5P_DEFAULT, (char*)contentBuffer.get());

      if (err < 0) {
        H5Sclose(memspace_id);
        H5Sclose(dataspace_id);
        H5Tclose(t);
        H5Dclose(did);
        throw Exception("failed to read image region");
      }

                    std::string buf((char*)contentBuffer.get(), (size_t)(theSize));
                    ws->send(std::string_view(buf));
                    std::cout<<"imeta.boundary "<<imeta.boundary<<std::endl;
                    hsize_t originStartX=imeta.start[0];
                    hsize_t originStartY=imeta.start[1];
                    for(int boundary=1;boundary<=imeta.boundary;boundary++){
                    for(int j=-boundary;j<=boundary;j++){
                        imeta.start[1]=originStartY+j*imeta.count[1];
                    for(int i=-boundary;i<=boundary;i++){
                        if(i>-boundary && j>-boundary && i<boundary && j<boundary)continue;
                        imeta.start[0]=originStartX+i*imeta.count[0];
                        rapidjson::StringBuffer sb;
                        rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
                        writer.StartObject();
                        writer.String("name");
                        writer.String(leaf.data());
                        writer.String("startX");
                        writer.Uint(i*imeta.count.get()[0]);
                        writer.String("startY");
                        writer.Uint(j*imeta.count.get()[1]);
                        writer.String("width");
                        writer.Uint(imeta.count.get()[0]);
                        writer.String("height");
                        writer.Uint(imeta.count.get()[1]);
                        writer.String("planes");
                        writer.Uint(planes);
                        writer.String("interlace");
                        writer.String(interlace);
                        writer.String("npals");
                        writer.Uint(npals);
                        writer.String("size");
                        writer.Uint(theSize);
                        writer.EndObject();
                        ws->send(sb.GetString(), opCode);
                        err                = H5Sselect_hyperslab(dataspace_id, H5S_SELECT_SET, imeta.start.get(), imeta.stride.get(), imeta.count.get(), NULL);
                        if (err < 0) {
                          H5Sclose(memspace_id);
                          H5Sclose(dataspace_id);
                          H5Dclose(did);
                          throw Exception("failed to select hyperslab");
                        }
                        std::unique_ptr<unsigned char[]> contentBuffer(new unsigned char[(size_t)(theSize)]);
                        err = H5Dread(did, type_id, memspace_id, dataspace_id, H5P_DEFAULT, (char*)contentBuffer.get());

                        if (err < 0) {
                          H5Sclose(memspace_id);
                          H5Sclose(dataspace_id);
                          H5Tclose(t);
                          H5Dclose(did);
                          throw Exception("failed to read image region");
                        }
                    std::string buf((char*)contentBuffer.get(), (size_t)(theSize));
                    ws->send(std::string_view(buf));
                    }
                    }
                    }
      H5Tclose(t);
      H5Sclose(memspace_id);
      H5Sclose(dataspace_id);
      H5Dclose(did);
                    
                    ws->end(uWS::CLOSE);
                    h5.closeStem(hidPath);
                    }
                    break;
            }
        };
        drain = [](auto *ws) {
            /* Check getBufferedAmount here */
                    //std::cout<<"getBufferedAmount "<<ws->getBufferedAmount()<<std::endl;
        };
        ping = [](auto *ws) {
            ws->send(std::string_view(""), uWS::PONG);
        };
        pong = [](auto *ws) {

        };
        close = [](auto *ws, int code, std::string_view message) {
                    //std::cout<<"got closed "<<std::endl;

        };
    };
    
    pmc::h5 h5;
};
    
template <bool SSL>
    struct h5images{
        pmc::h5 h5;
        int port;
        MakeImage<SSL> make;
        ReadImage<SSL> read;
        ReadImageRegion<SSL> readRegion;
        ReadImageMosaic<SSL> readMosaic;
        
        std::string getInfo(std::string_view path) {
            std::string_view stem, leaf;
            std::tie (stem, leaf)=h5.toStemAndLeaf(path);
            std::vector<hid_t>       hidPath;
            hid_t       previous_hid=h5.openStem(stem, hidPath);
            hsize_t           width;
            hsize_t           height;
            hsize_t           planes;
            char              interlace[255];
            hssize_t          npals;
            herr_t            err    = H5IMget_image_info(previous_hid, leaf.data(), &width, &height, &planes, interlace, &npals);
            h5.closeStem(hidPath);
            if (err < 0) {
              throw Exception("failed to get image info");
            }
            rapidjson::StringBuffer s;
            rapidjson::Writer<rapidjson::StringBuffer> writer(s);
            writer.StartObject();
            writer.String("width");
            writer.Uint(width);
            writer.String("height");
            writer.Uint(height);
            writer.String("planes");
            writer.Uint(planes);
            writer.String("interlace");
            writer.String(interlace);
            writer.String("npals");
            writer.Uint(npals);
            writer.EndObject();
            return s.GetString();
        };
        
        void makeImage(std::string_view path) {
            
        };
        
        void readImage(std::string_view path, std::function<void()> cb) {
            
        };
        
        
    };
}
