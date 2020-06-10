#include "tcpconnection.h"

#include "event.h"
using namespace tudou;



TcpConnection::TcpConnection(EventLoop::Ptr loop, int fd)
: _loop(loop)
, _sockfd(fd)
, _timeout(10)
, _input()
, _output()
{
    //_read_cb = _write_cb = _error_cb = _close_cb = nullptr;
    _name = reinterpret_cast<uint64_t>(this);
    _channel = make_shared<Channel>(_loop.get(), _sockfd); //初始化一个channel
}


//connect封装
TcpConnection::Ptr TcpConnection::connection(EventLoop::Ptr loop, const string &host, uint16_t port, int timeout, const char *localIp)
{
    Ptr _conn(new TcpConnection(loop, 0));
    _address.set(host, port);
    _timeout = timeout;
    auto nowTime = getMsec(); //当前时间
    auto ret = SocketTool::connectTcp(host.c_str(), port); //socket - bind - connect
    switch(ret.first.getStatus())
    {
    case Error::OK:
        {
            _state = HANDSHAKING;
            set(_loop, ret.second);
            break;
        }
    default:
        break;
    }
    
    if(ret.second < 0)
    {
        return nullptr;
    }
    
    handleTimeout(shared_from_this(), timeout);
    return _conn; //返回一个Tcpconnection
}

//设置回调函数
void TcpConnection::setReadCb(const MessageCallback &cb)
{
    _read_cb = cb;
}
void TcpConnection::setWriteCb(const MessageCallback &cb)
{
    _write_cb = cb;
}
void TcpConnection::setCloseCb(const CloseCallback &cb)
{
    _close_cb = cb;
}
void TcpConnection::setErrorCb(const ErrorCallback &cb)
{
    _error_cb = cb;
}

//发送消息，首先把消息存到缓冲区

int TcpConnection::send(const string &msg) {  send(msg.c_str(), msg.size()); }

/*
    1. 缓冲区没有数据尝试直接::write
    2. 若有数据就append到缓冲区
    3. 没全部发完把剩下的append到缓冲区
*/
int TcpConnection::send(const char *msg, size_t len)
{
    if(_channel)
    {
        if(!_output.empty()) //输出缓冲区有数据还没有发送
        {
            _output.append(msg, len);
            return 0;
        }
        //尝试直接发送
    
        auto ret = SocketTool::sendPart(_channel->getFd(), msg, len);

        auto &_err = ret.first;
        if(ret.first.getStatus() != Error::OK)
        {
            return -1; //发送不成功
        }
        if(ret.second == len)
        {
            return len; //全部发送成功
        }
   
        _output.append(msg+ret.second, len-ret.second); //剩下的加入缓冲区
     
        return 1;
    }
    return -1;
}

EventLoop::Ptr TcpConnection::getEventLoop() { return _loop; }
Channel::Ptr TcpConnection::getChannel() { return _channel; }
TcpConnection::TcpState TcpConnection::getState() { return _state; }
    
Buffer* TcpConnection::getInput() { return &_input; }
Buffer* TcpConnection::getOutput() { return &_output; }
SocketAddress TcpConnection::getAddress() { return _address; }

void TcpConnection::setState(TcpState s) { _state = s; }

//主动断开链接
void TcpConnection::shutdown()
{
    //缓冲区还有数据没有发完
    if(!_output.empty())
    {
        //把缓冲区所有信息发出去
        handleWrite(shared_from_this());
    }
    //handleClose(shared_from_this());
    _channel->close();
}

