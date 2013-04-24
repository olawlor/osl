/*
	Easy HTTP accessible program configuration library.
	
	The basic idea is you use a PUP-style interface to expose
	the tweakable variables in your program, and then just
	point a web browser at the program to view and edit them.
	
	Handy for debugging, remote access, etc.
	
	Dr. Orion Lawlor, lawlor@alaska.edu, 2012-01-31 (Public Domain)
*/
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include "webconfig.h"



/***************** Implementation of webconfig **************/


std::vector<pup_this_object *> webconfig_pup_list;

/* Add this object to be pup'd at any time by webconfig. */
void webconfig_add_pup(pup_this_object *p) {
	webconfig_pup_list.push_back(p);
}

/* Pup all web config objects registered above */
void webconfig_pup_all(pup_er_virtual &p) {
	for (unsigned int i=0;i<webconfig_pup_list.size();i++)
		webconfig_pup_list[i]->pupto(p);
}

/*************** Implementation Utility Functions ****************/
/* Convert an integer to a short std::string */
std::string itos(int i) {
	char v[100];
	snprintf(v,100,"%d",(int)i);
	return v;
}

/* Return the number of newlines in this string */
int count_newlines(const std::string &src) {
	int r=0;
	for (unsigned int i=0;i<src.size();i++) if (src[i]=='\n') r++;
	return r;
}

/**
 Escape quotes and other bad HTML inside this string.
*/
std::string escape_HTML(const std::string &str) {
	std::string ret;
	for (unsigned int i=0;i<str.size();i++) {
		char c=str[i];
		    if (c=='\"') { ret+="&quot;"; }
		else if (c=='<') { ret+="&lt;"; }
		else if (c=='>') { ret+="&gt;"; }
		else if (c=='&') { ret+="&amp;"; }
		else {
			ret+=c;
		}
	}
	return ret;
}

/**
 Un-escape URL encoded text values, coming back in an URL from a textarea.
*/
std::string unescape_URL(const std::string &str) {
	std::string ret;
	for (unsigned int i=0;i<str.size();i++) {
		char c=str[i];
		if (c=='+') { /* + stands for space */
			ret+=' ';
		}
		else if (c=='%' && i+2<str.size()) { /* Two digit hex code, e.g., %21 for exclamation point */
			char buf[10];
			buf[0]=str[i+1];
			buf[1]=str[i+2];
			buf[2]=0;
			int v=0;
			sscanf(buf,"%x",&v);
			ret+=(char)v;
			i+=2;
		}
		else {
			ret+=c;
		}
	}
	return ret;
}





/**
 Read object data from a simple flat binary file.
*/
class pup_from_binary_file : public pup_er_virtual {
public:
	typedef pup_from_binary_file this_t;
	std::istream &s;
	pup_from_binary_file(std::istream &s_) :s(s_) {}
	
	void pup(const char *shortname,float &value) {
		s.read((char *)&value,sizeof(float));
	}
	void pup(const char *shortname,int &value) {
		s.read((char *)&value,sizeof(int));
	}
	void pup(const char *shortname,std::string &value) {
		int len=value.length();
		s.read((char *)&len,sizeof(int));
		value.resize(len);
		s.read((char *)&value[0],len);
	}
};

/**
 Write object data to a simple flat binary file.
*/
class pup_to_binary_file : public pup_er_virtual {
public:
	typedef pup_to_binary_file this_t;
	std::ostream &s;
	pup_to_binary_file(std::ostream &s_) :s(s_) {}
	
	void pup(const char *shortname,float &value) {
		s.write((char *)&value,sizeof(float));
	}
	void pup(const char *shortname,int &value) {
		s.write((char *)&value,sizeof(int));
	}
	void pup(const char *shortname,std::string &value) {
		int len=value.length();
		s.write((char *)&len,sizeof(int));
		s.write((char *)&value[0],len);
	}
};



/**
 Write to only this name and value.
*/
class pup_from_name_value : public pup_er_virtual  {
public:
	typedef pup_from_name_value this_t;
	bool found; /* set to true once we find the value we want */
	pup_from_name_value(const std::string &fullname_,const std::string &value_) 
		:target(fullname_),newvalue(value_) {found=false;}
	
	/* Pup for basic value types */
	void pup(const char *shortname,float &value) {
		if (inner(shortname)) {value=atof(newvalue.c_str());}
	}
	void pup(const char *shortname,int &value) {
		if (inner(shortname)) {value=atoi(newvalue.c_str());}
	}
	void pup(const char *shortname,std::string &value) {
		if (inner(shortname)) {value=unescape_URL(newvalue);}
	}
	// Pup an enum, with a list of name/value pairs
	void pup(const char *shortname,
			unsigned int &value,const name_value_record *namevalue) 
	{
		if (inner(shortname)) {value=atoi(newvalue.c_str());}
		// pup(shortname,value); // <- why doesn't this work?
	}
	
