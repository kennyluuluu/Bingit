// An nginx config file parser.
//
// See:
//   http://wiki.nginx.org/Configuration
//   http://blog.martinfjordvald.com/2010/07/nginx-primer/
//
// How Nginx does it:
//   http://lxr.nginx.org/source/src/core/ngx_conf_file.c

#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <stack>
#include <string>
#include <vector>
#include <unordered_map>
#include <utility>
#include "config_parser.h"

std::string NginxConfig::ToString(int depth) const
{
  std::string serialized_config;
  for (const auto &statement : statements_)
  {
    serialized_config.append(statement->ToString(depth));
  }
  return serialized_config;
}

int NginxConfig::get_port() const
{
    for (const auto &statement : statements_)
    {
         if(statement->tokens_.size() == 2 &&
	    statement->tokens_[0].compare("port") == 0)
	 {
	     std::string port_num = statement->tokens_[1];

	     //checking if provided port is a number
	     //obtained and modified from:
	     //https://stackoverflow.com/questions/4654636/how-to-determine-if-a-string-is-a-number-with-c
	     std::string::const_iterator it = port_num.begin();
	     while (it != port_num.end() && std::isdigit(*it)) ++it;
	     if (port_num.empty() || it != port_num.end())
	         return -1;
	     
	     return atoi(port_num.c_str());
	}
    }
    return -1;
}

std::string NginxConfig::get_key_value(std::string key) const
{

  for (const auto &statement : statements_)
  {
    //check if there's 2 tokens, no child block,
    //and the 1st token matches the key
    if(statement->tokens_.size() == 2 &&
        statement->tokens_[0].compare(key) == 0 &&
        statement->child_block_.get() == nullptr)
    {
        return statement->tokens_[1];
    }
  }
  return "";
}

void NginxConfig::get_handler_paths(std::map<std::string, std::pair<std::string, NginxConfig>>* map) const
{
  for (const auto &statement : statements_)
  {
    //if there's 2 tokens in the statement,
    //and the 1st token is "handler"
    //and it has a child block
    if (statement->tokens_.size() == 2 &&
        statement->tokens_[0].compare("handler") == 0 &&
        statement->child_block_.get() != nullptr)
    {
      std::string handler_type = statement->tokens_[1];
      std::string path = statement->child_block_.get()->get_key_value("location");
      
      //if there's a location key-value pair in the child block
      if(path.compare("") != 0)
      {
        //if the map doesn't have this handler yet, initialize it
        if (map->find(handler_type) == map->end())
        {
          (*map)[path] = std::make_pair(handler_type, *(statement->child_block_.get()));
        }
        else
        {
          //TODO: add logging to say there's a duplicate config?
        }
      }
    }
    else if (statement->child_block_.get() != nullptr)
    {
      statement->child_block_.get()->get_handler_paths(map);
    }
  }
}

std::string NginxConfigStatement::ToString(int depth)
{
  std::string serialized_statement;
  for (int i = 0; i < depth; ++i)
  {
    serialized_statement.append("  ");
  }
  for (unsigned int i = 0; i < tokens_.size(); ++i)
  {
    if (i != 0)
    {
      serialized_statement.append(" ");
    }
    serialized_statement.append(tokens_[i]);
  }
  if (child_block_.get() != nullptr)
  {
    serialized_statement.append(" {\n");
    serialized_statement.append(child_block_->ToString(depth + 1));
    for (int i = 0; i < depth; ++i)
    {
      serialized_statement.append("  ");
    }
    serialized_statement.append("}");
  }
  else
  {
    serialized_statement.append(";");
  }
  serialized_statement.append("\n");
  return serialized_statement;
}

const char *NginxConfigParser::TokenTypeAsString(TokenType type)
{
  switch (type)
  {
  case TOKEN_TYPE_START:
    return "TOKEN_TYPE_START";
  case TOKEN_TYPE_NORMAL:
    return "TOKEN_TYPE_NORMAL";
  case TOKEN_TYPE_START_BLOCK:
    return "TOKEN_TYPE_START_BLOCK";
  case TOKEN_TYPE_END_BLOCK:
    return "TOKEN_TYPE_END_BLOCK";
  case TOKEN_TYPE_COMMENT:
    return "TOKEN_TYPE_COMMENT";
  case TOKEN_TYPE_STATEMENT_END:
    return "TOKEN_TYPE_STATEMENT_END";
  case TOKEN_TYPE_EOF:
    return "TOKEN_TYPE_EOF";
  case TOKEN_TYPE_ERROR:
    return "TOKEN_TYPE_ERROR";
  default:
    return "Unknown token type";
  }
}

