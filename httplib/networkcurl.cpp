#include "networkcurl.h"
#include <sstream>
#include <string>
#include <curl/curl.h>

struct MemoryStruct {
    char *memory;
    size_t size;
};

static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = (char*)realloc(mem->memory, mem->size + realsize + 1);
    if(!ptr) {
        /* out of memory! */
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}


class InternalWork{
public:
    explicit InternalWork(){

    }
    ~InternalWork(){
    }

    void Setup(std::string_view url, int timeout){
        m_url = url.data();
        m_timeout = timeout;
    }

    void Request(std::string_view data,const std::unordered_map<std::string,std::string> *headers,bool is_post_or_get){
        CURLcode ret;
        CURL *hnd;
        struct curl_slist *slist1 =NULL;

        curl_global_init(CURL_GLOBAL_ALL);

        struct MemoryStruct chunk;

        chunk.memory =(char*) malloc(1);  /* will be grown as needed by the realloc above */
        chunk.size = 0; 

        if(headers){
            for(const auto &hdr: *headers){
                std::stringstream str;
                str<< hdr.first;
                str<< ": ";
                str<< hdr.second;
                slist1 = curl_slist_append(slist1, str.str().c_str());
            }
        }

        hnd = curl_easy_init();
        curl_easy_setopt(hnd, CURLOPT_BUFFERSIZE, CURL_MAX_READ_SIZE);
        curl_easy_setopt(hnd, CURLOPT_URL, this->m_url.c_str());
        curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
        curl_easy_setopt(hnd, CURLOPT_TIMEOUT_MS, this->m_timeout);
        if(is_post_or_get){ 
            curl_easy_setopt(hnd, CURLOPT_POSTFIELDS, data.data());
        }else{
            /* use a GET to fetch this */
            curl_easy_setopt(hnd, CURLOPT_HTTPGET, 1L);
        }
        //curl_easy_setopt(hnd, CURLOPT_POSTFIELDSIZE_LARGE, data.size());
        if(slist1) curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, slist1);
        curl_easy_setopt(hnd, CURLOPT_USERAGENT, "CDM FE");
        curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50L);
        curl_easy_setopt(hnd, CURLOPT_TCP_KEEPALIVE, 0);
       
        /* send all data to this function  */
        curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

        /* we pass our 'chunk' struct to the callback function */
        curl_easy_setopt(hnd, CURLOPT_WRITEDATA, (void *)&chunk);

        ret = curl_easy_perform(hnd);
        this->m_errorcode = static_cast<int>(ret);
        /*
        long response_code;
        double elapsed;
        curl_easy_getinfo(hnd, CURLINFO_RESPONSE_CODE, &response_code);
        curl_easy_getinfo(hnd, CURLINFO_TOTAL_TIME, &elapsed);
        */

        curl_easy_cleanup(hnd);
        if(slist1) curl_slist_free_all(slist1);
        slist1 = NULL;
        m_data = std::string(chunk.memory, chunk.size);
        free(chunk.memory);

        curl_global_cleanup();
        hnd = NULL;
        chunk.memory = NULL;
    }

    int m_errorcode = 0;
    std::string m_data;
private:
    std::string m_url;
    int m_timeout = 0;
};

int restcurl(std::string_view url, int timeout, std::string_view data, std::string &output,const std::unordered_map<std::string,std::string> *headers,bool is_post_or_get)
{
    InternalWork work;
    work.Setup(url, timeout);
    work.Request(data,headers,is_post_or_get);
    if(!work.m_data.empty()){
        output = work.m_data;
    }
    return work.m_errorcode;    
}