	virtual void pup_objectbegin(const char *shortname) {
		old_addresses.push_back(address); /* store old address */
		address=address+shortname+"."; /* add full name to our sub-objects */
		
	}
	virtual void pup_objectend(const char *shortname) {
		address=*(old_addresses.end()-1);
		old_addresses.pop_back();
	}
private:
	const std::string &target; /* fullname of target object */
	const std::string &newvalue; /* HTML string where our output is stored */
	
	std::string address; /* current fully-qualified object address */
	std::vector<std::string> old_addresses; /* for tracing object names */
	
	/* Return true if this field is the one we're looking for */
	bool inner(const char *shortname) {
		std::string fullname=address+shortname;
		// std::cout<<"Comparing '"<<fullname<<"' against '"<<target<<"'\n"; // debug
		if (fullname==target) { found=true; return true; }
		else return false;
	}
};


/**
 Convert arbitrary incoming types into working HTML form fields.
 To simplify processing of the returned data, we use a separate FORM for each field
 rather than one big form.
*/
class pup_to_HTML_form : public pup_er_virtual {
public:
	typedef pup_to_HTML_form this_t;
	pup_to_HTML_form(std::string &html_,const std::string &form_name_) 
		:html(html_),form_name(form_name_),indent(0),divcount(0) {}
	
	void comment(const std::string &s) {
		html+=s;
	}
	
	// Pup a float
	void pup(const char *shortname,float &value) {
		char curvalue[100];
		snprintf(curvalue,100,"%f",(float)value);
		inner(shortname,curvalue);
	}
	
	// Pup an integer
	void pup(const char *shortname,int &value) {
		inner(shortname,itos(value));
	}
	
	// Pup a string
	void pup(const char *shortname,std::string &value) {
		if (count_newlines(value)==0) { // short string (no newlines)
			inner(shortname,escape_HTML(value).c_str());
		} else { // long string, with newlines
			std::string fullname=address+shortname;
			html+=startform()
				+shortname+":<br>\n"
				"<textarea name=\""+fullname+"\" cols=\"85\" rows=\""+itos(count_newlines(value)+2)+"\">"
				+escape_HTML(value)
				+"</textarea><br>"+endform();
		}
	}
	
	// Pup an enum, with a list of name/value pairs
	void pup(const char *shortname,
			unsigned int &value,const name_value_record *namevalue) 
	{
		std::string fullname=address+shortname;
		html+=startform()
			+shortname+": <SELECT name=\""+fullname+"\" >\n";
		
		// Loop over the enum options
		for (const name_value_record *nv=namevalue;nv->name!=0;nv++) {
			html=html+"<option value=\""+itos((int)nv->value)+"\" ";
			if (value==nv->value) html+="selected=\"selected\"";
			html=html+">"+nv->name+"</option>\n";
		}
		
		html+="</SELECT>"+endform();
	}
	
	virtual void pup_objectbegin(const char *shortname) {
		old_addresses.push_back(address); /* store old address */
		
		html+=itemdiv();
		html+=address+"<B>"+shortname+"</B> {"; /* GUI */
		address=address+shortname+"."; /* add full name to our sub-objects */
		indent++;
		html+="<DIV STYLE=\"margin-left:1em; padding-left:1em;\">\n";
		// background-color:"+getobjectcolor()+"\">\n";
		
	}
	virtual void pup_objectend(const char *shortname) {
		html+="</DIV>}<br>\n\n";
		indent--;
		address=*(old_addresses.end()-1);
		html+="</DIV><br>";
		
		old_addresses.pop_back();
	}
	
private:
	std::string &html; /* HTML string where our output is stored */
	const std::string &form_name; /* relative URL to send HTTP responses */
	std::string startform(void) {
		return itemdiv()+"<FORM ACTION=\"/"
			+form_name+"\">";
	}
	
	std::string endform(void) {
		return "<INPUT type=\"submit\" value=\"Go!\"/></FORM></DIV>\n\n";
	}
	int indent;
	int divcount;
	/* Return the color of an object's content div */
	std::string getobjectcolor(void) {
		switch (indent) {
		case 0:
			return "#f0f0f0";
		case 1:
			return "#d0d0d0";
		case 2:
			return "#b0b0b0";
		default:
			return "#909090";
		};
	}
	/* Make a div for this item */
	std::string itemdiv(void) {
		divcount++;
		std::string color=getobjectcolor();
		// Just adjust low digit of color, to get subtle colors:
		if (divcount%2) {
			//color[5]='f';
			color[6]='9';
		} else {
			//color[1]='f';
			color[2]='9';
		}
		/*  // Candy cane debug colors:
		switch (divcount%2) {
		case 0:
			color="#ffffff"; break;
		case 1:
			color="#ff0000"; break;
		}
		*/
		return "<DIV STYLE=\"background-color:"+color+"\">\n\t";
	}
	
	std::string address; /* current fully-qualified object address */
	std::vector<std::string> old_addresses; /* for tracing object names */
	
