#include "clienthttp.h"
#include "networkcurl.h"
#include <memory> 
#include <thread>


HttpClient::HttpClient()
{

}

HttpClient::~HttpClient()
{
    
}

void HttpClient::rest()
{
    std::thread task_td(&HttpClient::run,this);

    smphSignalMainToThread.release();
 
    // wait until the worker thread is done doing the work
    // by attempting to decrement the semaphore's count
    smphSignalThreadToMain.acquire();
    
    task_td.join();
}

void HttpClient::runwt(){
    std::string output;
    int res = restcurl(this->target,this->timeout, this->http_req_data, output,&this->headers,this->is_post_or_get);
    if(res== 0){
        isok = true;
    }else{
        isok = false;
    }
    res_type = res;
    response = output;
}

void HttpClient::run()
{
    smphSignalMainToThread.acquire();
    std::string output;
    int res = restcurl(this->target,this->timeout, this->http_req_data, output,&this->headers,this->is_post_or_get);
    if(res== 0){
        isok = true;
    }else{
        isok = false;
    }
    response = output;
    // signal the main proc back
    smphSignalThreadToMain.release();
}