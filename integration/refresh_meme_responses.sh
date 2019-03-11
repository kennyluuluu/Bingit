#!/bin/bash

# start webserver
../build/bin/server ../configs/8080_config > /dev/null & 
id=$!

sleep 1

#warm up the server
curl -sS localhost:8080/static/index.html > /dev/null

#check new meme page
echo -e "Refreshing new Meme\n"
echo -e "GET /meme/new HTTP/1.1\r\n\r\n" | nc localhost 8080 -w1  > new_meme_response

echo -e "Creating Memes\n"
#create memes
echo -e "GET /meme/create HTTP/1.1\r\n\r\nimage=a&top=b&bottom=space+test" | nc localhost 8080 -w1 > /dev/null 
echo -e "GET /meme/create HTTP/1.1\r\n\r\nimage=a&top=b&bottom=space+test" | nc localhost 8080 -w1  > /dev/null 
echo -e "GET /meme/create HTTP/1.1\r\n\r\nimage=b&top=different&bottom=%20space" | nc localhost 8080 -w1 > /dev/null

echo -e "Refreshing list Memes\n"
#list memes
echo -e "GET /meme/list HTTP/1.1\r\n\r\n" | nc localhost 8080 -w1 > list_meme_response

echo -e "Refreshing search Memes\n"
#search memes
echo -e "GET /meme/list?query=differ HTTP/1.1\r\n\r\n" | nc localhost 8080 -w1 > search_meme_response

echo -e "Refreshing view Meme\n"
#view meme
echo -e "GET /meme/view?id=2 HTTP/1.1\r\n\r\n" | nc localhost 8080 -w1 > view_meme_response

echo -e "Editing Meme\n"
#edit meme
echo -e "GET /meme/create HTTP/1.1\r\n\r\nimage=test.jpg&top=new+text&bottom=wowzer+dood&update=1" | nc localhost 8080 -w1 > /dev/null

echo -e "Refreshing view edited Meme\n"
#view edited meme
echo -e "GET /meme/view?id=1 HTTP/1.1\r\n\r\n" | nc localhost 8080 -w1 > edit_meme_response

echo -e "Deleting Meme\n"
#delete a meme
echo -e "GET /meme/delete?id=2 HTTP/1.1\r\n\r\n" | nc localhost 8080 -w1 > /dev/null

echo -e "Refreshing view remaining Memes\n"
#view remaining memes
echo -e "GET /meme/list HTTP/1.1\r\n\r\n" | nc localhost 8080 -w1 > list_after_delete_meme_response

kill -s SIGINT $id