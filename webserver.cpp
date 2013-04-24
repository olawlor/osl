/**
 Crappy, but tiny and fully flexible web *server* interface.
 
 Orion Sky Lawlor, olawlor@acm.org, 2007/09/28 (Public Domain)
*/
#include <stdio.h> /* for snprintf */
#include /*osl/*/"webserver.h"

using namespace osl;

osl::http_server::http_server(unsigned int port_,int timeoutSeconds)
	:port(port_)
{
	s=skt_server(&port);
}

http_served_client osl::http_server::serve(void) const
{
	skt_ip_t ip; unsigned int port;
	SOCKET client=skt_accept(s,&ip,&port);
	return http_served_client(client,ip,port);
}

osl::http_served_client::http_served_client(SOCKET socket,skt_ip_t ip_,unsigned int port_)
	:s(socket), ip(ip_), port(port_), error(0)
{
	/* Pull down the first HTTP request line.*/
	std::string req=skt_recv_line(s);
	if (std::string(req,0,4)!="GET ") {error="Malformed HTTP header (only GET supported for now)"; return;}
	std::string path_ver(req,4); /* clip off "GET " */
	int ver_start=path_ver.find(" HTTP/"); /* find " HTTP/1.x" marker */
	path=std::string(path_ver,0,ver_start); /* extract path in between */
	
	/* Pull down the rest of the HTTP request headers. */
	std::string l;
	while (0!=(l=skt_recv_line(s)).size()) 
	{   /* ^ a zero-length line indicates the end of the HTTP headers */
		int firstColon=l.find_first_of(":");
		std::string keyword=l.substr(0,firstColon);
		std::string value=l.substr(firstColon+2,std::string::npos);
		header[keyword]=value;
	}
}

/* Send ONLY an HTTP header indicating these many bytes are coming. */
void osl::http_served_client::send_header(std::string mime_type,
	int total_data_length,int status)
{
	enum {header_len=1000};
	char header[header_len];
	if (mime_type.size()>=header_len-200) {error="Ridiculous mime_type length"; return;}
	sprintf(header,
		"HTTP/1.1 %d %s\r\n"
		"Content-Length: %d\r\n"
		"Connection: close\r\n"
		"Content-Type: %s\r\n"
		"\r\n", /* blank line indicates end of HTTP header */
		status,status==200?"OK":"error",
		total_data_length,
		mime_type.c_str()
		);
	send_raw(header,strlen(header));
}

/* Send these raw data bytes, which eventually must total total_data_length */
void osl::http_served_client::send_raw(const char *data,int nData)
{
	skt_sendN(s,data,nData);
}
