#include "meme_handler.h"
#include <stdio.h>
#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>
#include <boost/bind.hpp>
#include <mutex>
#include <sqlite3.h>
#include <vector>

std::mutex id_lock;

struct meme_row {
    std::string temp;
    std::string top;
    std::string bottom;
    std::string id;
    bool data_found;
};

int meme_handler::meme_count = 0;

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
        BOOST_LOG_TRIVIAL(info) << "WARNING: NEW_MEME HANDLER WITH EMPTY/UNCONFIGURED LOCATION";
    }


    std::string view_key = "view";
    view_path = config.get_key_value(view_key);

    if(view_path == "")
    {
        BOOST_LOG_TRIVIAL(info) << "WARNING: NEW_MEME HANDLER WITH EMPTY/UNCONFIGURED VIEW PATH";
    }
}

handler *meme_handler::create(const NginxConfig &config, const std::string &root_path)
{
    return new meme_handler(config, root_path);
}

void meme_handler::set_db_ptr(sqlite3 *db)
{
    db_ = db;
}

std::unique_ptr<reply> meme_handler::HandleRequest(const request& request)
{
    short code = 404;
    std::string mime_type = "text/plain";
    std::unordered_map<std::string, std::string> headers;
    std::string content = "404 Error: Page not found";

    //std::string request_path_without_location = "";

    // if the request path is not found at the beginning
    if(request.path.find(location) != 0)
    {
        BOOST_LOG_TRIVIAL(info) << "WARNING: meme_handler location does not match beginning of path";
        headers["Content-Type"] = mime_type;                                                                                                                                                                               headers["Content-Length"] = std::to_string(content.size());
        return std::unique_ptr<reply>(new reply(request.http_version, code, mime_type, content, headers));
    }
    
    std::string path = handler_root;
    BOOST_LOG_TRIVIAL(info) << request.path;

    if (request.path.compare("/meme/new") == 0 || request.path.compare("/meme/new") == 0)
    {
        prepare_new_request(path, code, mime_type, content);
    }
    else if (request.path.compare("/meme/create") == 0 || request.path.compare("/meme/create/") == 0)
    {
        prepare_create_request(request.body, code, mime_type, content);
    }
    else if (request.path.find(location + view_path + "?id=") == 0)
    {
        prepare_view_request(request.path, code, mime_type, content);
    }
    else if (request.path.compare("/meme/list") == 0 || request.path.compare("/meme/list/") == 0)
    {
        prepare_list_request(request.body, code, mime_type, content);
    }
    

    headers["Content-Type"] = mime_type;
    headers["Content-Length"] = std::to_string(content.size());

    return std::unique_ptr<reply>(new reply(request.http_version, code, mime_type, content, headers));
}

void meme_handler::prepare_new_request(std::string &path, 
                                short &code, 
                                std::string &mime_type, 
                                std::string &content)
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

    if (boost::filesystem::is_regular_file(path_object) && static_file.good())
    {
        code = 200;
        mime_type = "text/html";
        // https://stackoverflow.com/questions/2602013/read-whole-ascii-file-into-c-stdstring
        std::stringstream buffer;
        buffer << static_file.rdbuf();
        content = buffer.str();
    }
}

static int create_callback(void* arg, int numColumns, char** result, char** columnNames)
{
    return 0;
}


void meme_handler::prepare_create_request(const std::string body, short &code, std::string &mime_type, std::string &content)
{
    // process form input about new meme
    BOOST_LOG_TRIVIAL(info) << body;

    // referenced https://stackoverflow.com/questions/9899440/parsing-html-form-data
    char pic_file[sizeof(body)] = "";
    char top_text[sizeof(body)] = "";
    char bot_text[sizeof(body)] = "";

    sscanf(body.c_str(), "image=%[^&]&top=%[^&]&bottom=%[^&]", pic_file, top_text, bot_text);

    // convert to string
    std::string pic_file_s(pic_file);
    std::string top_text_s(top_text);
    std::string bot_text_s(bot_text);

    // empty fields are bad
    std::string default_option = "Choose...";
    if (default_option.compare(pic_file_s) == 0 || pic_file_s.empty() || top_text_s.empty() || bot_text_s.empty())
    {
        code = 400;
        mime_type = "text/plain";
        content = "400 Error: Bad Request";
    }
    else 
    {
        id_lock.lock();
        meme_handler::meme_count++;
        unique_id = meme_handler::meme_count;
        content = "<header>Meme " + std::to_string(meme_handler::meme_count) + " Generated!</header><a href=\"/meme/view?id=" + std::to_string(meme_handler::meme_count) + "\">Click here to see your meme</a>";
        
        std::string SQL = "INSERT INTO MEMES VALUES (" + std::to_string(meme_handler::meme_count) + ", '"+pic_file_s+"', '" + top_text_s + "', '" + bot_text_s + "');";
        char* err = NULL;
        sqlite3_exec(db_, SQL.c_str(), create_callback, NULL, &err );
        if (err!=NULL)
        {
            BOOST_LOG_TRIVIAL(info) << err;
            sqlite3_free(err); 
            code = 404;
            mime_type = "text/html";
            content = "<header>Couldn't create meme :( Try again later</header>";
            return;
        }
        id_lock.unlock();
        code = 200;
        mime_type = "text/html";
    }
}

