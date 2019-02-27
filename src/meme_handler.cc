#include "meme_handler.h"
#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>
#include <boost/bind.hpp>

meme_handler::meme_handler(const NginxConfig &config, const std::string &server_root)
{
    std::string key = "root";
    std::string config_root = config.get_key_value(key);

    //if there's no config, set the handler_root to the server root
    if(config_root.size() == 0)
    {
        handler_root = server_root;
    }
    else
    {
        //check if config_root is an absolute path
        if(config_root[0] == '/')
        {
            handler_root = config_root;
        }
        else
        {
            handler_root = server_root + config_root;
        }
    }

    std::string loc_key = "location";
    location = config.get_key_value(loc_key);

    if(location == "")
    {
        BOOST_LOG_TRIVIAL(info) << "WARNING: NEW_MEME HANDLER WITH EMPTY/UNCONFIGURED PATH";
    }
}

handler *meme_handler::create(const NginxConfig &config, const std::string &root_path)
{
    return new meme_handler(config, root_path);
}

std::unique_ptr<reply> meme_handler::HandleRequest(const request& request)
{
    short code = 200;
    std::string mime_type = "text/html";
    std::unordered_map<std::string, std::string> headers;
    std::string content = "";

    std::string request_path_without_location = "";
    //should always start with location
    std::size_t location_double_check = request.path.find(location);
    bool passed_double_check = true;


    //should never happen, if we see this that's an uh oh
    if(location_double_check == std::string::npos || location_double_check != 0)
    {
        BOOST_LOG_TRIVIAL(info) << "WARNING: new_meme_handler location does not match beginning of path";
        passed_double_check = false;
    }
    else
    {
        request_path_without_location = request.path.substr(location.size());
    }
    
    std::string path = handler_root;
    BOOST_LOG_TRIVIAL(info) << request.path;

    if (request.path.compare("/meme/new") == 0)
    {
        path = path + "/new.html";
    }
    // always reply with new.html

    boost::filesystem::path path_object(path);

    // std::string extension = "";
    // if (path_object.has_extension())
    // {
    //     extension = path_object.extension().string();
    // }

    std::ifstream static_file(path.c_str());

    if (passed_double_check && boost::filesystem::is_regular_file(path_object) && static_file.good())
    {
        headers["Content-Type"] = mime_type;
        // https://stackoverflow.com/questions/2602013/read-whole-ascii-file-into-c-stdstring
        std::stringstream buffer;
        buffer << static_file.rdbuf();

        content = buffer.str();
    }
    else 
    {
        code = 404;
        mime_type = "text/plain";
        content = "404 Error: Page not found";
    }

    headers["Content-Type"] = mime_type;
    headers["Content-Length"] = std::to_string(content.size());

    return std::unique_ptr<reply>(new reply(request.http_version, code, mime_type, content, headers));
}