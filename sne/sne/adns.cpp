/* Copyright 1998 by the Massachusetts Institute of Technology.
 *
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 * M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "adns.h"

extern "C" {

#include "dns/ares.h"
#include "dns/inet_ntop.h"

}

#include "NetUtil.h"

#ifndef MAX_HOSTNAME_LEN
#define MAX_HOSTNAME_LEN 128
#endif


class AdnsImp : public Adns
{
private:

	ares_channel m_channel;
	fd_set m_read_fds, m_write_fds;
	//struct timeval *m_tvp, m_tv;

	lookupcallback m_cb;
	void* m_arg;

	bool m_needclear;

	static int m_aresinit_count;

	char m_hostname[MAX_HOSTNAME_LEN];

public:
	AdnsImp() : m_cb(NULL), m_channel(NULL), m_needclear(false), m_arg(NULL)
	{
		memset(m_hostname, 0, sizeof(m_hostname));

		if (m_aresinit_count == 0)
		{
			WORD wVersionRequested = MAKEWORD(2,2);
			WSADATA wsaData;
			WSAStartup(wVersionRequested, &wsaData);

			int status = ares_library_init(ARES_LIB_INIT_ALL);
			if (status != ARES_SUCCESS)
			{
				UTL_LOG_HIGH("ares_library_init failed: %s\n", ares_strerror(status));
			}
		}

		++m_aresinit_count;

	}

	~AdnsImp()
	{
		--m_aresinit_count;

		if (m_aresinit_count == 0)
		{
			ares_library_cleanup();

			WSACleanup();
		}

	}

	static void callback(void *arg, int status, int timeouts, struct hostent *host)
	{
		char **p;

		AdnsImp* dnsptr = ((AdnsImp*)arg);

		if (status != ARES_SUCCESS)
		{
			UTL_LOG_HIGH("dns", "%s: %s\n", dnsptr->m_hostname, ares_strerror(status));
			return;
		}

		for (p = host->h_addr_list; *p; p++)
		{
			char addr_buf[46] = "??";

			ares_inet_ntop(host->h_addrtype, *p, addr_buf, sizeof(addr_buf));

			UTL_LOG_HIGH("dns", "lookup ok, [%s==>%s]", host->h_name, addr_buf);

			dnsptr->m_cb(addr_buf, dnsptr->m_arg);

			break;
		}

		dnsptr->m_needclear = true;
	
	}

	virtual void StartLookup( const char* hostname, lookupcallback cb, void* arg ) 
	{
		if (m_channel != NULL)
		{
			FinishLookup();
		}

		m_cb = cb;
		m_arg = arg;

		strcpy_s(m_hostname, MAX_HOSTNAME_LEN, hostname);

		int status = ares_init(&m_channel);
		if (status != ARES_SUCCESS)
		{
			UTL_LOG_HIGH("dns", "ares_init failed: %s\n", ares_strerror(status));
			return ;
		}

		ares_gethostbyname(m_channel, hostname, AF_INET, callback, this);

		m_needclear = false;
	}

	virtual void Update() 
	{
		if (m_channel == NULL)
		{
			return;
		}

		if (m_needclear)
		{
			FinishLookup();
			m_needclear = false;
			return;
		}

		FD_ZERO(&m_read_fds);
		FD_ZERO(&m_write_fds);
		int nfds = ares_fds(m_channel, &m_read_fds, &m_write_fds);
		if (nfds == 0)
			return;

		//m_tvp = ares_timeout(m_channel, NULL, &m_tv);

		struct timeval tvp;
		tvp.tv_sec = 0;
		tvp.tv_usec = 0;

		select(nfds, &m_read_fds, &m_write_fds, NULL, &tvp);
		ares_process(m_channel, &m_read_fds, &m_write_fds);
	}

	virtual void FinishLookup() 
	{
		if (m_channel != NULL)
		{
			ares_destroy(m_channel);
			m_channel = NULL;
			m_arg = NULL;
		}
	}
};

int AdnsImp::m_aresinit_count = 0;

Adns* Adns::New()
{
	return new AdnsImp();
}

void Adns::Delelte( Adns* ptr )
{
	delete ptr;
}