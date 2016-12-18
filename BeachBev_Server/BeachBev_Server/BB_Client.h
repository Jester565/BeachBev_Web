#pragma once
#include "stdafx.h"
#include <Client.h>

class BB_Client : public Client
{
public:
		BB_Client(boost::shared_ptr <TCPConnection> tcpConnection, Server* server, IDType id);

		~BB_Client();
};