static int view_callback(void* arg, int numColumns, char** result, char** columnNames)
{
    meme_row* mrp = (meme_row*) arg;
    BOOST_LOG_TRIVIAL(info) << "Found matching row in db";

    bool foundData = false;

    for (int i=0; i< numColumns; i++){
        BOOST_LOG_TRIVIAL(info) << columnNames[i] << " " << result[i];
        std::string column = columnNames[i];
        std::string val = result[i];
        if(column.compare("TEMPLATE") == 0)
        {
            mrp->temp = val;
            foundData = true;
        }
        else if(column.compare("TOP") == 0)
        {
            mrp->top = val;
            foundData = true;
        }
        else if(column.compare("BOTTOM") == 0)
        {
            mrp->bottom = val;
            foundData = true;
        }
        else
        {
            BOOST_LOG_TRIVIAL(info) << "Unexpected column/value " << column << " " << val;
        }
    }
    mrp->data_found = foundData; 
    return 0;
}

void meme_handler::prepare_view_request(const std::string path, short &code, std::string &mime_type, std::string &content)
{
    // referenced https://stackoverflow.com/questions/9899440/parsing-html-form-data
    char meme_id[sizeof(path)] = "";

    std::string regex = location + view_path + "?id=%[^&]";

    sscanf(path.c_str(), regex.c_str(), meme_id);

    // convert to string
    std::string meme_id_s(meme_id);

    if (meme_id_s.empty())
    {
        code = 400;
        mime_type = "text/plain";
        content = "400 Error: Bad Request";
    }
    else 
    {
        id_lock.lock();
        std::string SQL = "SELECT TEMPLATE, TOP, BOTTOM from MEMES where ID='" + meme_id_s + "';";
        char* err = NULL;
        meme_row result;
        result.data_found = false;
        sqlite3_exec(db_, SQL.c_str(), view_callback, &result, &err );
        id_lock.unlock();
        if (err!=NULL)
        {
            BOOST_LOG_TRIVIAL(info) << err;
            sqlite3_free(err); 
            code = 404;
            mime_type = "text/html";
            content = "<header>Meme " + meme_id_s + " not found :(</header>";
            return;
        }
        else if (!result.data_found)
        {
            BOOST_LOG_TRIVIAL(info) << "No error, but db read didn't reach callback";
            code = 404;
            mime_type = "text/html";
            content = "<header>Meme " + meme_id_s + " not found :(</header>";
            return;
        }
        code = 200;
        mime_type = "text/html";

        
        content =
        "<style> \
            body{position: relative; text-align: center; color: white; background-color: #283747;} \
            div{color: white; font: 2em bold Impact, sans-serif; \
                position: absolute; text-align: center;} \
            #top{top:5%; left: 50%; transform: translate(-50%, -50%);} \
            #bottom{top:95%; left: 50%; transform: translate(-50%, -50%); } \
        </style> \
        <body> \
            <img src=\"/static/" + result.temp + "\"> \
            <div id=\"top\">" + result.top + "</div> \
            <div id=\"bottom\">" + result.bottom + "</div> \
        </body>";
    }
}

static int list_callback(void* arg, int numColumns, char** result, char** columnNames)
{
    meme_row* mrp = new meme_row; 
    std::vector<meme_row*>* meme_rows = (std::vector<meme_row*>*)arg; // cast arg to a pointer to a vector of pointers to meme_rows

    bool data_found = false;


    for (int i=0; i< numColumns; i++){
        std::string column = columnNames[i];
        std::string val = result[i];
        if (column.compare("ID") == 0)
        {
            mrp->id = val;
            data_found = true;
        }
        else if(column.compare("TEMPLATE") == 0)
        {
            mrp->temp = val;
            data_found = true;
        }
        else if(column.compare("TOP") == 0)
        {
            mrp->top = val;
            data_found = true;
        }
        else if(column.compare("BOTTOM") == 0)
        {
            mrp->bottom = val;
            data_found = true;
        }
        else
        {
            BOOST_LOG_TRIVIAL(info) << "Unexpected column/value " << column << " " << val;
        }
    }
    meme_rows->push_back(mrp);
    mrp->data_found = data_found;
    return 0;
}

void meme_handler::prepare_list_request(const std::string body, short &code, std::string &mime_type, std::string &content)
{
    id_lock.lock();
    std::string SQL = "SELECT TEMPLATE, TOP, BOTTOM, ID FROM MEMES;";
    char* err = NULL;
    std::vector<meme_row*> result;
    sqlite3_exec(db_, SQL.c_str(), list_callback, &result, &err );
    id_lock.unlock();
    if (err!=NULL)
        {
            BOOST_LOG_TRIVIAL(info) << err;
            sqlite3_free(err); 
            code = 404;
            mime_type = "text/html";
            content = "<header>No memes found :(</header>";
            return;
        }
    BOOST_LOG_TRIVIAL(info) << "Found " << result.size() <<  " rows in db";
    code = 200;
    mime_type = "text/html";
    std::string output_html = "<h1>List of All Memes</h1>";
    output_html += "<ul>";
    for (int i = 0; i < result.size(); i++)
    {
        output_html += "<li><a href=/meme/view?id=" + result[i]->id + ">" + "/meme/view?id=" + result[i]->id + "</a></li>";
    }
    output_html += "</ul>";
    content = output_html;
}