#pragma once
#include <exception>
#include <fstream>
#include <sstream>
#include <string_view>

#include "hdf5.h"
#include "hdf5_hl.h"
#include "H5Lpublic.h"

namespace pmc{
    
    class Exception : public std::exception {
    public:
        std::string message;
    public:
        Exception()
        {
        }
        Exception(std::string message)
        {
            this->message=message;
        }
        Exception(const Exception& orig)
        {
        }
        virtual ~Exception() throw()
        {
        }
        virtual const char* what(){return message.c_str();};

    protected:

    };

    struct h5{
        std::string currentH5Path={"./newone.h5"};
        
        hid_t       id      = -1;
        hid_t       gcpl_id = 0;
        hid_t plist_id, gcpl, dtpl_id, dapl_id, dcpl_id;

        unsigned int compression = 0;
        bool         error       = false;
    
        void open(){
            createH5(std::string_view(currentH5Path));
        };
        
        void close(){
            if (id > 0) {
              H5Fclose(id);
            }
        };
        
        void createH5(std::string_view path) {
            bool exists = std::ifstream(path.data()).good();
            if (exists) {
              id = H5Fopen(path.data(), H5F_ACC_RDWR, H5P_DEFAULT);
              if (id < 0) {
//                std::stringstream ss;
//                ss << "Failed to read file, with return: " << id << ".\n";
//                error = true;
//                throw  Exception(ss.str());
//                return;
              }
            }
            unsigned int flags=H5F_ACC_TRUNC;
            if(!exists && id < 0 && (flags & (H5F_ACC_EXCL | H5F_ACC_TRUNC | H5F_ACC_DEBUG))) {
              plist_id = H5Pcreate(H5P_FILE_ACCESS);
              id       = H5Fcreate(path.data(), flags, H5P_DEFAULT, plist_id);
              if (id < 0) {
                std::stringstream ss;
                ss << "Failed to create file, with return: " << id << ".\n";
                error = true;
                throw  Exception(ss.str());
                return;
              }
            } else {
              bool exists = std::ifstream(path.data()).good();
              if (!exists) {
                std::stringstream ss;
                ss << "File " << path << " doesn't exist.";
                error = true;
                throw  Exception(ss.str());
                return;
              }
              id = H5Fopen(path.data(), flags, H5P_DEFAULT);
              if (id < 0) {
                std::stringstream ss;
                ss << "Failed to open file, " << path << " and flags " << flags << " with return: " << id << ".";
                error = true;
                throw  Exception(ss.str());
                return;
              }
            }

            gcpl       = H5Pcreate(H5P_GROUP_CREATE);
            herr_t err = H5Pset_link_creation_order(gcpl, H5P_CRT_ORDER_TRACKED | H5P_CRT_ORDER_INDEXED);
            if (err < 0) {
              std::stringstream ss;
              ss << "Failed to set link creation order, with return: " << err << ".\n";
              error =true;
              throw  Exception(ss.str());
            }
            
        };
        
        void createGroup(std::string_view path) {
            std::cout<<"path "<<path<<std::endl;
    std::vector<std::string> trail;
    std::vector<hid_t>       hidPath;
    std::istringstream       buf(path.data());
    for (std::string token; getline(buf, token, '/');)
      if (!token.empty())
        trail.push_back(token);
    hid_t previous_hid = id;
    bool  created      = false;
    for (unsigned int index = 0; index < trail.size(); index++) {
      // check existence of stem
      if (H5Lexists(previous_hid, trail[index].c_str(), H5P_DEFAULT)) {
        hid_t hid = H5Gopen(previous_hid, trail[index].c_str(), H5P_DEFAULT);
        if (hid >= 0) {
          if (index < trail.size() - 1)
            hidPath.push_back(hid);
          previous_hid = hid;
          continue;
        }
      }

      // create group
      hid_t hid = H5Gcreate(previous_hid, trail[index].c_str(), H5P_DEFAULT, gcpl, H5P_DEFAULT);
      if (hid < 0) {
        std::string desc;
        {
          H5Ewalk2(H5Eget_current_stack(),
                   H5E_WALK_UPWARD,
                   [](unsigned int n, const H5E_error2_t* err_desc, void* client_data) -> herr_t {
                     if (((std::string*)client_data)->empty())
                       ((std::string*)client_data)->assign(err_desc[0].desc, strlen(err_desc[0].desc));
                     return 0;
                   },
                   (void*)&desc);
        }
        desc = "Group create failed: " + desc;
        throw Exception(desc);
        return;
      }

      if (index < trail.size() - 1) {
        hidPath.push_back(hid);
      }

      if (index == trail.size() - 1) {
//        Group* group = new Group(hid);
//        group->name.assign(trail[index].c_str());
//        group->gcpl_id = H5Pcreate(H5P_GROUP_CREATE);
//        herr_t err     = H5Pset_link_creation_order(group->gcpl_id, args[2]->IntegerValue(context).ToChecked());
//        if (err < 0) {
//          throw Exception("Failed to set link creation order");
//          return;
//        }
//
//        created = true;
      }
      previous_hid = hid;
    }
    if (!created) {
//      Group* group = new Group(previous_hid);
//      group->name.assign(trail[trail.size() - 1].c_str());
//      group->gcpl_id = H5Pcreate(H5P_GROUP_CREATE);
//      herr_t err     = H5Pset_link_creation_order(group->gcpl_id, args[2]->IntegerValue(context).ToChecked());
//      if (err < 0) {
//        throw Exception("Failed to set link creation order");
//        args.GetReturnValue().SetUndefined();
//        return;
//      }
//
//      created = true;
    }
            
        };
        
        void renameGroup(std::string_view path) {
            
        };
        
        void setCompression(std::string_view path) {
            
        };
        
        std::tuple<std::string_view, std::string_view> toStemAndLeaf(std::string_view path){
            size_t index=path.find_last_of('/');
            if(index != std::string::npos){
                return std::make_tuple(path.substr(0, index), path.substr(index+1, path.length()));
            }
            else
                return std::make_tuple("", std::string(path.begin(), path.end()));
        }
        
        hid_t openStem(std::string_view stem, std::vector<hid_t> hidPath){
            std::vector<std::string> trail;
            std::istringstream       buf(stem.data());
            for (std::string token; getline(buf, token, '/');) {
              if (!token.empty()) {
                trail.push_back(token);
              }
            }

            hid_t previous_hid =id;

            for (unsigned int index = 0; index < trail.size() - 1; index++) {
              // check existence of stem
              if (H5Lexists(previous_hid, trail[index].c_str(), H5P_DEFAULT)) {
                hid_t hid = H5Gopen(previous_hid, trail[index].c_str(), H5P_DEFAULT);
                if (hid >= 0) {
                  if (index < trail.size() - 1)
                    hidPath.push_back(hid);
                  previous_hid = hid;
                  continue;
                } else {
                  std::string msg = "Group" + trail[index] + " doesn't exist";
                  throw Exception(msg);
                }
              } else {
                std::string msg = "Group" + trail[index] + " doesn't exist";
                throw Exception(msg);
              }
            }
            return previous_hid;
        }
        
        void closeStem(std::vector<hid_t> hidPath){
            for (std::vector<hid_t>::reverse_iterator it = hidPath.rbegin(); it != hidPath.rend(); ++it)
              H5Gclose(*it);            
        }
    };
    
    struct H5SocketData {
        int msgCount=0;
        std::string path;
        std::string metaData;

    };
}