//处理读事件,如果使用lt模式，有数据就会一直触发
void TcpConnection::handleRead(const Ptr &ptr)
{	
    //unique_lock<mutex> lck(_mutex);
    if(_state == TcpConnection::HANDSHAKING && !handleHandshake(ptr)) //判断是否链接
    {
        return ;
    }
    auto sockfd = _channel->getFd();
    if(_state == CONNECTED && _channel->getFd() > 0)
    {

        auto ret = SocketTool::readPart(_channel->getFd(), _input);	
        if(ret.second == -1) //断开连接
        {
            std::cout << "this is read" << std::endl;
            //_channel->close();
            //shutdown();
            return ;
        }
   
        //更新timer
        if(_timer.lock())
        {
            _timer.lock()->reset();
        }
        
        if(_read_cb && ret.second)
        {	
            _read_cb(ptr);
        }
        ptr->_channel->enableRead(false);
        ptr->_channel->enableWrite(true);
        return ;
    
    }
    
}
void TcpConnection::handleWrite(const Ptr &ptr)
{
    //unique_lock<mutex> lck(_mutex);
    
    if(ptr->getState() == TcpConnection::HANDSHAKING)
    {
        handleHandshake(ptr);
    }

    if(_state == CONNECTED)
    {
        //没有数据可以发送

         if(_output.empty()) return;
                 std::cout << "this is write__" << _output.size() << std::endl;
        if(_output.empty() && _write_cb)
        {
            _write_cb(ptr);
            return ;
        }
        auto ret = SocketTool::sendPart(_channel->getFd(), _output);
         //std::cout << "this is write__" << _output.size() << std::endl;
        if(ret.first.getStatus() == Error::ERROR) //发送失败
        {
            std::cout << "this is write__" << std::endl;
            shutdown();
            return ;
        }
        if(_write_cb)
        {
            _write_cb(ptr);
        }

        ptr->_channel->enableWrite(false);
        ptr->_channel->enableRead(true);
    }
    

}
void TcpConnection::handleError(const Ptr &ptr)
{
    unique_lock<mutex> lck(_mutex);
    Error _error;
    // _channel->enableRead(false);
    // _channel->enableWrite(false);
    _state = FAILED;
    if(_error_cb)
    {
        _error_cb(ptr, _error);
    }
}
int TcpConnection::handleHandshake(const Ptr &ptr) //握手,connect函数在第二次握手返回,缓冲区是可写的
{ 									//判断是否链接成功，成功之后触发pollout
    struct pollfd pf;
    memset(&pf, 0, sizeof(pf));
    pf.fd = _channel->getFd();
    pf.events = POLLOUT | POLLERR;
    int ret = poll(&pf, 1, 0);
    if(ret == 1)
    {
        if(pf.events & POLLOUT)
        {
            //监听到读事件连接成功
            _state = CONNECTED;
            return 0;
        }
    }
    
    return -1;
}
//处理关闭,被动断开链接
void TcpConnection::handleClose(const Ptr &ptr)
{
    _state = CLOSED;  //设置状态为close
    //std::cout << &(*_channel) <<"  --123 " << endl;
    //把channel从eventloop中删除
    _loop->delChannel(_channel.get());
    if(_close_cb)
    {
        _close_cb(ptr);
    }
}

//超时还没连上就断开
void TcpConnection::handleTimeout(const Ptr &ptr, int timeout)
{
    if(timeout > 0)
    {
        auto conn = shared_from_this();
        auto _time = make_shared<Timer>(timeout, [conn]()
                            {
                                if(conn->getState() == HANDSHAKING)
                                {
                                    conn->setState(TcpConnection::CLOSED);
                                    auto ch = conn->getChannel();
                                    ch->close(); //关闭channel
                                    ch = nullptr; 
                                }
                                return false;
                            }
                    , ptr);	
        TimerManager::getInstance().addTimer(_time);
    }
}

void TcpConnection::set(EventLoop::Ptr loop, int fd)
{
    //_sockfd = fd;   
    Ptr _point = shared_from_this(); //获取一个指针,设置channel相关回调函数
    _point->getChannel()->setReadCb( [=](){ _point->handleRead(_point); } );
    _point->getChannel()->setWriteCb( [=](){ _point->handleWrite(_point); } );
    //////////////////////
    _point->getChannel()->setErrorCb( [=](){ _point->handleError(_point); } );
    _point->getChannel()->setCloseCb( [=](){ _point->handleClose(_point); } );
}
weak_ptr<Timer>& TcpConnection::getTimer() { return _timer; }
void TcpConnection::setTimer(Timer::Ptr &timer)
{
    _timer = timer; //每个connection都有一个定时器
}





TcpConnection::Ptr & TcpClient::start(EventLoop::Ptr loop, const string &host, uint16_t port, int timeout)
{
    TcpConnection::Ptr client(new TcpConnection(loop, 0));
    _client = std::move(client->connection(loop, host, port, timeout, nullptr));
    return _client;
}

TcpConnection::Ptr TcpClient::_client = nullptr;




TcpServer::TcpServer(EventLoop::Ptr loop)
: _loop(loop)
, _channel(nullptr)
, _read_cb(nullptr)
, _write_cb(nullptr)
, _connection_cb(nullptr)
, _error_cb(nullptr)
{}
int TcpServer::listen(const string &host, uint16_t port)
{
    auto ret = SocketTool::listenTcp(host.c_str(), port); //pair<Error, fd>
    
    if(ret.second < 0)
    {
        return -1;
    }

    _channel.reset(new Channel(_loop.get(), ret.second)); //监听socket加入epoll
    
    _channel->enableWrite(false);
    _channel->enableRead(true);
    std::cout << "423   " <<&(*_channel) << std::endl;
    
    auto conn = shared_from_this();
    _channel->setReadCb([=](){ handleAccept(conn); });
    return 0;
}

TcpServer::Ptr TcpServer::start(EventLoop::Ptr loop, const string &host, uint16_t port)
{
    Ptr ptr(new TcpServer(loop));  //在这里要设置回调函数
    ptr->setReadCb(_read_cb);
    ptr->setWriteCb(_write_cb);	
    ptr->setConnectionCb(_connection_cb);
    ptr->setErrorCb(_error_cb);
       //   std::cout << strerror(errno) << "++++3++1+++++"<< std::endl;
    auto ret = ptr->listen(host, port);
    if(ret)
    {
        return nullptr;
    }
    
    return ptr;
}

