// dvntest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <iostream>

#include <string>
#include <stdlib.h>
#include <string.h>
#include <map>
#include <new>
#include <json/json.h>


std::string urlDecode(std::string str)
{
    std::string temp;
    int i;
    char tmp[5], tmpchar;
    strncpy(tmp,"0x",5);
    int size = str.size();
    for (i=0; i<size; i++) {
        if (str[i]=='%') {
            if (i+2<size) {
                tmp[2]=str[i+1];
                tmp[3] = str[i+2];
                tmp[4] = '\0';
                tmpchar = (char)strtol(tmp,NULL,0);
                temp+=tmpchar;
                i += 2;
                continue;
            } else {
                break;
            }
        } else if (str[i]=='+') {
            temp+=' ';
        } else {
            temp+=str[i];
        }
    }
    return temp;
}


void initializePost(std::map <std::string, std::string> &Post)
{
    std::string tmpkey, tmpvalue;
    std::string *tmpstr = &tmpkey;
    int content_length;
    char *ibuffer;
    char *buffer = NULL;
    char *strlength = getenv("CONTENT_LENGTH");
    if (strlength == NULL) {
        Post.clear();
        return;
    }
    content_length = atoi(strlength);
    if (content_length == 0) {
        Post.clear();
        return;
    }

    try {
        buffer = new char[content_length*sizeof(char)];
    } catch (std::bad_alloc xa) {
        Post.clear();
        return;
    }
    if(fread(buffer, sizeof(char), content_length, stdin) != (unsigned int)content_length) {
        Post.clear();
        return;
    }
    *(buffer+content_length) = '\0';
    ibuffer = buffer;
    while (*ibuffer != '\0') {
        if (*ibuffer=='&') {
            if (tmpkey!="") {
                Post[urlDecode(tmpkey)] = urlDecode(tmpvalue);
            }
            tmpkey.clear();
            tmpvalue.clear();
            tmpstr = &tmpkey;
        } else if (*ibuffer=='=') {
            tmpstr = &tmpvalue;
        } else {
            (*tmpstr) += (*ibuffer);
        }
        ibuffer++;
    }
    //enter the last pair to the map
    if (tmpkey!="") {
        Post[urlDecode(tmpkey)] = urlDecode(tmpvalue);
        tmpkey.clear();
        tmpvalue.clear();
    }
}


std::map<std::string, double> configMap;


extern std::string TrafficOptimTest();
extern std::string TrafficOptimConjTest();
extern std::string  FailureFindTest();

extern void initVars1();
extern void initVars();

int main(int argc, char **argv)
{
   int err = 0;
   if(argc > 1)
   {
      if (!strncmp(argv[1], "Optim",5))
      {
         TrafficOptimTest();
      }
      else if (!strncmp(argv[1], "Conj",4))
      {
         TrafficOptimConjTest();
      }
      else if (!strncmp(argv[1], "Find",4))
      {
         FailureFindTest();
      }
      getchar();
   }
   else
   {
      std::map <std::string, std::string> Post;
      initializePost(Post) ;
      std::string sReq;
      
      if(Post.count("req"))
      {
         sReq = Post["req"];
         err =1;
      }

      if (Post.count("data"))
      {
         Json::Value v;
         Json::Reader r;
         if (!r.parse("data", v))
         {
            err = 1;
         }
         else
         {
            for (Json::Value::iterator it = v.begin();
                 it != v.end(); ++it)
            {
               configMap[it.name()] == it->asDouble();
            }
         }
      }

      std::string sRes;
      if (!err)
      {
         if (sReq == "Optim")
         {
            initVars();
            sRes = TrafficOptimTest();
         }
         else if (sReq== "Conj")
         {
            initVars1();
            sRes = TrafficOptimConjTest();
         }
         else if (sReq== "Find")
         {
            initVars();
            sRes= FailureFindTest();
         }
         std::cout << "Content-Type:text/html:" << std::endl << std::endl;
         std::cout << sRes.c_str() << std::endl;
        
      }
      else
      {
         std::cout << "Content-Type:text/html:" << std::endl << std::endl;
         std::cout << "{\"res\":\"failed\"}" << std::endl;

      }
      
   }
   return 0;
}

