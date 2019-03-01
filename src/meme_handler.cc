#include "meme_handler.h"
#include <stdio.h>
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
    short code = 404;
    std::string mime_type = "text/plain";
    std::unordered_map<std::string, std::string> headers;
    std::string content = "404 Error: Page not found";

    std::string request_path_without_location = "";
    //should always start with location
    std::size_t location_double_check = request.path.find(location);
    bool passed_double_check = true;


    //should never happen, if we see this that's an uh oh
    if(location_double_check == std::string::npos || location_double_check != 0)
    {
        BOOST_LOG_TRIVIAL(info) << "WARNING: meme_handler location does not match beginning of path";
        passed_double_check = false;
    }
    else
    {
        request_path_without_location = request.path.substr(location.size());
    }
    
    std::string path = handler_root;
    BOOST_LOG_TRIVIAL(info) << request.path;

    if (request.path.compare("/meme/new") == 0 || request.path.compare("/meme/new/") == 0)
    {
        handle_new(path, code, mime_type, content, passed_double_check);
    }
    else if (request.path.compare("/meme/create") == 0 || request.path.compare("/meme/create/") == 0)
    {
        // process form input about new meme
        BOOST_LOG_TRIVIAL(info) << request.body;
        // referenced https://stackoverflow.com/questions/9899440/parsing-html-form-data
        char pic_file[sizeof(request.body)] = "";
        char top_text[sizeof(request.body)] = "";
        char bot_text[sizeof(request.body)] = "";

        sscanf(request.body.c_str(), "image=%[^&]&top=%[^&]&bottom=%s", pic_file, top_text, bot_text);

        // maybe strings is easier than char[]
        std::string pic_file_s(pic_file);
        std::string top_text_s(top_text);
        std::string bot_text_s(bot_text);
        // BOOST_LOG_TRIVIAL(info) << pic_file_s;
        // BOOST_LOG_TRIVIAL(info) << top_text_s;
        // BOOST_LOG_TRIVIAL(info) << bot_text_s;

        // empty fields are bad
        std::string default_option = "Select+a+template...";
        BOOST_LOG_TRIVIAL(info) << default_option;
        if (default_option.compare(pic_file_s) == 0 || pic_file_s.empty() || top_text_s.empty() || bot_text_s.empty())
        {
            code = 400;
            mime_type = "text/plain";
            content = "400 Error: Bad Request";
        }
        else 
        {
            // TODO: check for more invalid inputs
            // TODO: escape user-generate input
            // TODO: save creation request to file if safe THIS MUST BE THREAD SAFE
            code = 200;
            mime_type = "text/plain";
            content = "Made a meme!";
        }
    }
    // TODO: handle /meme/list
    

    headers["Content-Type"] = mime_type;
    headers["Content-Length"] = std::to_string(content.size());

    return std::unique_ptr<reply>(new reply(request.http_version, code, mime_type, content, headers));
}

void meme_handler::handle_new(std::string &path, 
                                short &code, 
                                std::string &mime_type, 
                                std::string &content,
                                const bool &passed_double_check)
{
    // show form to create new meme
    path = path + "/new.html";
    boost::filesystem::path path_object(path);

    // std::string extension = "";
    // if (path_object.has_extension())
    // {
    //     extension = path_object.extension().string();
    // }

    std::ifstream static_file(path.c_str());

    if (passed_double_check && boost::filesystem::is_regular_file(path_object) && static_file.good())
    {
        code = 200;
        mime_type = "text/html";
        // https://stackoverflow.com/questions/2602013/read-whole-ascii-file-into-c-stdstring
        std::stringstream buffer;
        buffer << static_file.rdbuf();

        content = buffer.str();
    }
}