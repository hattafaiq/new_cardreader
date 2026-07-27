#ifndef CLIENT_HTTP
#define CLIENT_HTTP
#include <string>
#include <unordered_map> 
#include <semaphore>

class HttpClient{
public:
    explicit HttpClient();
    ~HttpClient();
    void rest();
    
    bool isok = false;
    int timeout = 5;
    std::string target; //url
    std::string response;
    std::unordered_map<std::string,std::string> headers;
    std::string http_req_data; //http req data
    void runwt();
    bool is_post_or_get = false; //get
    int res_type = 0;
private:   
    void run();
    std::binary_semaphore
    smphSignalMainToThread{0},
    smphSignalThreadToMain{0};
};

#endif