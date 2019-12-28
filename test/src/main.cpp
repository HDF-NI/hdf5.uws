#include <exception>
#include <memory>
#include <string>

#include <csignal>

#include "App.h"
#include "helpers/AsyncFileReader.h"
#include "helpers/AsyncFileStreamer.h"
#include "helpers/Middleware.h"

#include "Url.h"
#include "mime_types.h"
#include "h5.h"
#include "h5images.h"

namespace
{
  volatile std::sig_atomic_t gSignalStatus;
}

void signal_handler(int signal)
{
    gSignalStatus = signal;
    std::cout<<"graceful attempt at shutting down other processes..."<<std::endl;
    std::cout << "SignalValue: " << gSignalStatus << '\n';
    std::exit(signal);
}

//Main function. 
int main(int argc, char *argv[])
{
  // Install a signal handler
  std::signal(SIGINT, signal_handler);
  try
  {
      pmc::mimetypes mimetypes("./node_modules/mime-db/db.json");
      pmc::h5 h5{.currentH5Path={"./newone.h5"}};
      h5.open();
      pmc::h5images<true> h5images{.h5=h5, .port={8888}};
      h5images.make.h5=h5;
      h5images.read.h5=h5;
      h5images.readRegion.h5=h5;
      h5images.readMosaic.h5=h5;
      
      AsyncFileStreamer asyncFileStreamer("test/examples");
uWS::SSLApp({

    /* There are tons of SSL options */
    .key_file_name = "../privkey.pem",
    .cert_file_name = "../fullchain.pem"
    
}).ws<pmc::H5SocketData>("/make_image/", std::move(h5images.make)).ws<pmc::H5SocketData>("/read_image/", std::move(h5images.read)).ws<pmc::H5SocketData>("/read_image_region/", std::move(h5images.readRegion)).ws<pmc::H5SocketData>("/read_image_mosaic/", std::move(h5images.readMosaic)).post("/create_group/*", [&h5](auto *res, auto *req) {
    std::cout<<"cg "<<req->getUrl()<<std::endl;
    std::string url(req->getUrl().begin(), req->getUrl().end());
    std::cout<<"cg2 "<<url::Url::unescape(url)<<std::endl;
    h5.createGroup(std::string_view(url::Url::unescape(url).substr(14)));
    res->writeStatus(uWS::HTTP_200_OK);
    res->end();
}).get("/get_image_info/*", [&h5images](auto *res, auto *req) {
    std::cout<<"gii "<<req->getUrl()<<std::endl;
    std::string url(req->getUrl().begin(), req->getUrl().end());
    std::string info=h5images.getInfo(std::string_view(url::Url::unescape(url).substr(16)));
    res->writeStatus(uWS::HTTP_200_OK);
    res->end(info);
}).post("/read_image_region/*", [&h5images](auto *res, auto *req) {
    std::cout<<"read_image_region "<<req->getUrl()<<std::endl;
}).get("/hello", [](auto *res, auto *req) {

    /* You can efficiently stream huge files too */
    res->writeHeader("Content-Type", "text/html; charset=utf-8")->end("Hello HTTP!");
    
/*}).ws<UserData>("/*", {

    .open = [](auto *ws, auto *req) {
        ws->subscribe("buzzword weekly");
    },
    .message = [](auto *ws, std::string_view message, uWS::OpCode opCode) {
        ws->send(message, opCode);
    }
*/    
}).get("/*", [&asyncFileStreamer, &mimetypes](auto *res, auto *req) {
            res->writeStatus(uWS::HTTP_200_OK);
            res->writeHeader("Content-Type", mimetypes.lookup(req->getUrl()));
            res->onAborted([]() {
                std::cout << "ABORTED!" << std::endl;
            });
            asyncFileStreamer.streamFile(res, req->getUrl());
    }).listen("localhost", 8888, [](auto *token) {

    if (token) {
        std::cout << "Listening on port " << 8888 << std::endl;
    }
    
}).run();
if (gSignalStatus) {
            std::cout << "W: interrupt received, killing server…" << std::endl;
            h5.close();
        }
  }
  catch (pmc::Exception& e)
  {
    std::cout <<"Exception\n"<< e.message << std::endl;
    return EXIT_FAILURE;
  }
  catch (std::exception& e)
  {
    std::cout << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}