/**
  Uses osl/porthread and osl/webserver to make a classic 
  one-thread-per-client web server.
  
  Dr. Orion Sky Lawlor, lawlor@alaska.edu, 2012-01-03 (Public Domain)
*/
#ifndef __OSL_WEBSERVER_THREAD_H
#define __OSL_WEBSERVER_THREAD_H 1

#include <iostream>
#include "osl_dll.h"
#include "porthread.h"
#include "webserver.h"
#include <vector>

namespace osl {

/*
 Responds to web clients' requests.
*/
class OSL_DLL http_responder {
public:
	/* Return true if we just handled this client, otherwise false. 
	   CAUTION: MULTITHREADED CALLS!
	*/
	virtual bool respond(osl::http_served_client &client) =0;
};


/* Logs the requests of clients, as they go by. 
   Format is similar to Apache's access_log format,
   with IP address, date, referer, and user-agent. */
class OSL_DLL html_logger : public http_responder {
	std::ostream &out;
public:
	html_logger(std::ostream &out_) :out(out_) {}
	bool respond(osl::http_served_client &client);
};



/*
 Creates a web server, running in its own separate thread.
 A typical usage is 
    osl::http_threaded_server *server=new osl::http_threaded_server(1234);
	server->add_responder(new my_web_responder);
	server->start();
*/
class OSL_DLL http_threaded_server : public http_server {
	porthread_t server_thread;
	std::vector<http_responder *> responders;
public:
	http_threaded_server(unsigned int port=8080);
	
	/* Add a responder into the HTTP namespace.
	   Responders are tried one at a time, in order.
	*/
	void add_responder(http_responder *responder);
	
	/* If no responders are found, send back this error page.  
	   CAUTION: MULTITHREADED CALLS!
	*/
	virtual void no_responder(osl::http_served_client &client);
	
	/* Start the thread to respond to HTTP requests.
	   Once this is running, the class can't be deleted. */
	void start(void); 
	
	/* Service the currently connected client 
	   CAUTION: MULTITHREADED CALLS!*/
	void service_client(void);
};


}; /* end namespace osl */

#endif
