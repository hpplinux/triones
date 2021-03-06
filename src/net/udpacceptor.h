/******************************************************
 *   FileName: UDPAcceptor.h
 *     Author: triones  2014-9-18
 *Description: (1) UDPAcceptor 来管理有其派生出来的UDPComponent而不是由Transport直接管理；
 *Description: 所有的UDPComponent共用一个socket，所有的读操作UDPAcceptor完成
 *Description: 写操作由UDPComponent完成，同时写操作属于同步接口。
 *Description: （2） socket关闭的策略：立马标志socket的状态，清理网络层资源。业务层的packet包，可以通过packet包通知
 *Description:     而不是直接回调。
 *Description: （3）引用计数的策略，其他对象聚合A引用计数加1，解除聚合关系引用计数减1
 *******************************************************/

#ifndef UDPACCEPTOR_H_
#define UDPACCEPTOR_H_

#include "../comm/tqueue.h"

//UDP包的最大长度即为64KB, 每次直接发送64KB是有问题的，故每次发送63KB，经测试没有问题。
#define TRIONES_UDP_MAX_PACK  (64 * 1024 - 1024)
#define TRIONES_UDP_RECV_SIZE (64 * 1024)

namespace triones
{
class UDPAcceptor: public IOComponent
{
public:
	UDPAcceptor(Transport *owner, Socket *socket, triones::TransProtocol *streamer,
	        IServerAdapter *serverAdapter);

	virtual ~UDPAcceptor()
	{
		if (_socket)
		{
			_socket->close();
			delete _socket;
			_socket = NULL;
		}
	}

	bool init();

	//处理写事件
	bool handle_read_event();

	//不处理写事件
	bool handle_write_event()
	{
		return true;
	}

	//IOComponent 网络资源清理
	virtual void close();

	//检查超时，检查_online并回调超时处理函数
	bool check_timeout(uint64_t now);

	//最大包为64KB，也就一个IP包的长度。不处理多个UDP包组成一个应用包的情况，
	//处理一个UDP包中包含多个应用包的情况。
	bool read_data();

	//不操作写操作, 对UDPAcceptor不会调用到writeData
	bool write_data();

	//根据sockid获取对应的UDPComponent, 如果没有找到新建一个
	UDPComponent *get(uint64_t sockid, struct sockaddr_in *addr);

private:

	struct sockaddr_in _sock_addr;

	//这里的_isServer指的accpect出来的socket，而不是listen socket
	bool _isServer;

	// 发送队列锁
	triones::Mutex _output_mutex;

	// UDP接收长度
	char _read_buff[TRIONES_UDP_RECV_SIZE];

	// 接收队列，解包时使用
	PacketQueue _inputQueue;

	// packet header已经取过
	bool _gotHeader;

};

} /* namespace triones */

#endif /* UDPACCEPTOR_H_ */
