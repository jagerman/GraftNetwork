#include <boost/bind.hpp>
#include <string>
#include <iostream>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/tokenizer.hpp>
#include "misc_log_ex.h"
#include "DAPI_RPC_Server.h"
#include "FSN_Servant.h"
#include "PosProxy.h"
#include "WalletProxy.h"
#include "AuthSample.h"
using namespace std;



int main(int argc, char** argv) {
	mlog_configure("", true);
	mlog_set_log_level(5);

	string conf_file("conf.ini");
	if(argc>1) conf_file = argv[1];
	LOG_PRINT_L5("conf: "<<conf_file);

	// load config
	boost::property_tree::ptree config;
	boost::property_tree::ini_parser::read_ini(conf_file, config);


	// TODO: Init all monero staff here


	// Init super node objects
	const boost::property_tree::ptree& dapi_conf = config.get_child("dapi");
	supernode::DAPI_RPC_Server dapi_server;
	dapi_server.Set( dapi_conf.get<string>("ip"), dapi_conf.get<string>("port"), dapi_conf.get<int>("threads") );

	const boost::property_tree::ptree& wc = config.get_child("wallets");
	supernode::FSN_Servant servant;
	servant.Set( wc.get<string>("stake_file"), wc.get<string>("stake_passwd"), wc.get<string>("miner_file"), wc.get<string>("miner_passwd") );

	vector<supernode::BaseRTAProcessor*> objs;
	objs.push_back( new supernode::WalletProxy() );
	objs.push_back( new supernode::PosProxy() );
	objs.push_back( new supernode::AuthSample() );

	for(unsigned i=0;i<objs.size();i++) {
		objs[i]->Set(&servant, &dapi_server);
		objs[i]->Start();
	}

	dapi_server.Start();

	for(unsigned i=0;i<objs.size();i++) {
		objs[i]->Stop();
		delete objs[i];
	}


	return 0;
}



