/*
	Easy HTTP accessible program configuration library.
	
	The basic idea is you use a PUP-style interface to expose
	the tweakable variables in your program, and then just
	point a web browser at the program to view and edit them.
	
	Handy for debugging, remote access, etc.
	
	Dr. Orion Lawlor, lawlor@alaska.edu, 2012-01-31 (Public Domain)
*/
#ifndef __OSL_WEBCONFIG_H
#define __OSL_WEBCONFIG_H

#include <stdio.h> /* for snprintf, below */
#include "webserver_threaded.h"




/* Default web browser for non-Windows platforms */
//#define WEBCONFIG_BROWSER "firefox"
#ifndef WEBCONFIG_BROWSER
#define WEBCONFIG_BROWSER "x-www-browser"  /*<- is this Ubuntu specific? */
#endif

/**
  Webconfig will save your data to this binary file
  every time you edit it over the web.
*/
#ifndef WEBCONFIG_FILENAME
#define WEBCONFIG_FILENAME "config.dat"
#endif




/* Used for pup'ing enums: lists values for the enum, and human/GUI names */
struct name_value_record {
public:
	unsigned int value;
	const char *name; /* or 0, for the end of the list */
};


/**
  Pup the value v's field x, using the name x, into the pup_er named p.
*/
#define PUPf(x) pup(p,#x,v.x);

/**
  Pup the variable x, using the name x, into the pup_er named p.
*/
#define PUPn(x) pup(p,#x,x);

/**
  Pup the comment c--this shows up as HTML code in the GUI.
*/
#define PUPc(commentString) p.comment(commentString)


/* Pup's std::vectors */
template <class PUP_er,class T>
void pup(PUP_er &p,std::vector<T> &v) {
	int length=v.size();
	PUPn(length);
	v.resize(length);
	
	for (int i=0;i<length;i++) {
		char index[100];
		snprintf(index,100,"%d",i);
		pup(p,index,v[i]);
	}
}


/**
 Virtual superclass of all inheritable PUP_er objects.
 PUP_ers can also be defined at compile time, by overloading.
*/
class pup_er_virtual {
public:
	typedef pup_er_virtual this_t;

	/* Default: ignore comments */
	virtual void comment(const std::string &s) { }

	/* Subclasses need to define how to pup floats, ints, and strings. */
	virtual void pup(const char *shortname,float &value) =0;
	friend void pup(this_t &p,const char *n,float &v) {p.pup(n,v);}
	
	virtual void pup(const char *shortname,int &value) =0;
	friend void pup(this_t &p,const char *n,int &v) {p.pup(n,v);}
	
	virtual void pup(const char *shortname,std::string &value) =0;
	friend void pup(this_t &p,const char *n,std::string &v) {p.pup(n,v);}
	
	/* Default: pup enums like ints */
	virtual void pup(const char *shortname,unsigned int &value,const name_value_record *namevalues) 
	{
		int v=value;
		pup(shortname,v);
		value=v;
	}
	friend void pup(this_t &p,const char *shortname,
			unsigned int &value,const name_value_record *namevalues) 
	{
		p.pup(shortname,value,namevalues);
	}
	
	/* Pup for objects: default is to do nothing */
	virtual void pup_objectbegin(const char *shortname) {}
	virtual void pup_objectend(const char *shortname) {}
};
	template <class T>
	void pup(pup_er_virtual &p,const char *shortname,T &value) {
		p.pup_objectbegin(shortname);
		pup(p,value);
		p.pup_objectend(shortname);
	}


/************ Webconfig User Functions *************/

/* Read a copy of all persistent data from this file */
void webconfig_restore(const char *configfile=WEBCONFIG_FILENAME);

/* Save a copy of all persistent data to our .dat file: */
void webconfig_save(const char *configfile=WEBCONFIG_FILENAME);

/* Create a web server to respond to webconfig requests to read/modify persistent data */
void webconfig_init(unsigned int portNumber=8888,bool startbrowser=true);

/* This is our server process. Use add_responder to populate the namespace. */
extern osl::http_threaded_server *webconfig_server;


/* Pup this object.  
  This is an adaptor, usually generated for you via the WEBCONFIG_THIS macro below. */
class pup_this_object {
public:
	virtual void pupto(pup_er_virtual &p) =0;
};

/* Add this object to be pup'd at any time by webconfig. */
void webconfig_add_pup(pup_this_object *p);

/* Pup all web config objects registered above */
void webconfig_pup_all(pup_er_virtual &p);

template <class T>
class pup_this_object_t : public pup_this_object {
public:
	std::string name;
	T &obj;
	pup_this_object_t(const std::string &name_,T &obj_) :name(name_), obj(obj_) {}
	virtual void pupto(pup_er_virtual &p) { pup(p,name.c_str(),obj); }
};
template <class T> pup_this_object *make_pup_this_object_t(const std::string &name,T &t) {
	return new pup_this_object_t<T>(name,t);
}

/** Use webconfig to allow read/write access to this object. */
#define WEBCONFIG_THIS(objectname) \
	do { \
		static bool added=false; \
		if (!added) { \
			added=true; \
			webconfig_add_pup(make_pup_this_object_t(#objectname,objectname)); \
	}	} while(0)







#endif
