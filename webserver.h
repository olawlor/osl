/**
 Crappy, but tiny and fully flexible web *server* interface.
 
 Orion Sky Lawlor, olawlor@acm.org, 2007/09/28 (Public Domain)
*/
#ifndef __OSL_WEBSERVER_H
#define __OSL_WEBSERVER_H

#include "webservice.h"


/* This macro is handy for quoting long strings of HTML.
  The one restriction is you can't include commas, or it whines about "takes just one argument".
  The preprocessor also removes newlines, so we add one manually. */
#define OSL_QUOTE_HTML(htmlcode) #htmlcode "\n"

namespace osl {

/**
 Represents an HTTP connection from one client to our server.
*/
class OSL_DLL http_served_client {
public:
	http_served_client(SOCKET socket,skt_ip_t ip,unsigned int port);
	~http_served_client() { close();}
	void close(void) { if (s) skt_close(s); s=0; }

/* Client and request info access: */
	/** Return the human-readable connection error code, or 0 if none. */
	const char *get_error(void) const {return error;}
	
	/** Return the IP address the client connected from. */
	skt_ip_t get_ip(void) const {return ip;}
	/** Return the TCP port the client connected from. */
	unsigned int get_port(void) const {return port;}
	
	/** Return the path the client has requested, like "/foo/bar.cgi?baz=3"
	*/
	const std::string get_path(void) const {return path;}

	/** Look up the value of the client's HTTP header line with this keyword, or empty string if none. */
	std::string get_header(const std::string &keyword) {return header[keyword];}
	
/* Send data back to the client */
	/* Send a complete HTTP header and this data back to the client */
	inline void send(std::string mime_type,const char *data,int nData) 
		{send_header(mime_type,nData); send_raw(data,nData);}
	inline void send(std::string mime_type,const std::string &str) 
		{send(mime_type,&str[0],str.size());}
	
	/* Send back a complete not-found HTTP error page */
	inline void send_error(std::string mime_type,const std::string &str,int error_type=404) 
	{
		send_header(mime_type,str.size(),error_type);
		send_raw(&str[0],str.size());
	}
	
	
	/* Send ONLY an HTTP header indicating:
		- The data to come has this mime_type ("text/html","image/jpeg", ...)
		- These many bytes are coming for the total response. 
		- The HTTP response status is this.  The default is 200, OK.  404 would work too.
	*/
	void send_header(std::string mime_type,int total_data_length,int status=200);
	/* Send these raw data bytes, which eventually must total total_data_length */
	void send_raw(const char *data,int nData);
	
	
private:
	SOCKET s;
	skt_ip_t ip; unsigned int port;
	std::string path; /* GET ... HTTP/1.x */
	std::map<std::string,std::string> header; /**< http header names and values */
	const char *error;
};

/**
 Runs an HTTP server.
*/
class OSL_DLL http_server {
public:
	/**
	  Create an HTTP server listening on the given port.
	  Note that to listen on port 80, your code must run as root.
	*/
	http_server(unsigned int port_=8080,int timeoutSeconds=60);
	unsigned int get_port(void) {return port;} /* return port we're listening on */
	~http_server() { close();}
	void close(void) { if (s) skt_close(s); s=0; }
	
	/* Return true if a client has connected to our server.
	   Waits up to msec milliseconds for the client; waits forever if msec==0. 
	   This function is normally called from the main thread.
	*/
	bool ready(int msec=1) const {return skt_select1(s,msec);}
	
	/* Service the waiting client, and return an object describing his request. 
	  For best performance, this function should be called from a worker thread.
	*/
	http_served_client serve(void) const;
	
private:
	SERVER_SOCKET s;
	unsigned int port;
};


};

#endif
