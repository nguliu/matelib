// Author Fangping Liu
// 2019-09-07

#include "../Buffer.h"
#include "HttpContext.h"

using namespace lfp;


//利用有限状态机解析HTTP请求
bool HttpContext::parseRequest(Buffer* buf)
{
    bool ok = true;
    bool hasMore = true;
    while (hasMore)
    {
        if (expectRequestLine())	// 处于解析请求行状态
        {
            const char* crlf = buf->findCRLF(); // 找到请求行结尾
            if (crlf)		//有请求行，是一个HTTP请求
            {
                ok = processRequestLine(buf->peek(), crlf);	// 解析请求行
                if (ok)
                {
                    buf->retrieveUntil(crlf + 2);	//将请求行从buf中去除，包括\r\n
                    receiveRequestLine();			//将HttpContext状态改为kExpectHeaders
                }
                else
                {
                    hasMore = false;
                }
            }
            else		//没有请求行，则不是一个HTTP请求
            {
                hasMore = false;
            }
        }
        else if (expectHeaders())		// 解析header
        {
            const char* crlf = buf->findCRLF();
            if (crlf)
            {
                const char* colon = std::find(buf->peek(), crlf, ':');	//冒号所在位置
                if (colon != crlf)
                {
                    request_.addHeader(buf->peek(), colon, crlf);
                }
                else	//头部解析完成
                {
                    receiveHeaders();	// HttpContext将状态改为kExpectBody
                }
                buf->retrieveUntil(crlf + 2);	//将header从buf中去除，包括\r\n
            }
            else
            {
                hasMore = false;
            }
        }
        else if (expectBody())	 //当前还不支持请求中带body
        {
			receiveBody();
			hasMore = false;
        }
    }
    return ok;
}

//解析[begin, end)是否为正确的请求行
bool HttpContext::processRequestLine(const char* begin, const char* end)
{
    bool succeed = false;

    // 解析请求方法并保存到request_
    const char* start = begin;
    const char* space = std::find(start, end, ' ');
    if (space != end && request_.setMethod(start, space))
    {
        //解析请求路径
        start = space + 1;
        space = std::find(start, end, ' ');
        if (space != end)
        {
            request_.setPath(start, space);  // 解析请求路径并保存到request

            //解析HTTP版本      
            start = space + 1;
            succeed = (end - start == 8) && std::equal(start, end - 1, "HTTP/1.");
            if (succeed)
            {
                if (*(end - 1) == '1') {
                    request_.setVersion(HttpRequest::kHttp11);  // HTTP/1.1
                }
                else if (*(end - 1) == '0') {
                    request_.setVersion(HttpRequest::kHttp10);  // HTTP/1.0
                }
                else
                {
                    succeed = false;
                }
            }
        }
    }
    return succeed;
}