void TcpServer::setReadCb(const MessageCallback& cb) { _read_cb = cb; }
void TcpServer::setWriteCb(const MessageCallback& cb) { _write_cb = cb; }
void TcpServer::setErrorCb(const ErrorCallback &cb) { _error_cb = cb; }
void TcpServer::setConnectionCb(const MessageCallback &cb) { _connection_cb = cb; }
Channel::Ptr TcpServer::getChannel() { return _channel; }
void TcpServer::handleAccept(shared_ptr<TcpServer> _conn) //listen readcb
{
    
    auto _channel = _conn->getChannel();
    struct sockaddr_in client;
    memset(&client, 0, sizeof(client));
    socklen_t len = sizeof(client);
    //链接客户端
    
    int connfd = ::accept(_channel->getFd(), (struct sockaddr *)&client, &len);
    if(connfd < 0)
    {
        std::cout<< strerror(errno);
        return ;
    }
    
    auto _choose_loop = EventLoopPool::getInstance().choose(); //选取一个eventloop
    //if(_choose_loop == _loop)	std::cout << "great" << std::endl;
    SocketTool::setNonBlock(connfd, true); //设置非阻塞
    SocketTool::setCloseOnExec(connfd);
    TcpConnection::Ptr con(new TcpConnection(_choose_loop, connfd)); //新建一个tcpconnection
    con->setState(TcpConnection::CONNECTED); //已链接
    con->getAddress().set(SocketTool::inetNtop(client), SocketTool::ntoh(client.sin_port)); //链接客户端的信息
   
   
    _connections[con->_name] = con; //有一个链接加入
    std::cout << " this is a new connection: " <<_connections.size()<< std::endl;
     //  cout << strerror(errno)<<"-------------------" << endl;
    auto fun = [=]()
                {
                    //加入定时器，超时断开连接
                    //还要设置connection的timer
        #if 0
                    auto _time = make_shared<Timer>(_CLOSE_TIME_, [=]()
                        {
                            con->getTimer().lock()->delTimer();
                            con->shutdown();  
                            return false;
                        }
                    , con);


                    con->setTimer(_time);

                    TimerManager::getInstance().addTimer(_time);
         #endif    
                    
                    con->setReadCb([=](const TcpConnection::Ptr &s) 
                                    {
                                        if(_read_cb)	{ _read_cb(s); }
                                    } );	

                    con->setWriteCb([&, this](const TcpConnection::Ptr &s) 
                                    {
                                        if(_write_cb)	{ _write_cb(s); }
                                    } );	
                    con->setCloseCb([&, this](const TcpConnection::Ptr &s) 
                                    {
                                        _connections.erase(s->_name);
                                        if(_close_cb)	{ _close_cb(s); }
                                    } );						
                                    
                    con->setErrorCb([&, this](const TcpConnection::Ptr &x, const Error &s)
                                    {
                                        if(_error_cb) { _error_cb(x, s); }
                                    } );
                    
                    if(_connection_cb) { _connection_cb(con); }
                       
                    con->set(_choose_loop, connfd); //新建channel并加入监听
                   
                };

    _loop->runAsync(fun, true);

}

//关闭连接
void TcpServer::shutdown(TcpConnection::Ptr con)
{
    auto it = _connections.find(con->_name);
    if(it != _connections.end())
    {
        con->shutdown(); //关闭connection
    }
}

void TcpServer::shutdown()
{
    for(auto & it : _connections)
    {
        (it.second)->shutdown();
    }
}

size_t TcpServer::size() { return _connections.size(); } //链接个数

void TcpServer::send(const string &msg)
{
    
    for(auto &it : _connections)
    {
        (it.second)->send(msg);
    }
}








TudouServer::TudouServer(EventLoop::Ptr loop, const string &host, uint16_t port, size_t threadnum, size_t eventnum)
: _main_loop(loop)
, _thread_pool(threadnum)
, _event_pool(EventLoopPool::getInstance(loop, eventnum))
, _host(host)
, _port(port)
, _server(nullptr)
{}
void TudouServer::setReadCb(const MessageCallback& cb) { _read_cb = cb; }
void TudouServer::setWriteCb(const MessageCallback& cb) {_write_cb = cb; }
void TudouServer::setErrorCb(const ErrorCallback &cb) { _error_cb = cb; }
void TudouServer::setConnectionCb(const MessageCallback &cb) { _connection_cb = cb; }

void TudouServer::start() //最后一个参数是线程池所创建的线程
{
    TcpServer _temp_server(_main_loop);
    _temp_server.setReadCb(_read_cb);
    _temp_server.setWriteCb(_write_cb);
    _temp_server.setErrorCb(_error_cb);
    _temp_server.setConnectionCb(_connection_cb);
    _server = _temp_server.start(_main_loop, _host, _port); //启动监听
    
    
}

void TudouServer::shutdown()
{
    _server->shutdown();
    _thread_pool.shutdown();
}
//发送数据到缓冲区

void TudouServer::send(const string &message)
{
    //将任务加入线程池
    
    bool flag = _thread_pool.addTask(
        [&, this, message]()
        {
            _server->send(message + "\r\n");
        }
    );
    
}