	/* Make an HTML form to set this field, with this current value. */
	void inner(const char *shortname,const std::string &curvalue) {
		std::string fullname=address+shortname;
		html+=startform()
			+shortname+": <INPUT type=\"text\" name=\""
			+fullname+"\" value=\""
			+curvalue+"\" />"
			+endform(); 
	}
};


class webconfig_editor : public osl::http_responder {
	std::string form_name;
public:
	std::string page_start; /* <HTML>, <BODY>, up to actual items. */
	std::string page_end; /* </BODY>, </HTML> */

	webconfig_editor(const std::string &form_name_) :form_name(form_name_) 
	{
		page_start=OSL_QUOTE_HTML(
<HTML><TITLE>Edit Configuration</TITLE>
  <BODY>
    <H1>Configuration Editor</H1>
);
		page_start+="<FORM ACTION=\"/"+form_name+"\"><INPUT type=\"submit\" value=\"Refresh\" /></FORM>\n\n";
		page_start+=OSL_QUOTE_HTML(
Enter new values and hit enter or Submit:<br>
);
		
		page_end=OSL_QUOTE_HTML(
  </BODY>
</HTML>
);
	}

	bool respond(osl::http_served_client &client) {
		/* Serialize responses.  Parallel packing or unpacking is asking for disaster. */
		static porlock web_lock;
		porlock_scoped scoped_lock(&web_lock);
	
	/* Start the page */
		std::string html=page_start;

	/* Check parameters */
		bool make_form=false;
		bool send_response=false;
		if (client.get_path()=="/" || client.get_path()=="/"+form_name) 
		{ /* initial page request */
			send_response=true; make_form=true;
		}
		
		if (client.get_path().substr(0,2+form_name.size())=="/"+form_name+"?") 
		{ /* form data coming back */
			send_response=true;
			make_form=apply_parameters(html,client.get_path().substr(2+form_name.size()));
		}
		
	/* Create the main form */
		if (make_form) {
			pup_to_HTML_form p(html,form_name);
			webconfig_pup_all(p);

			html+=page_end;
		}
		if (send_response) client.send("text/html",html);
		return send_response;
	}
private:
	/* Change our values according to these CGI FORM parameters.
	   CAUTION: NETWORK-SOURCED DATA! */
	bool apply_parameters(std::string &html,const std::string &parameters)
	{
		if (parameters.size()<2) return true; /* nothing to apply */
		size_t eq=parameters.find_first_of("=");
		if (eq==std::string::npos) {html+="<P>ERROR! Missing equals sign in CGI parameters!\n";return false;}
		std::string fullname=parameters.substr(0,eq);
		std::string value=parameters.substr(eq+1);
		std::cout<<"Setting '"<<fullname<<"' to '"<<value<<"'\n";
		
		pup_from_name_value p(fullname,value);
		webconfig_pup_all(p);
		
		webconfig_save();
		
		if (p.found) {
			return true;
		} else {
			html+="<P>ERROR! Missing field '"+fullname+"'!\n";
			return false;
		}
	}	
};

/* Restore our objects from this .dat file: */
void webconfig_restore(const char *configfile)
{
	try {
		std::ifstream config(configfile,std::ios_base::binary);
		pup_from_binary_file pconf(config);
		webconfig_pup_all(pconf);
		if (config)
			std::cout<<"Restored "<<config.tellg()<<" bytes of objects from "<<configfile<<"\n";
	}
	catch (...) {
		std::cout<<"Tried to restore from "<<configfile<<", but failed...\n";
	}	
}


/* Save a copy of the modified data to our .dat file: */
void webconfig_save(const char *configfile)
{
	try {
		std::ofstream config(configfile,std::ios_base::binary);
		pup_to_binary_file pconf(config);
		webconfig_pup_all(pconf);
	}
	catch (...) {
		std::cout<<"Tried to save to "<<configfile<<", but failed...\n";
	}	
}

osl::http_threaded_server *webconfig_server=0;

/* Create a web server to respond to webconfig requests */
void webconfig_init(unsigned int portNumber,bool startBrowser) {
	webconfig_server=new osl::http_threaded_server(portNumber);
	
	// FIXME: look up real IP address, not localhost here.
	std::string ourURL="http://localhost:"+itos(webconfig_server->get_port());
	std::cout<<"Listening on "<<ourURL<<"/\n";

	if (startBrowser) {
/* Try to launch a web browser to show our new web page. */
#ifdef _WIN32
#		pragma comment(lib,"shell32.lib")
		ShellExecute(NULL, TEXT("open"), TEXT(ourURL.c_str()), NULL, NULL, SW_SHOWNORMAL);
#else
		int ret=system((WEBCONFIG_BROWSER " "+ourURL+" &").c_str());
		if (ret) {}
#endif
	}

	webconfig_server->add_responder(new osl::html_logger(std::cout));
	webconfig_server->add_responder(new webconfig_editor("conf"));
	webconfig_server->start();
}


