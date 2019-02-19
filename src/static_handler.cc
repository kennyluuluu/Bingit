#include <boost/bind.hpp>
#include <iostream>
#include "static_handler.h"
#include <string>
#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>

static_handler::static_handler(const NginxConfig &config, const std::string& server_root)
{
  //TODO: use config object to set member variable handler_root
  //so handler knows where to serve file
  std::string loc_key = "location";
  handler_root = config.get_key_value(loc_key);
}

handler *static_handler::create(const NginxConfig &config, const std::string &root_path)
{
  return new static_handler(config, root_path);
}

std::unique_ptr<reply> static_handler::HandleRequest(const request &request)
{
  short code = -1;
  std::string mime_type = "";
  std::unordered_map<std::string, std::string> headers;
  std::string content = "";

  std::string path = handler_root + request.path;

  boost::filesystem::path path_object(path);
  std::string extension = "";
  if (path_object.has_extension())
  {
    extension = path_object.extension().string();
  }

  std::ifstream static_file(path.c_str());

  if (static_file.good())
  {
    code = 200;

    if (extension.compare(".txt") == 0)
    {
      mime_type = "text/plain";
    }
    else if (extension.compare(".html") == 0)
    {
      mime_type = "text/html";
    }
    else if (extension.compare(".jpeg") == 0 ||
             extension.compare(".jpg"))
    {
      mime_type = "image/jpeg";
    }
    else if (extension.compare(".png") == 0)
    {
      mime_type = "image/png";
    }
    else if (extension.compare(".zip") == 0)
    {
      mime_type = "application/zip";
    }
    else
    {
      mime_type += "text/plain";
    }

    headers["Content-Type"] = mime_type;

    // https://stackoverflow.com/questions/2602013/read-whole-ascii-file-into-c-stdstring
    std::stringstream buffer;
    buffer << static_file.rdbuf();

    content = buffer.str();

    //add content length of file
  }
  else
  {
    BOOST_LOG_TRIVIAL(info) << "Path does not exist, responding with 404";

    code = 404;
    mime_type = "text/plain";
    content = "404 Error: Page not found";
  }

  headers["Content-Type"] = mime_type;
  headers["Content-Length"] = std::to_string(content.size());

  return std::unique_ptr<reply>(new reply(request.http_version, code, mime_type, content, headers));
}