#ifndef TPID_CPP 
#define TPID_CPP 

#include "TPiD.hpp" 
#include <tobicore/TCBlock.hpp>

TPiD::TPiD(void) {
}

TPiD::~TPiD(void) {
}
#include <iostream>
int TPiD::ConfAsClient(const std::string &ip, const std::string& port) {
	int status = TPInterface::ConfAsClient(ip, port);
	if(status == TPInterface::Successful)
		this->_com->Async(true);
	return status;
}
		
int TPiD::ConfAsServer(const std::string &ip, const std::string& port) {
	return TPInterface::ErrorNotSupported;
}

int TPiD::Set(IDSerializer* serializer, int bidx, int* abidx) {
	if(TPInterface::_com == NULL)
		return TPInterface::ErrorSocket;
	if(TPInterface::_com->IsConnected() == false)
		return TPInterface::ErrorSocket;
	
	if(abidx != NULL)
		*abidx = TCBlock::BlockIdxUnset;
	
	serializer->message->SetBlockIdx(bidx);
	serializer->message->absolute.Tic();
	serializer->Serialize(&this->_cache);
	if(TPInterface::_com->Send(TPInterface::_cache) <= 0)
		return TPInterface::ErrorSocket;

	int count = 0;
	while(true) {
		TPInterface::_cache.clear();
		if(TPInterface::_com->Recv(&this->_cache) <= 0) {
			if(count++ == 100) 
				return TPInterface::ErrorTimeout;
			TCSleep(1.000f);
			continue;
		}


		this->_stream.Append(TPInterface::_cache);
		TPInterface::_cache.clear();
		if(this->_stream.Extract(&this->_cache, "<tcstatus", "/>") == true) {
			break;
		} else {
		}
	}
	
	int comp, status, fidx;
	if(this->_lang.IsStatus(this->_cache, &comp, &status, &fidx) == true) {
		if(abidx != NULL) 
			*abidx = fidx;
		return TPInterface::Successful;
	}

	return TPInterface::ErrorProtocol;
}

int TPiD::Get(IDSerializer* serializer) {
	if(TPInterface::_com == NULL)
		return TPInterface::ErrorSocket;
	if(TPInterface::_com->IsConnected() == false)
		return TPInterface::ErrorSocket;
		
	TPInterface::_cache.clear();
	if(TPInterface::_com->Recv(&this->_cache) > 0)
		this->_stream.Append(TPInterface::_cache);
	
	if(this->_stream.Extract(&this->_cache, "<tobiid", "/>") == false) 
		return TPInterface::Unsuccessful;

	serializer->Deserialize(&this->_cache);
	return TPInterface::Successful;
}

#endif
