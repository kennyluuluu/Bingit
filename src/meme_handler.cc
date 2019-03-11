#include "meme_handler.h"
#include <stdio.h>
#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>
#include <boost/bind.hpp>
#include <boost/algorithm/string/replace.hpp>
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
        if(config_root[0] == '/' || config_root.compare(".") == 0)
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

    // if the request path is not found at the beginning
    if(request.path.find(location) != 0)
    {
        BOOST_LOG_TRIVIAL(info) << "WARNING: meme_handler location does not match beginning of path";
        headers["Content-Type"] = mime_type;                                                                                                                                                                               headers["Content-Length"] = std::to_string(content.size());
        return std::unique_ptr<reply>(new reply(request.http_version, code, mime_type, content, headers));
    }
    
    std::string path = handler_root;
    BOOST_LOG_TRIVIAL(info) << "Request Path: " << request.path;

    if (request.path.compare("/meme/new") == 0)
    {
        prepare_new_request(path, code, mime_type, content);
    }
    else if (request.path.find("/meme/edit?update=") == 0)
    {
        // https://stackoverflow.com/questions/28163723/c-how-to-get-substring-after-a-character
        std::string id_param = request.path.substr(request.path.find("?update=") + 8);
        prepare_edit_request(path, code, mime_type, content, id_param);
    }
    else if (request.path.compare("/meme/create") == 0)
    {
        prepare_create_request(request.body, code, mime_type, content);
    }
    else if (request.path.find(location + view_path + "?id=") == 0)
    {
        prepare_view_request(request.path, code, mime_type, content);
    }
    else if (request.path.find("/meme/list") == 0)
    {
        prepare_list_request(request.path, code, mime_type, content);
    }
    else if (request.path.find("/meme/delete?id=") == 0)
    {
        // https://stackoverflow.com/questions/28163723/c-how-to-get-substring-after-a-character
        std::string id_param = request.path.substr(request.path.find("?id=") + 4);
        prepare_delete_request(code, mime_type, content, id_param);
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
 
void meme_handler::prepare_edit_request(std::string &path,
                                        short &code,
                                        std::string &mime_type,
                                        std::string &content,
                                        const std::string &meme_id)
{
    id_lock.lock();
    std::string SQL = "SELECT TEMPLATE, TOP, BOTTOM from MEMES where ID='" + meme_id + "';";
    char* err = NULL;
    meme_row result;
    result.data_found = false;
    sqlite3_exec(db_, SQL.c_str(), view_callback, &result, &err );
    id_lock.unlock();

    if (err!=NULL || !result.data_found)
    {
        if (err!=NULL)
        {
            BOOST_LOG_TRIVIAL(info) << err;
            sqlite3_free(err);
        }
        code = 404;
        mime_type = "text/html";
        content = "<header>Meme " + meme_id + " not found :(</header>";
        return;
    }

    path = path + "/edit.html";
    boost::filesystem::path path_object(path);
    std::ifstream static_file(path.c_str());

    if (boost::filesystem::is_regular_file(path_object) && static_file.good())
    {
        code = 200;
        mime_type = "text/html";
        std::stringstream buffer;
        buffer << static_file.rdbuf();
        content = buffer.str();
        
        //https://stackoverflow.com/questions/4643512/replace-substring-with-another-substring-c
        boost::replace_all(content, "$meme.top", result.top);
        boost::replace_all(content, "$meme.bot", result.bottom);
        boost::replace_all(content, "$meme.id", meme_id);
    }
}

void meme_handler::prepare_create_request(const std::string body, short &code, std::string &mime_type, std::string &content)
{
    // process form input about new meme
    BOOST_LOG_TRIVIAL(info) << "Received Create Request with inputs: " << body;

    // referenced https://stackoverflow.com/questions/9899440/parsing-html-form-data
    char pic_file[sizeof(body)] = "";
    char top_text[sizeof(body)] = "";
    char bot_text[sizeof(body)] = "";
    char edit_id[sizeof(body)] = "";

    // No edit paramters found
    if (body.find("&update=") == std::string::npos)
    {
        sscanf(body.c_str(), "image=%[^&]&top=%[^&]&bottom=%[^&]", pic_file, top_text, bot_text);
    }
    // Edit Parameter Found
    else
    {
        sscanf(body.c_str(), "image=%[^&]&top=%[^&]&bottom=%[^&]&update=%[^&]", pic_file, top_text, bot_text, edit_id);
    }
    
    for(int i=0; i < strlen(pic_file); i++)
    {
        if(pic_file[i] == '+')
            pic_file[i] = ' ';
    }
    for(int i=0; i < strlen(top_text); i++)
    {
        if(top_text[i] == '+')
            top_text[i] = ' ';
    }
    for(int i=0; i < strlen(bot_text); i++)
    {
        if(bot_text[i] == '+')
            bot_text[i] = ' ';
    }

    //decode strings
    CURL* curl = curl_easy_init();
    char* escaped_pic_file = curl_easy_unescape(curl, pic_file, 0, NULL);
    char* escaped_top_text = curl_easy_unescape(curl, top_text, 0, NULL);
    char* escaped_bot_text = curl_easy_unescape(curl, bot_text, 0, NULL);

    // convert to string
    std::string pic_file_s(escaped_pic_file);
    std::string top_text_s(escaped_top_text);
    std::string bot_text_s(escaped_bot_text);
    std::string edit_id_s(edit_id);

    curl_free(escaped_pic_file);
    curl_free(escaped_top_text);
    curl_free(escaped_bot_text);

    curl_easy_cleanup(curl);

    // empty fields are bad
    std::string default_option = "Choose...";
    if (default_option.compare(pic_file_s) == 0 || pic_file_s.empty() || top_text_s.empty() || bot_text_s.empty())
    {
        code = 400;
        mime_type = "text/plain";
        content = "400 Error: Bad Request";
        return;
    }

    char* err = NULL;
    // No Edit Parameters Found
    if (edit_id_s.empty())
    {
        id_lock.lock();
        meme_handler::meme_count++;
        unique_id = std::to_string(meme_handler::meme_count);
        content = "<header>Meme " + unique_id + " Generated!</header><a href=\"/meme/view?id=" + unique_id + "\">Click here to see your meme</a>";
        content += "<br><a href=\"/meme/new\">Click here to create another meme</a>";
        content += "<br><a href=\"/meme/list\">Click here to look at all the memes</a>";
        
        std::string SQL = "INSERT INTO MEMES VALUES (" + unique_id + ", '"+pic_file_s+"', '" + top_text_s + "', '" + bot_text_s + "');";
        sqlite3_exec(db_, SQL.c_str(), NULL, NULL, &err );
        id_lock.unlock();
    }
    // Edit Parameter Detected
    else
    {
        unique_id = edit_id_s;
        content = "<header>Meme " + unique_id + " Updated!</header><a href=\"/meme/view?id=" + unique_id + "\">Click here to see your meme</a>";
        content += "<br><a href=\"/meme/new\">Click here to create another meme</a>";
        content += "<br><a href=\"/meme/list\">Click here to look at all the memes</a>";

        std::string SQL = "UPDATE MEMES \
                           SET TEMPLATE = '" + pic_file_s + "', TOP = '" + top_text_s + "', BOTTOM = '" + bot_text_s + "'\
                           WHERE ID = " + unique_id + ";";
        id_lock.lock();
        sqlite3_exec(db_, SQL.c_str(), NULL, NULL, &err );
        id_lock.unlock();
    }

    if (err!=NULL)
    {
        BOOST_LOG_TRIVIAL(info) << err;
        sqlite3_free(err); 
        code = 404;
        mime_type = "text/html";
        content = "<header>Couldn't create meme :( Try again later</header>";
        return;
    }
    code = 200;
    mime_type = "text/html";
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
            body{position: relative; background-color: #283747;} \
            .container{position: relative; width: 100%;          \
                display:flex;justify-content:center;align-items:center; \
                flex-direction: row; position: relative; \
                color: white; font: 2em bold Impact, sans-serif; \
                text-align:center; font-weight: bold; \
            } \
            #top{top:3%; left: 50%; transform: translate(-50%); position:absolute;} \
            #bottom{bottom:3%; left: 50%; transform: translate(-50%); position:absolute} \
            a{left: 50%; transform: translate(-50%); position:absolute; font: 2em bold Impact, sans-serif;} \
        </style> \
        <body> \
            <div class=\"container\"> \
                <img src=\"/static/" + result.temp + "\"> \
                <div id=\"top\">" + result.top + "</div> \
                <div id=\"bottom\">" + result.bottom + "</div> \
            </div> \
            <br><a href=\"/meme/edit?update=" + meme_id_s + "\">Edit This Meme</a> \
            <br><a href=\"/meme/delete?id=" + meme_id_s + "\" onclick=\"return confirm(\'Are you sure?\')\">Delete This Meme</a> \
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

void meme_handler::prepare_list_request(const std::string path, short &code, std::string &mime_type, std::string &content)
{
    char search_query[sizeof(path)] = "";

    std::string regex = location + "/list" + "?query=%[^&]";

    sscanf(path.c_str(), regex.c_str(), search_query);

    for(int i=0; i < strlen(search_query); i++)
    {
        if(search_query[i] == '+')
            search_query[i] = ' ';
    }

    //decode strings
    CURL* curl = curl_easy_init();
    char* escaped_search_query = curl_easy_unescape(curl, search_query, 0, NULL);
    // convert to string
    std::string search_query_s(escaped_search_query);

    curl_free(escaped_search_query);

    curl_easy_cleanup(curl);

    id_lock.lock();
    std::string SQL = "SELECT TEMPLATE, TOP, BOTTOM, ID FROM MEMES";

    if(search_query_s.compare("") == 0)
    {
        SQL += ";";
    }
    else
    {
        SQL += " WHERE TOP LIKE '%" + search_query_s + "%' OR BOTTOM LIKE '%" + search_query_s + "%';";
    }

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
    std::string output_html = "<h1>List of All Memes</h1> \
        <form method=\"get\"> \
            <input type=\"text\" name=\"query\" > \
            <input type=\"submit\" value=\"Search\"> \
        </form> \
    ";
    output_html += "<ul>";

    for (int i = 0; i < result.size(); i++)
    {
        output_html += "<li><a href=/meme/view?id=" + result[i]->id + ">" + result[i]->id + " top: " + result[i]->top + " bot: " + result[i]->bottom + "</a></li>";
    }

    output_html += "</ul>";

    if(result.size() == 0)
    {
        output_html += "<div>No memes with '" + search_query_s + "' in the top or bottom text</div>";
    }

    if(search_query_s.compare("") != 0)
    {
        output_html += "<a href=\"/meme/list\">Clear search</a>";
    }
    content = output_html;
}

void meme_handler::prepare_delete_request(short &code, std::string &mime_type, std::string &content, const std::string &meme_id)
{
    BOOST_LOG_TRIVIAL(info) << "Delete handler with id " << meme_id << "\n";
    id_lock.lock();
std::string SQL = "DELETE FROM MEMES WHERE ID='" + meme_id + "';";
    char* err = NULL;
    sqlite3_exec(db_, SQL.c_str(), NULL, NULL, &err );
    id_lock.unlock();


    if (err!=NULL)
    {
        BOOST_LOG_TRIVIAL(info) << err;
        sqlite3_free(err); 
        code = 404;
        mime_type = "text/html";
        content = "<header>No Meme with id = " + meme_id + "found :(</header>";
        return;
    }

    BOOST_LOG_TRIVIAL(info) << "Successfully deleted meme with id = " << meme_id << "\n";
    code = 200;
    mime_type = "text/html";
    content = "<div>Meme with id " + meme_id + " has been deleted </div>";
    content += "<br><a href=\"/meme/new\">Click here to create another meme</a>";
    content += "<br><a href=\"/meme/list\">Click here to look at all the memes</a>";



}
