#include "peer_point.h"
#include "socket_exception.h"
#include "socket_manager.h"

#include <iostream>

using namespace std;
extern SocketManager* Manager;

void PeerOnRead(struct bufferevent * buf_ev, void * arg)
{
	clsPeerPoint* p = static_cast<clsPeerPoint *>(arg);
	p->OnRead(buf_ev, arg);
}

void PeerOnError(struct bufferevent * buf_ev, short error_no, void * arg)
{
	clsPeerPoint* p = static_cast<clsPeerPoint *>(arg);
	p->OnError(buf_ev, error_no, arg);
}

clsPeerPoint::clsPeerPoint(int Vfd, int VfdType)
{
	_Vfd = Vfd;
	_VfdType = VfdType;
	_BuffEv = bufferevent_new(_Vfd, PeerOnRead, NULL, PeerOnError, this);
	bufferevent_enable(_BuffEv, EV_READ);
	_Status = 1;
}

clsPeerPoint::~clsPeerPoint()
{
}

void clsPeerPoint::OnRead(struct bufferevent* buf_ev, void* arg)
{
	if(_Status == 0) throw SocketError(_Vfd, 1, "Try To Read Cleared vfd");

	int ReadLen = 0;
	char buf[RECV_BLOCK_SIZE];	

	while(1) {
		ReadLen = bufferevent_read(buf_ev, buf, sizeof(buf));
		if (ReadLen == 0) return;
		
		//调用上层的收包函数
		Manager->PeerVfdOnRead(_Vfd, buf, ReadLen);
	}

}

void clsPeerPoint::OnError(struct bufferevent* buf_ev, short error_no, void* arg)
{
	if (_Status == 0) return;
	_Status = 0;
	Manager->PeerVfdOnClose(_Vfd);
	close(_Vfd);
	Manager->ShowInfo();
}

void clsPeerPoint::OnConnect(int Vfd)
{
	Manager->PeerVfdOnConnect(Vfd);
	Manager->ShowInfo();
}

bool clsPeerPoint::WriteData(char* Buf, int BufLen)
{
	if (_Status == 0) return 0;
	return !(bufferevent_write(_BuffEv, (void *)Buf, BufLen));	
}