NginxConfigParser::TokenType NginxConfigParser::ParseToken(std::istream *input,
                                                           std::string *value)
{
  TokenParserState state = TOKEN_STATE_INITIAL_WHITESPACE;
  while (input->good())
  {
    const char c = input->get();
    if (!input->good())
    {
      break;
    }
    switch (state)
    {
    case TOKEN_STATE_INITIAL_WHITESPACE:
      switch (c)
      {
      case '{':
        *value = c;
        return TOKEN_TYPE_START_BLOCK;
      case '}':
        *value = c;
        return TOKEN_TYPE_END_BLOCK;
      case '#':
        *value = c;
        state = TOKEN_STATE_TOKEN_TYPE_COMMENT;
        continue;
      case '"':
        *value = c;
        state = TOKEN_STATE_DOUBLE_QUOTE;
        continue;
      case '\'':
        *value = c;
        state = TOKEN_STATE_SINGLE_QUOTE;
        continue;
      case ';':
        *value = c;
        return TOKEN_TYPE_STATEMENT_END;
      case ' ':
      case '\t':
      case '\n':
      case '\r':
        continue;
      default:
        *value += c;
        state = TOKEN_STATE_TOKEN_TYPE_NORMAL;
        continue;
      }
    case TOKEN_STATE_SINGLE_QUOTE:
      // TODO: the end of a quoted token should be followed by whitespace.
      // TODO: Maybe also define a QUOTED_STRING token type.
      // TODO: Allow for backslash-escaping within strings.
      *value += c;
      if (c == '\'')
      {
        return TOKEN_TYPE_NORMAL;
      }
      continue;
    case TOKEN_STATE_DOUBLE_QUOTE:
      *value += c;
      if (c == '"')
      {
        return TOKEN_TYPE_NORMAL;
      }
      continue;
    case TOKEN_STATE_TOKEN_TYPE_COMMENT:
      if (c == '\n' || c == '\r')
      {
        return TOKEN_TYPE_COMMENT;
      }
      *value += c;
      continue;
    case TOKEN_STATE_TOKEN_TYPE_NORMAL:
      if (c == ' ' || c == '\t' || c == '\n' || c == '\t' ||
          c == ';' || c == '{' || c == '}')
      {
        input->unget();
        return TOKEN_TYPE_NORMAL;
      }
      *value += c;
      continue;
    }
  }

  // If we get here, we reached the end of the file.
  if (state == TOKEN_STATE_SINGLE_QUOTE ||
      state == TOKEN_STATE_DOUBLE_QUOTE)
  {
    return TOKEN_TYPE_ERROR;
  }

  return TOKEN_TYPE_EOF;
}

bool NginxConfigParser::Parse(std::istream *config_file, NginxConfig *config)
{
  std::stack<NginxConfig *> config_stack;
  config_stack.push(config);
  TokenType last_token_type = TOKEN_TYPE_START;
  TokenType token_type;
  int num_start_block = 0;
  int num_end_block = 0;
  while (true)
  {
    std::string token;
    token_type = ParseToken(config_file, &token);
    //Shows parsing each token, uncomment to see where a config is failing
    //printf ("%s: %s\n", TokenTypeAsString(token_type), token.c_str());
    if (token_type == TOKEN_TYPE_ERROR)
    {
      break;
    }

    if (token_type == TOKEN_TYPE_COMMENT)
    {
      // Skip comments.
      continue;
    }

    if (token_type == TOKEN_TYPE_START)
    {
      // Error.
      break;
    }
    else if (token_type == TOKEN_TYPE_NORMAL)
    {
      if (last_token_type == TOKEN_TYPE_START ||
          last_token_type == TOKEN_TYPE_STATEMENT_END ||
          last_token_type == TOKEN_TYPE_START_BLOCK ||
          last_token_type == TOKEN_TYPE_END_BLOCK ||
          last_token_type == TOKEN_TYPE_NORMAL)
      {
        if (last_token_type != TOKEN_TYPE_NORMAL)
        {
          config_stack.top()->statements_.emplace_back(
              new NginxConfigStatement);
        }
        config_stack.top()->statements_.back().get()->tokens_.push_back(
            token);
      }
      else
      {
        // Error.
        break;
      }
    }
    else if (token_type == TOKEN_TYPE_STATEMENT_END)
    {
      if (last_token_type != TOKEN_TYPE_NORMAL)
      {
        // Error.
        break;
      }
    }
    else if (token_type == TOKEN_TYPE_START_BLOCK)
    {
      if (last_token_type != TOKEN_TYPE_NORMAL)
      {
        // Error.
        break;
      }
      num_start_block += 1;
      NginxConfig *const new_config = new NginxConfig;
      config_stack.top()->statements_.back().get()->child_block_.reset(
          new_config);
      config_stack.push(new_config);
    }
    else if (token_type == TOKEN_TYPE_END_BLOCK)
    {
      if (last_token_type != TOKEN_TYPE_STATEMENT_END &&
          last_token_type != TOKEN_TYPE_END_BLOCK)
      {
        // Error.
        break;
      }
      if (num_start_block <= num_end_block)
      {
        // Error. can't have more } than open blocks
        break;
      }
      num_end_block += 1;
      config_stack.pop();
    }
    else if (token_type == TOKEN_TYPE_EOF)
    {
      if (last_token_type != TOKEN_TYPE_STATEMENT_END &&
          last_token_type != TOKEN_TYPE_END_BLOCK &&
          last_token_type != TOKEN_TYPE_START)
      {
        // Error.
        break;
      }
      if (num_start_block != num_end_block)
      {
        // Error. unmatched blocks present
        break;
      }
      return true;
    }
    else
    {
      // Error. Unknown token.
      break;
    }
    last_token_type = token_type;
  }

  printf("Bad transition from %s to %s\n",
         TokenTypeAsString(last_token_type),
         TokenTypeAsString(token_type));
  return false;
}

bool NginxConfigParser::Parse(const char *file_name, NginxConfig *config)
{
  std::ifstream config_file;
  config_file.open(file_name);
  if (!config_file.good())
  {
    printf("Failed to open config file: %s\n", file_name);
    return false;
  }

  const bool return_value =
      Parse(dynamic_cast<std::istream *>(&config_file), config);
  config_file.close();
  return return_value;
}
